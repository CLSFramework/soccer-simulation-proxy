#!/bin/bash

# clone and build librcsc
git clone https://github.com/CLSFramework/soccer-simulation-proxy-librcsc.git

cd soccer-simulation-proxy-librcsc

mkdir build

cd build

cmake ..

make -j

make install

cd ../../../

mkdir build

cd build

cmake ..

make -j