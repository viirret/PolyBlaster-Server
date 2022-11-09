#!/bin/bash

if [ ! -d "build" ]; then
	rm -rf build
	mkdir build
fi

cd build
cmake ..
make
cd ..

build/x 2>&1 | tee data.txt

