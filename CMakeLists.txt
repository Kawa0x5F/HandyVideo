cmake_minimum_required(VERSION 3.25)
project(HandyVideo)

# コンパイラの指定
set(CMAKE_C_COMPILER hgcc)
set(CMAKE_C_STANDARD 11)

# 使用しているライブラリのパスを指定
find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h)
find_library(AVCODEC_LIBRARY avcodec)

find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h)
find_library(AVFORMAT_LIBRARY avformat)

find_path(AVUTIL_INCLUDE_DIR libavutil/avutil.h)
find_library(AVUTIL_LIBRARY avutil)

find_path(SWSCALE_INCLUDE_DIR libswscale/swscale.h)
find_library(SWSCALE_LIBRARY swscale)

# ビルド対象のソースファイルの指定
add_executable(HandyVideo src/main.c src/videoInput.c)

# インクルードディレクトリの指定
target_include_directories(HandyVideo PRIVATE ${AVCODEC_INCLUDE_DIR} ${AVFORMAT_INCLUDE_DIR} ${AVUTIL_INCLUDE_DIR} ${SWSCALE_INCLUDE_DIR})

# ライブラリを実行ファイルにリンクする
target_link_libraries(HandyVideo PRIVATE ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY} ${SWSCALE_LIBRARY})