#!/bin/bash

CLAY_VERSION=${CLAY_VERSION:-0.13}
CLAY_RELEASE=https://github.com/nicbarker/clay/releases/download/v$CLAY_VERSION/clay.h
CLAY_MAIN=https://raw.githubusercontent.com/nicbarker/clay/refs/heads/main/clay.h

URL=$CLAY_RELEASE
if [ "$(echo $@ | grep main)" != "" ]; then
    URL=$CLAY_MAIN
fi

echo "Downloading from $URL"
wget --output-document clay.h "$URL"