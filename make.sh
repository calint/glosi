#!/bin/bash
#
# dependencies in arch linux installation:
# * clang++ 20.1.8
# * mesa 1:25.2.3-1
# * glm 1.0.1
# * sdl3 3.2.22-1
# * sdl3_ttf 3.2.2-3
# * sdl3_image 3.4.0-1
# * intel-oneapi-tbb 2021.12.0-2
#
set -e
cd $(dirname "$0")

BIN="glosi"

COMPILER="clang" # "gcc"" or "clang"

if [[ "$COMPILER" == "gcc" ]]; then
    CC="g++ -std=c++26 -Wno-changes-meaning -flifetime-dse=1"
    # note: -flifetime-dse=1 : workaround for issue when compiler optimizes away stores before new in place
    #       falsely assuming that all object fields are initialized by constructor resulting in crash
    WARNINGS="-Wall -Wextra -Wpedantic \
            -Wshadow -Wconversion -Wsign-conversion \
            -Wnull-dereference -Warray-bounds -Wdouble-promotion \
            -Wnon-virtual-dtor -Wformat -Wctor-dtor-privacy \
            -Woverloaded-virtual -Wcast-align -Wzero-as-null-pointer-constant \
            -Wdeprecated -Wdefaulted-function-deleted -Wmismatched-new-delete \
            -Wpessimizing-move -Wreturn-type -Wformat=2 \
            -Wno-unused-variable -Wno-unused-function -Wno-unused-parameter"

elif [[ "$COMPILER" == "clang" ]]; then
    CC="clang++ -std=c++26"
    WARNINGS="-Weverything \
            -Wno-c++98-compat -Wno-float-equal -Wno-covered-switch-default \
            -Wno-padded -Wno-global-constructors -Wno-exit-time-destructors \
            -Wno-weak-vtables -Wno-unsafe-buffer-usage -Wno-unused"
fi

SRC="src/main.cpp"
CFLAGS="-Wfatal-errors -Werror"
OPTIMIZATION="-O3"
# for SDL3 builds link against the new library names
LIBS="-ltbb -lGL -lSDL3 -lSDL3_image -lSDL3_ttf"
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
    LDFLAGS="-fsanitize=thread -DMODE_JTHREADS"
fi

CMD="$CC -o $BIN $SRC $DEBUG $PROFILE $OPTIMIZATION $CFLAGS $LDFLAGS $WARNINGS $LIBS"
echo $CMD
$CMD
echo
ls -la --color $BIN
echo
