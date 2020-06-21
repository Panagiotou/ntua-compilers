#!/bin/sh

if [ "$1" != "" ]; then
    echo "Compiling $1"
    ./pcl < $1 > output.ll
    if [ $? -eq 0 ]; then
      echo OK
    else
      echo FAIL
      ./pcl < $1
      exit 1
    fi
    llc output.ll -o output.s
    clang output.s lib.a -o output.out
    echo "Executing output"
    ./output.out
else
  echo "No Input"
fi
