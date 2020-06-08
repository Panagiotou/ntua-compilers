#!/bin/sh

if [ "$1" != "" ]; then
    echo "Compiling $1"
    ./pcl < $1 > output.ll || exit 1
    llc output.ll -o output.s
    clang output.s lib.a -o output.out
fi
