#!/bin/bash
# dependencies (in ubuntu 24.04 installation):
# * clang++ version 18.1.3
# * libglm-dev/noble,noble,now 0.9.9.8+ds-7
# * libtbb-dev/noble,now 2021.11.0-2ubuntu2
# * libsdl2-dev/noble,now 2.30.0+dfsg-1build3
# * libsdl2-gfx-dev/noble,now 1.0.4+dfsg-5build1
# * libsdl2-image-dev/noble,now 2.8.2+dfsg-1build2
# * libsdl2-ttf-dev/noble,now 2.22.0+dfsg-1
# * libgl-dev/noble,now 1.7.0-1build1
set -e
cd $(dirname "$0")

BIN="glosi"
#CC="g++ -std=c++23 -Wno-changes-meaning"
CC="clang++ -std=c++23"  # "-Xclang -fdump-record-layouts"
SRC="src/main.cpp"
CFLAGS="-mavx -Wfatal-errors $(sdl2-config --cflags)"
LIBS="-ltbb -lGL -lSDL2_image -lSDL2_ttf $(sdl2-config --libs)"
WARNINGS="-Wall -Wextra -Wpedantic \
    -Wshadow -Wconversion -Wsign-conversion \
    -Wno-unused-variable -Wno-unused-function -Wno-unused-parameter"
OPTIMIZATION="-O3"
DEBUG="-g"
if [[ "$1" == "release" ]]; then
    DEBUG=""
fi
PROFILE=""
if [[ "$1" == "profile" ]]; then
    PROFILE="-pg"
fi
LDFLAGS=""
if [[ "$1" == "sanitize1" ]]; then
    LDFLAGS="-fsanitize=address,undefined -fsanitize-address-use-after-scope"
fi
if [[ "$1" == "sanitize2" ]]; then
    # note: run > MSAN_OPTIONS=halt_on_error=0 ./glosi
    LDFLAGS="-fsanitize=memory,undefined -fno-omit-frame-pointer -fsanitize-address-use-after-scope"
fi
if [[ "$1" == "sanitize3" ]]; then
    LDFLAGS="-fsanitize=thread"
fi

CMD="$CC -o $BIN $SRC $DEBUG $PROFILE $OPTIMIZATION $CFLAGS $LDFLAGS $WARNINGS $LIBS"
echo $CMD
$CMD
echo
ls -la --color $BIN
echo
