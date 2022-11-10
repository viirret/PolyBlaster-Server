#!/bin/bash

if [ ! -d "build" ]; then
	rm -rf build
	mkdir build
fi

cd build
cmake ..
make
cd ..

rm data.txt 2>/dev/null

build/x 2>&1 | tee data.txt

