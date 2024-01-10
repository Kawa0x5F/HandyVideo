# 重大な警告
このプログラムは未完成です。以下の致命的な問題がまだ解決されていません。

このプログラムを使う時は以下の問題点があることを念頭に置いて実行してください。

- 描画時に画面上部がうまく描画されない
- 描画の速度が遅いため、再生も遅い
- 動画の途中で異常終了する
- 動作確認済みなのはMP4ファイルのみ

# HandyVideo
これはHandyGraphicとFFmpegを使用して
C言語で動画を読み込み、再生するプログラムです。

なお、このプログラムを実行するにはHandyGraphicおよび 
FFmpegのインストールが必要です。

FFmpegは、以下の手順に従ってインストールしてください。

## FFmpegのインストール
### Homebrew
```bash:bash
$ /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```
インストールの詳細については[Homebrewの公式ページ（日本語）](https://brew.sh/ja/ "Homebrew")を参照してください。

### FFmpeg
HomebrewでFFmpegをインストールします。
```bash:bash
$ brew install ffmpeg
```

## 利用方法
プロジェクトをダウンロードし、展開先のディレクトリで以下のコマンドを実行してください。
cmakeのインストール方法は省略します。
```bash:bash
$ cmake -S . -B build
$ cmake --build build
$ ./build/HandyVideo
```
この後は、プログラムの指示に従ってください。