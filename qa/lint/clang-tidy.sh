#!/bin/sh
# tools:
#   clang-tidy: Ubuntu LLVM version 15.0.7
set -e
cd $(dirname "$0")

SRC="../../src/main.cpp"

date | tee clang-tidy.log
clang-tidy --config-file=clang-tidy.cfg $SRC -- -std=c++23 | tee -a clang-tidy.log
date | tee -a clang-tidy.log

