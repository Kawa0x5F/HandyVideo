/*
 * HnadyGraphicで動画を再生するプログラム
 * 2024/01/10 Kawa_
 */

#include <stdio.h>
#include <stdlib.h>
#include <handy.h>

#include "videoInput.h"

#define WID_WIDTH_SIZE 100
#define WID_HEIGHT_SIZE 100
#define STREAM_FRAME_NUM 20

int main() {
    /* 変数の宣言 */
    char* videoPath = NULL;
    int frameIndex = 0;
    double transIntToDouble[256];
    doubleLayer doubleLayerId; // ダブルレイヤ変数の宣言


    // 0~255の整数を0~1の小数に変換するための配列を作成
    for(int i = 0; i < 256; i++) {
        transIntToDouble[i] = (double)i / 255;
    }

    // ピクセルデータ格納用の構造体
    PixFrameData pixCtx;

    // 動画のパスを受け取る
    videoPath = GetVideoPath();

    // 動画の情報を取得するための変数を初期化する
    InitInputVariable(videoPath,WID_WIDTH_SIZE,WID_HEIGHT_SIZE);
    printf("初期化しました");

    // pixCtxのメモリを確保する
    pixCtx.pix = (RGB**) malloc((STREAM_FRAME_NUM + 1) * sizeof(RGB*));
    for(int i = 0; i < (STREAM_FRAME_NUM + 1); i++) {
        pixCtx.pix[i] = (RGB *) malloc((WID_WIDTH_SIZE * WID_HEIGHT_SIZE) * sizeof(RGB));
    }

    // ウィンドウを開く
    HgOpen(WID_WIDTH_SIZE, WID_HEIGHT_SIZE);
    HgSetTitle("HandyVideo")

    // ダブルレイヤを作成する
    doubleLayerId = HgWAddDoubleLayer(0);

    printf("EnterKeyを押すと動画の再生を開始します。\n");
    // ビットマップをHandyGraphicで描画する
    while (1) {
        // レイヤ処理
        int layerId = HgLSwitch(&doubleLayerId);
        HgLClear(layerId); // レイヤの描画を削除する

        // フレームごとのRGBピクセルデータを取得する
        if(GetFrameData(&pixCtx,frameIndex, STREAM_FRAME_NUM) == -1){
            printf("動画の読み込みに失敗しました。\n");
            break;
        }
        int nowFrameIndex = frameIndex%STREAM_FRAME_NUM;
        if(frameIndex >= 1)
            qsort(pixCtx.pix[nowFrameIndex],WID_WIDTH_SIZE*WID_HEIGHT_SIZE,sizeof(RGB*),Compare);
        for (int i = 0; i < WID_HEIGHT_SIZE*WID_WIDTH_SIZE; i++) {
            if(frameIndex > 0 && pixCtx.pix[nowFrameIndex][i].diff == '0')continue;
            if (i > 0 && pixCtx.pix[nowFrameIndex][i].r != pixCtx.pix[nowFrameIndex][i - 1].r &&
                pixCtx.pix[nowFrameIndex][i].g != pixCtx.pix[nowFrameIndex][i - 1].g &&
                pixCtx.pix[nowFrameIndex][i].b != pixCtx.pix[nowFrameIndex][i - 1].b) {
                // RGBでピクセルデータのセットをする
                HgWSetFillColor(layerId,HgRGB(transIntToDouble[pixCtx.pix[nowFrameIndex][i].r],
                                     transIntToDouble[pixCtx.pix[nowFrameIndex][i].g],
                                     transIntToDouble[pixCtx.pix[nowFrameIndex][i].b]));
            }
            // 大きさ1 1 の塗りつぶされたボックスを描画する
            HgWBoxFill(layerId,pixCtx.pix[nowFrameIndex][i].x,pixCtx.pix[nowFrameIndex][i].y,1,1,0);
        }
        frameIndex++;
        // 描画のために1ms時間を止める
        HgSleep(0.1);
    }

    // ウィンドウを閉じる
    HgCloseAll();

    printf("終了処理をしています。そのままで待ってください\n");

    FreeInputVariable();

    // videoPathのメモリを解放する
    free(videoPath);

    // pixCtxのメモリを解放する
    for(int i = 0; i < STREAM_FRAME_NUM; i++) {
        free(pixCtx.pix[i]);
    }
    free(pixCtx.pix);

    return 0;
}
