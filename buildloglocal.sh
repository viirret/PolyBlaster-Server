#!/bin/bash

if [ -d "build" ]; then
	rm -rf build
fi

mkdir build
cd build
cmake ..
make

cd ..

build/x 2>&1 | tee localdata.txt






