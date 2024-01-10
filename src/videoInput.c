/*
 * 動画ファイルの受け取り関係の処理をする
 * 2023/01/10 Kawa_
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>

#include "videoInput.h"

#define FILE_PATH_BUF 256

// グローバル変数の宣言
// スケーリングコンテキストの確保
struct SwsContext *swsCtx  = NULL;

// フレームの読み込みループ
AVFormatContext *fmtCtx = NULL;
AVCodecContext  *decCtx = NULL;
AVPacket  pkt;

// デコーダーの探索
AVCodec const *dec = NULL;
int videoStreamIndex;
// 入力フレームと出力フレームのフォーマットとサイズの設定
int inWidth; // 入力フレームの幅
int inHeight; // 入力フレームの高さ
enum AVPixelFormat inPixFmt; // 入力フレームのピクセルフォーマット

int outWidth; // 出力フレームの幅
int outHeight; // 出力フレームの高さ
enum AVPixelFormat outPixFmt; // 出力フレームのピクセルフォーマット


// 動画のパスをターミナルから受け取る（仮）
char* GetVideoPath() {
    char* input = NULL;
    char* videoPath = NULL;

    // 文字列の保存用にメモリを確保
    input = (char*)malloc(sizeof(char) * FILE_PATH_BUF);
    videoPath = (char*)malloc(sizeof(char) * FILE_PATH_BUF);

    // メモリの確保に失敗したら終了
    if( input == NULL) {
        fprintf(stderr, "input：文字列配列の作成に失敗しました\n");
    }else {
        fprintf(stderr, "input：文字列配列の作成に成功しました\n");
    }
    if( videoPath == NULL) {
        fprintf(stderr, "videoPath：文字列配列の作成に失敗しました\n");
    }else {
        fprintf(stderr, "videoPath：文字列配列の作成に成功しました\n");
    }

    // inputに動画までの絶対パスを受け取る
    printf("動画の絶対パスを入力してください\n");
    scanf("%s", input);
    getchar();// デバッグ用。scanf後のreturnキー入力の残りをとる

    // videoPathにルートディレクトリを表す/と、動画までの絶対パスを付与する
    strcat(videoPath,input);

    // inputのメモリを解放する
    free(input);

    return videoPath;
}

void InitInputVariable(char* videoPath, int width, int height){
    int ret;

    printf("%s \n", videoPath);

    // 入力ファイルのオープン
    ret = avformat_open_input(&fmtCtx, videoPath, NULL, NULL);
    if(ret < 0) {
        fprintf(stderr, "could not open input file\n");
        exit(1);
    }

    // ストリーム情報の取得
    ret = avformat_find_stream_info(fmtCtx, NULL);
    if(ret < 0) {
        fprintf(stderr, "could not open stream information\n");
        exit(1);
    }

    // デコーダーの探索
    videoStreamIndex = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_VIDEO,
                                           -1,-1,&dec,0);
    if(videoStreamIndex < 0){
        fprintf(stderr, "Could not find video stream\n");
        exit(1);
    }

    // デコーダーコンテキストの確保
    decCtx = avcodec_alloc_context3(dec);

    // デコーダーコンテキストの設定
    ret = avcodec_parameters_to_context(decCtx,
                                        fmtCtx->streams[videoStreamIndex]->codecpar);
    if(ret < 0){
        fprintf(stderr, "Could not copy codec parameters\n");
        exit(1);
    }

    // パケットとフレームの確保
    pkt.data = NULL;
    pkt.size = 0;

    // 入力フレームと出力フレームのフォーマットとサイズの設定
    inWidth = fmtCtx->streams[videoStreamIndex]->codecpar->width; // 入力フレームの幅
    inHeight = fmtCtx->streams[videoStreamIndex]->codecpar->height; // 入力フレームの高さ
    inPixFmt = AV_PIX_FMT_YUV420P; // 入力フレームのピクセルフォーマット

    outWidth = width; // 出力フレームの幅
    outHeight = height; // 出力フレームの高さ
    outPixFmt = AV_PIX_FMT_RGB24; // 出力フレームのピクセルフォーマット

    // スケーリングコンテキストの初期化
    swsCtx = sws_getContext(inWidth,inHeight,inPixFmt,
                            outWidth,outHeight,outPixFmt,
                            SWS_BILINEAR,NULL,NULL,NULL);
}

int GetFrameData(PixFrameData *pixCtx, int reqFrameIndex, int streamFrame){

    int ret;
    int frameIndex = 0;

    // 入力フレームと出力フレームの確保
    AVFrame *inFrame = NULL;
    AVFrame *outFrame = NULL;

    // 入力フレームの確保と初期化
    inFrame = av_frame_alloc();
    inFrame->width = inWidth;
    inFrame->height = inHeight;
    inFrame->format = inPixFmt;
    av_image_alloc(inFrame->data, inFrame->linesize,
                   inFrame->width, inFrame->height,
                   inFrame->format, 32);

    // 出力フレームの確保と初期化
    outFrame = av_frame_alloc();
    outFrame->width = outWidth;
    outFrame->height = outHeight;
    outFrame->format = outPixFmt;
    av_image_alloc(outFrame->data, outFrame->linesize,
                   outFrame->width, outFrame->height,
                   outFrame->format, 32);


    // デコーダーのオープン
    ret = avcodec_open2(decCtx, dec, NULL);
    if(ret < 0){
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }

    while(frameIndex <= reqFrameIndex){
        ret = av_read_frame(fmtCtx, &pkt);
        if (ret < 0) {
            break; // EOF or error
        }

        // パケットがビデオストリームに属するかチェック
        if (pkt.stream_index == videoStreamIndex) {
            // パケットの送信
            ret = avcodec_send_packet(decCtx, &pkt);
            if (ret < 0) {
                fprintf(stderr, "Error sending packet\n");
                exit(1);
            }


            while (1) {
                ret = avcodec_receive_frame(decCtx, inFrame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
//                return -1; // フレームがないか、終了した場合はループを抜ける
                    break;
                } else if (ret < 0) {
                    fprintf(stderr, "Error receiving frame\n");
                    exit(1);
                }
                frameIndex += 1;

                // 入力フレームから出力フレームへの変換
                sws_scale(swsCtx,
                          (const uint8_t *const *) inFrame->data,
                          inFrame->linesize,
                          0,
                          inHeight,
                          outFrame->data,
                          outFrame->linesize);

                // 出力フレームからRGBピクセルデータの取得
                uint8_t *rgbData = outFrame->data[0]; // RGBプレーンの先頭アドレス
                int rgbLineSize = outFrame->linesize[0]; // RGBプレーンの１行あたりのバイト数
                // RGBプレーンを操作してピクセルデータを処理する
                int nowFrameIndex = reqFrameIndex % streamFrame;
                int beforeFrameIndex = (nowFrameIndex - 1) % streamFrame;
                for (int y = 0; y < outHeight; y++) {
                    for (int x = 0; x < outWidth; x++) {
                        uint8_t r = rgbData[y * rgbLineSize + x * 3]; // (x,y)座標の赤成分
                        uint8_t g = rgbData[y * rgbLineSize + x * 3 + 1]; //  (x,y)座標の緑成分
                        uint8_t b = rgbData[y * rgbLineSize + x * 3 + 2]; //  (x,y)座標の青成分
                        pixCtx->pix[nowFrameIndex][(y * outWidth) + x].r = (int) r;
                        pixCtx->pix[nowFrameIndex][(y * outWidth) + x].g = (int) g;
                        pixCtx->pix[nowFrameIndex][(y * outWidth) + x].b = (int) b;
                        if (nowFrameIndex == 0) {
                            pixCtx->pix[reqFrameIndex][(y * outWidth) + x].diff = '1';
                        } else {
                            if (pixCtx->pix[nowFrameIndex][(y * outWidth) + x].r ==
                                pixCtx->pix[beforeFrameIndex][(y * outWidth) + x].r &&
                                pixCtx->pix[nowFrameIndex][(y * outWidth) + x].g ==
                                pixCtx->pix[beforeFrameIndex][(y * outWidth) + x].g &&
                                pixCtx->pix[nowFrameIndex][(y * outWidth) + x].b ==
                                pixCtx->pix[beforeFrameIndex][(y * outWidth) + x].b) {
                                pixCtx->pix[nowFrameIndex][(y * outWidth) + x].diff = '0';
                            } else {
                                pixCtx->pix[nowFrameIndex][(y * outWidth) + x].diff = '1';
                            }
                        }
                        pixCtx->pix[nowFrameIndex][(y * outWidth) + x].y = outHeight - (y + 1);
                        pixCtx->pix[nowFrameIndex][(y * outWidth) + x].x = x;
                    }
                }
            }
        }
        // パケットを解放する
        av_packet_unref(&pkt);
    }

    av_freep(&inFrame->data[0]);
    av_freep(&outFrame->data[0]);

    return 0;
}

void FreeInputVariable(){
    // スケーリングコンテキストの解放
    sws_freeContext(swsCtx);
    // 入力ファイルのクローズ
    avformat_close_input(&fmtCtx);
}

int Compare(const void *rgb1, const void *rgb2){
    if(((RGB *)rgb1)->r < ((RGB *)rgb2)->r){
        return -1;
    }else if(((RGB *)rgb1)->r > ((RGB *)rgb2)->r){
        return 1;
    }else {
        if(((RGB *)rgb1)->g < ((RGB *)rgb2)->g){
            return -1;
        }else if(((RGB *)rgb1)->g > ((RGB *)rgb2)->g){
            return 1;
        }else{
            if(((RGB *)rgb1)->g < ((RGB *)rgb2)->g){
                return -1;
            }else if(((RGB *)rgb1)->g > ((RGB *)rgb2)->g){
                return 1;
            }else{
                return 0;
            }
        }
    }
}