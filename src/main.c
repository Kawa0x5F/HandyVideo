/*
 * HandyGraphicを利用して動画編集ができるアプリケーションを作成する
 * mp4拡張子の動画ファイルに対して、再生、編集、出力ができることを目標とする
 * 2023/12/07 Kawa0x5F
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <handy.h>

#include "videoInput.h"

#define WID_WIDTH_SIZE 100
#define WID_HEIGHT_SIZE 100

int main() {
    char* videoPath = NULL;
    int frameIndex = 0;

    // ピクセルデータ格納用の構造体
    PixFrameData pixCtx;

    // 動画のパスを受け取る
    videoPath = GetVideoPath();

    // 受け取ったパス先の動画からRBGのピクセルデータを取得
    printf("動画の読み込み中...\n");
    VideoToBit(&pixCtx,videoPath,WID_WIDTH_SIZE,WID_HEIGHT_SIZE);

    printf("動画の再生準備中...\n");
    for(int i=0; i < pixCtx.numFrames; i++){
        qsort(pixCtx.pix[i],WID_WIDTH_SIZE*WID_HEIGHT_SIZE,sizeof(RGB*),Compare);
    }

    printf("完了\n");

    HgOpen(WID_WIDTH_SIZE, WID_HEIGHT_SIZE);
    HgSetTitle("HandyVideo")

    printf("%lld\n", pixCtx.numFrames);
    printf("EnterKeyを押すと動画の再生を開始します。\n");
    getchar();
    // ビットマップをHandyGraphicで描画する
    while (pixCtx.numFrames >= frameIndex) {
        //getchar();
        for (int i = 0; i < WID_HEIGHT_SIZE*WID_WIDTH_SIZE; i++) {
            if(frameIndex > 0 && pixCtx.pix[frameIndex][i].diff == '0') {
                continue;
            }else {
                if (i > 0 && pixCtx.pix[frameIndex][i].r != pixCtx.pix[frameIndex][i - 1].r &&
                    pixCtx.pix[frameIndex][i].g != pixCtx.pix[frameIndex][i - 1].g &&
                    pixCtx.pix[frameIndex][i].b != pixCtx.pix[frameIndex][i - 1].b) {
                    // RGBでピクセルデータのセットをする
                    HgSetFillColor(HgRGB(pixCtx.pix[frameIndex][i].r,
                                         pixCtx.pix[frameIndex][i].g,
                                         pixCtx.pix[frameIndex][i].b));
                }
            }
            // 大きさ1 1 の塗りつぶされたボックスを描画する
            HgBoxFill(pixCtx.pix[frameIndex][i].x,pixCtx.pix[frameIndex][i].y,1,1,0);
            // 描画の位置を少しずつ変える
            // 描画のために1ms時間を止める
        }
        frameIndex++;
        HgSleep(0.1);
//        if(frameIndex > 100){
//            break;
//        }
    }
    HgGetChar();
    HgCloseAll();

    printf("終了処理をしています。そのままで待ってください\n");

    // videoPathのメモリを解放する
    free(videoPath);

    // pixCtxのメモリを解放する
    for(int i = 0; i < pixCtx.numFrames; i++) {
        free(pixCtx.pix[i]);
    }
    free(pixCtx.pix);

    return 0;
}
