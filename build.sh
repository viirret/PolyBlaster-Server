#!/bin/bash

if [ -d "build" ]; then
	rm -rf build
fi

mkdir build
cd build
cmake ..
make

mkdir -p $HOME/PolyBlaster

cp ./x $HOME/PolyBlaster/server
