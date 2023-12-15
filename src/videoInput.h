// RGBピクセルデータの格納用
typedef struct {
    int r; // 赤成分
    int g; // 緑成分
    int b; // 青成分
    char diff; // 前フレームと色が違うかどうか 0:同じ 1:違う
    int y; // y座標
    int x; // x座標
}RGB;

// フレームごとの２次元のRGBピクセルデータの格納用
typedef struct {
    RGB **pix;        // ピクセルデータのポインタ
    int64_t numFrames;   // フレーム数
}PixFrameData;

char* GetVideoPath();

void InitInputVariable(char *videoPath, int outWidth, int outHeight);

int GetFrameData(PixFrameData *pixCtx, int reqFrameIndex, int streamFrame);

void FreeInputVariable();

int Compare(const void *rgb1, const void *rgb2);