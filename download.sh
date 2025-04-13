#!/bin/bash

# ------------------------------------------
# CLAY
# ------------------------------------------

CLAY_VERSION=${CLAY_VERSION:-0.13}
CLAY_RELEASE=https://github.com/nicbarker/clay/releases/download/v$CLAY_VERSION/clay.h
CLAY_MAIN=https://raw.githubusercontent.com/nicbarker/clay/refs/heads/main/clay.h

if [ "$(echo $@ | grep clay-main)" != "" ]; then
    CLAY_URL=$CLAY_MAIN
elif [ "$(echo $@ | grep clay-release)" != "" ]; then
    CLAY_URL=$CLAY_RELEASE
fi

if [ ! -z $CLAY_URL ]; then
    echo "Downloading from $CLAY_URL"
    wget --output-document clay.h "$CLAY_URL"
fi

# ------------------------------------------
# SDL2
# ------------------------------------------

mkdir -p __tmp__
cd __tmp__

SDL2_ARCH=x86_64-w64-mingw32 #x86_64-w64-mingw32 #i686-w64-mingw32
SDL2_VERSION=2.32.4
SDL2_RELEASE=https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_VERSION}/SDL2-devel-${SDL2_VERSION}-mingw.zip

SDL2_TTF_VERSION=2.24.0
SDL2_TTF_RELEASE=https://github.com/libsdl-org/SDL_ttf/releases/download/release-${SDL2_TTF_VERSION}/SDL2_ttf-devel-${SDL2_TTF_VERSION}-mingw.zip

SDL2_IMAGE_VERSION=2.8.8
SDL2_IMAGE_RELEASE=https://github.com/libsdl-org/SDL_image/releases/download/release-${SDL2_IMAGE_VERSION}/SDL2_image-devel-${SDL2_IMAGE_VERSION}-mingw.zip

wget -nc $SDL2_RELEASE
wget -nc $SDL2_TTF_RELEASE
wget -nc $SDL2_IMAGE_RELEASE

unzip -n SDL2-devel-${SDL2_VERSION}-mingw.zip
unzip -n SDL2_ttf-devel-${SDL2_TTF_VERSION}-mingw.zip
unzip -n SDL2_image-devel-${SDL2_IMAGE_VERSION}-mingw.zip

cd ..
mkdir -p build
cp __tmp__/SDL2-${SDL2_VERSION}/${SDL2_ARCH}/bin/SDL2.dll build/
cp __tmp__/SDL2_ttf-${SDL2_TTF_VERSION}/${SDL2_ARCH}/bin/SDL2_ttf.dll build/
cp __tmp__/SDL2_image-${SDL2_IMAGE_VERSION}/${SDL2_ARCH}/bin/SDL2_image.dll build/

mkdir -p external

cp -r __tmp__/SDL2-${SDL2_VERSION}/${SDL2_ARCH}/{include,lib} external/
cp -r __tmp__/SDL2_ttf-${SDL2_TTF_VERSION}/${SDL2_ARCH}/{include,lib} external/
cp -r __tmp__/SDL2_image-${SDL2_IMAGE_VERSION}/${SDL2_ARCH}/{include,lib} external/
