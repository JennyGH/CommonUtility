#!/bin/bash
BUILD_DIR=`pwd`/build-linux
echo $BUILD_DIR
BUILD_TYPE=Debug
if [ ! -d "$BUILD_DIR" ]; then
    mkdir "$BUILD_DIR"
fi
cd "$BUILD_DIR"
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE                \
      ..
cmake --build . --config $BUILD_TYPE -- -j $(nproc)