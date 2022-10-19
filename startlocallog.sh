#!/bin/bash

cd build
make
cd ..

build/x 2>&1 | tee localdata.txt
