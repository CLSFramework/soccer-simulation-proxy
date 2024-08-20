#!/bin/bash

set -e

# wget -c "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage" -O linuxdeploy-x86_64.AppImage
# chmod 777 linuxdeploy-x86_64.AppImage
PLAYER_APP_IMAGE_DIR_NAME="sample-player-x86_64"
mkdir -p $PLAYER_APP_IMAGE_DIR_NAME
COACH_APP_IMAGE_DIR_NAME="sample-coach-x86_64"
mkdir -p $COACH_APP_IMAGE_DIR_NAME
TRAINER_APP_IMAGE_DIR_NAME="sample-trainer-x86_64"
mkdir -p $TRAINER_APP_IMAGE_DIR_NAME

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
echo "SCRIPT_DIR=" $SCRIPT_DIR

BUILD_PWD="${SCRIPT_DIR}/../../build/bin/"
APP_IMAGE_DIR="${SCRIPT_DIR}"
echo "BUILD_PWD=" $BUILD_PWD
echo "APP_IMAGE_DIR=" $APP_IMAGE_DIR

# print ldd info
ldd $BUILD_PWD/sample_player

# find libc and libstdc++ libz dependencies
LIBRCSC_PATH=$(ldd $BUILD_PWD/sample_player | grep librcsc.so.18 | awk '{ print $3 }')
LIBZ_PATH=$(ldd $BUILD_PWD/sample_player | grep libz.so | awk '{ print $3 }')
LIBSTDCPP_PATH=$(ldd $BUILD_PWD/sample_player | grep libstdc++ | awk '{ print $3 }')
LIBM_PATH=$(ldd $BUILD_PWD/sample_player | grep libm.so | awk '{ print $3 }')
LIBGCC_PATH=$(ldd $BUILD_PWD/sample_player | grep libgcc_s.so | awk '{ print $3 }')
LIBC_PATH=$(ldd $BUILD_PWD/sample_player | grep libc.so | awk '{ print $3 }')
LIB_THRIFT_PATH=$(ldd $BUILD_PWD/sample_player | grep libthrift-0.1 | awk '{ print $3 }')
LIB_SSL_PATH=$(ldd $BUILD_PWD/sample_player | grep libssl.so | awk '{ print $3 }')
LIB_CRYPTO_PATH=$(ldd $BUILD_PWD/sample_player | grep libcrypto.so | awk '{ print $3 }')
LIB_PTHREAD_PATH=$(ldd $BUILD_PWD/sample_player | grep libpthread.so | awk '{ print $3 }')
LIB_DL_PATH=$(ldd $BUILD_PWD/sample_player | grep libdl.so | awk '{ print $3 }')

echo "LIBRCSC_PATH=" $LIBRCSC_PATH
echo "LIBZ_PATH=" $LIBZ_PATH
echo "LIBSTDCPP_PATH=" $LIBSTDCPP_PATH
echo "LIBM_PATH=" $LIBM_PATH
echo "LIBGCC_PATH=" $LIBGCC_PATH
echo "LIBC_PATH=" $LIBC_PATH
echo "LIB_THRIFT_PATH=" $LIB_THRIFT_PATH
echo "LIB_SSL_PATH=" $LIB_SSL_PATH
echo "LIB_CRYPTO_PATH=" $LIB_CRYPTO_PATH
echo "LIB_PTHREAD_PATH=" $LIB_PTHREAD_PATH
echo "LIB_DL_PATH=" $LIB_DL_PATH

echo "Start to create app image for player"
/opt/linuxdeploy/AppRun --appdir ./$PLAYER_APP_IMAGE_DIR_NAME \
                                -e $BUILD_PWD/sample_player \
                                -l $LIBRCSC_PATH \
                                -l $LIB_THRIFT_PATH \
                                -d $APP_IMAGE_DIR/sample_player.desktop \
                                -i $APP_IMAGE_DIR/sample_player.png \
                                --output appimage 

echo "Start to create app image for coach"
/opt/linuxdeploy/AppRun --appdir ./$COACH_APP_IMAGE_DIR_NAME \
                                -e $BUILD_PWD/sample_coach \
                                -l $LIBRCSC_PATH \
                                -l $LIB_THRIFT_PATH \
                                -d $APP_IMAGE_DIR/sample_coach.desktop \
                                -i $APP_IMAGE_DIR/sample_coach.png \
                                --output appimage 

echo "Start to create app image for trainer"
/opt/linuxdeploy/AppRun --appdir ./$TRAINER_APP_IMAGE_DIR_NAME \
                                -e $BUILD_PWD/sample_trainer \
                                -l $LIBRCSC_PATH \
                                -l $LIB_THRIFT_PATH \
                                -d $APP_IMAGE_DIR/sample_trainer.desktop \
                                -i $APP_IMAGE_DIR/sample_trainer.png \
                                --output appimage 
echo "App Image Created."

echo "Start to create all in one."
cp ${BUILD_PWD} -r soccer-simulation-proxy
rm  soccer-simulation-proxy/sample_player
rm  soccer-simulation-proxy/sample_coach
rm  soccer-simulation-proxy/sample_trainer
mv samplecoach-x86_64.AppImage soccer-simulation-proxy/sample_coach
mv sampleplayer-x86_64.AppImage soccer-simulation-proxy/sample_player
mv sampletrainer-x86_64.AppImage soccer-simulation-proxy/sample_trainer

chmod 777 soccer-simulation-proxy/*

rm -rf $PLAYER_APP_IMAGE_DIR_NAME
rm -rf $COACH_APP_IMAGE_DIR_NAME
rm -rf $TRAINER_APP_IMAGE_DIR_NAME

# create tar file
tar -czvf soccer-simulation-proxy.tar.gz soccer-simulation-proxy/*

# create zip file
if [ -x "$(command -v zip)" ]; then
    zip -r soccer-simulation-proxy.zip soccer-simulation-proxy/*
fi

# create 7z file
if [ -x "$(command -v 7z)" ]; then
    7z a soccer-simulation-proxy.7z soccer-simulation-proxy/*
fi

echo "All in one created."