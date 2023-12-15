/*
 * 動画ファイルの受け取り関係の処理をする
 * 2023/12/15 Kawa0x5F
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
enum AVPixelFormat outPixFmt = AV_PIX_FMT_RGB24; // 出力フレームのピクセルフォーマット


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
    strcpy(videoPath,"/");
    strcat(videoPath,input);

    // inputのメモリを解放する
    free(input);

    return videoPath;
}

void InitInputVariable(char* videoPath, int width, int height){
    int ret;

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

    videoStreamIndex = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_VIDEO,
                                           -1,-1,&dec,0);
    if(videoStreamIndex < 0){
        fprintf(stderr, "Could not find video strea\n");
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

    ret = av_read_frame(fmtCtx, &pkt);
    if (ret < 0) {
        return -1; // EOF or error
    }

    // パケットがビデオストリームに属するかチェック
    if (pkt.stream_index == videoStreamIndex) {
        // パケットの送信
        ret = avcodec_send_packet(decCtx, &pkt);
        if (ret < 0) {
            fprintf(stderr, "Error sending packet\n");
            exit(1);
        }

        while (ret >= 0) {
            printf("この場所を通りました\n");
            // 目的の地点までフレームを進めます
            for (int frameIndex = 0; frameIndex < reqFrameIndex; frameIndex++) {
                printf("この場所を通りましたfor\n");
                ret = avcodec_receive_frame(decCtx, inFrame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    printf("抜けます1\n");
                    return -1; // フレームがないか、終了した場合はループを抜ける
                } else if (ret < 0) {
                    fprintf(stderr, "Error receiving frame\n");
                    exit(1);
                }
            }

            printf("この場所を通りました2\n");

            // 目的のフレームを読み込む
            ret = avcodec_receive_frame(decCtx, inFrame);
            printf("ret is %d\n", ret);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                printf("%d %d\n", AVERROR(EAGAIN), AVERROR_EOF);
                printf("抜けます2\n");
                return -1; // フレームがないか、終了した場合はループを抜ける
            } else if (ret < 0) {
                fprintf(stderr, "Error receiving frame\n");
                exit(1);
            }

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
            printf("この場所を通りました3\n");
            // RGBプレーンを操作してピクセルデータを処理する
            for (int y = 0; y < outHeight; y++) {
                for (int x = 0; x < outWidth; x++) {
                    uint8_t r = rgbData[y * rgbLineSize + x * 3]; // (x,y)座標の赤成分
                    uint8_t g = rgbData[y * rgbLineSize + x * 3 + 1]; //  (x,y)座標の緑成分
                    uint8_t b = rgbData[y * rgbLineSize + x * 3 + 2]; //  (x,y)座標の青成分
                    printf("videoInput %d %d %d\n", (int) r, (int) g, (int) b);
                    pixCtx->pix[reqFrameIndex % streamFrame][(y * outWidth) + x].r = (int) r;
                    pixCtx->pix[reqFrameIndex % streamFrame][(y * outWidth) + x].g = (int) g;
                    pixCtx->pix[reqFrameIndex % streamFrame][(y * outWidth) + x].b = (int) b;
                    if (reqFrameIndex == 0) {
                        pixCtx->pix[reqFrameIndex][(y * outWidth) + x].diff = '1';
                    } else {
                        if (pixCtx->pix[reqFrameIndex][(y * outWidth) + x].r ==
                            pixCtx->pix[reqFrameIndex - 1][(y * outWidth) + x].r &&
                            pixCtx->pix[reqFrameIndex][(y * outWidth) + x].g ==
                            pixCtx->pix[reqFrameIndex - 1][(y * outWidth) + x].g &&
                            pixCtx->pix[reqFrameIndex][(y * outWidth) + x].b ==
                            pixCtx->pix[reqFrameIndex - 1][(y * outWidth) + x].b) {
                            pixCtx->pix[reqFrameIndex][(y * outWidth) + x].diff = '0';
                        } else {
                            pixCtx->pix[reqFrameIndex][(y * outWidth) + x].diff = '1';
                        }
                    }
                    pixCtx->pix[reqFrameIndex][(y * outWidth) + x].y = outHeight - (y + 1);
                    pixCtx->pix[reqFrameIndex][(y * outWidth) + x].x = x;
                }
            }
        }
    }
    getchar();


    av_freep(&inFrame->data[0]);
    av_freep(&outFrame->data[0]);

    return 0;
}

// ffmpegを使用して動画からRGBピクセルデータを取得する
//void VideoToBit(PixFrameData *pixCtx,char* videoPath,int width,int height) {
//
//
//    // データ格納用の構造体の初期化
//    numFrames = fmtCtx->streams[videoStreamIndex]->duration;
//    pixCtx->numFrames = numFrames;
//
//    // pixCtxのメモリを確保する
//    (*pixCtx).pix = (RGB**) malloc((*pixCtx).numFrames * sizeof(RGB*));
//    for(int i = 0; i < (*pixCtx).numFrames; i++) {
//        (*pixCtx).pix[i] = (RGB*) malloc((outWidth*outHeight) * sizeof(RGB));
//
//    }
//    int frameIndex = 0;
//
//    while(1) {
//        // パケットの読み込み
//        ret = av_read_frame(fmtCtx, &pkt);
//        if (ret < 0) {
//            break; // EOF or error
//        }
//
//        // パケットがビデオストリームに属するかチェック
//        if (pkt.stream_index == videoStreamIndex) {
//            // パケットの送信
//            ret = avcodec_send_packet(decCtx, &pkt);
//            if (ret < 0) {
//                fprintf(stderr, "Error sending packet\n");
//                exit(1);
//            }
//
//            while (1) {
//                // フレームの受信
//                ret = avcodec_receive_frame(decCtx, inFrame);
//                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
//                    break; // フレームがないか、終了した場合はループを抜ける
//                } else if (ret < 0) {
//                    fprintf(stderr, "Error receiving frame\n");
//                    exit(1);
//                }
//
//                // 入力フレームから出力フレームへの変換
//                sws_scale(swsCtx,
//                          (const uint8_t *const *) inFrame->data,
//                          inFrame->linesize,
//                          0,
//                          inHeight,
//                          outFrame->data,
//                          outFrame->linesize);
//
//                // 出力フレームからRGBピクセルデータの取得
//                uint8_t *rgbData = outFrame->data[0]; // RGBプレーンの先頭アドレス
//                int rgbLinesize = outFrame->linesize[0]; // RGBプレーンの１行あたりのバイト数
//
//                // RGBプレーンを操作してピクセルデータを処理する
//                for (int y = 0; y < outHeight; y++) {
//                    for (int x = 0; x < outWidth; x++) {
//                        uint8_t r = rgbData[y * rgbLinesize + x * 3]; // (x,y)座標の赤成分
//                        uint8_t g = rgbData[y * rgbLinesize + x * 3 + 1]; //  (x,y)座標の緑成分
//                        uint8_t b = rgbData[y * rgbLinesize + x * 3 + 2]; //  (x,y)座標の青成分
//
//                        pixCtx->pix[frameIndex][(y*outWidth)+x].r = (int)r;
//                        pixCtx->pix[frameIndex][(y*outWidth)+x].g = (int)g;
//                        pixCtx->pix[frameIndex][(y*outWidth)+x].b = (int)b;
//                        if(frameIndex == 0){
//                            pixCtx->pix[frameIndex][(y*outWidth)+x].diff = '1';
//                        }else{
//                            if(pixCtx->pix[frameIndex][(y*outWidth)+x].r == pixCtx->pix[frameIndex-1][(y*outWidth)+x].r &&
//                               pixCtx->pix[frameIndex][(y*outWidth)+x].g == pixCtx->pix[frameIndex-1][(y*outWidth)+x].g &&
//                               pixCtx->pix[frameIndex][(y*outWidth)+x].b == pixCtx->pix[frameIndex-1][(y*outWidth)+x].b) {
//                                pixCtx->pix[frameIndex][(y*outWidth)+x].diff = '0';
//                            }else{
//                                pixCtx->pix[frameIndex][(y*outWidth)+x].diff = '1';
//                            }
//                        }
//                        pixCtx->pix[frameIndex][(y*outWidth)+x].y = outHeight - (y + 1);
//                        pixCtx->pix[frameIndex][(y*outWidth)+x].x = x;
//                    }
//                }
//                frameIndex++;
//            }
//
//        }
//        // パケットを解放する
//        av_packet_unref(&pkt);
//
//    }
//    // リソースを解放する
//    avcodec_free_context(&decCtx);
//    avformat_close_input(&fmtCtx);
//    av_freep(&inFrame->data[0]);
//    av_freep(&outFrame->data[0]);
//    av_frame_free(&inFrame);
//    av_frame_free(&outFrame);
//    sws_freeContext(swsCtx);
//}// end of VideoToBit()

// RGBピクセルデータをフレームごとに一次元配列を持つ構造体に代入する
void PushPixDraw(){

}// end of PushPixDraw()

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