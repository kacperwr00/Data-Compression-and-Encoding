#!/bin/bash

# USAGE: ./test.sh [p] [inputFileName] [outputFileWithNoExtension]
# ex: ./test.sh 0.001 testy/pan-tadeusz-czyli-ostatni-zajazd-na-litwie.txt pt
# ex: ./test.sh 0.001 testy/test1.bin t1

./koder $2 encoded/$3.bin
./szum $1 encoded/$3.bin noisy/$3.bin
./dekoder noisy/$3.bin decoded/$3.txt
./sprawdz $2 decoded/$3.txt