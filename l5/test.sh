#!/bin/bash

# usage ./test.sh colorBits [empryRegionMode]

./quantization testy/example0.tga compressed/example0-$1Colors$2 $1 $2;
./quantization testy/example1.tga compressed/example1-$1Colors$2 $1 $2;
./quantization testy/example2.tga compressed/example2-$1Colors$2 $1 $2;
./quantization testy/example3.tga compressed/example3-$1Colors$2 $1 $2;