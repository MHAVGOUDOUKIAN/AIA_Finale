#!/bin/sh

cd ./build
cmake .
cmake --build . --config=Release
cmake --install .

cd ..
mv ./build/App ./App
