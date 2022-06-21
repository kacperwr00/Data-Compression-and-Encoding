#!/bin/bash

g++ -Wall -Wextra -pedantic -O3 -std=c++11 -g -o encode encode.cpp
g++ -Wall -Wextra -pedantic -O3 -std=c++11 -g -o decode decode.cpp

./encode testy/test1.bin encoded/encodedt1
./encode testy/test2.bin encoded/encodedt2
./encode testy/test3.bin encoded/encodedt3
./encode testy/pan-tadeusz-czyli-ostatni-zajazd-na-litwie.txt encoded/encodedpt
./encode testy/my.txt encoded/encodedmy

./decode encoded/encodedt1 decoded/decodedt1
./decode encoded/encodedt2 decoded/decodedt2
./decode encoded/encodedt3 decoded/decodedt3
./decode encoded/encodedpt decoded/decodedpt
./decode encoded/encodedmy decoded/decodedmy

echo "diff test1:"
diff testy/test1.bin decoded/decodedt1
echo "diff test2:"
diff testy/test2.bin decoded/decodedt2
echo "diff test3:"
diff testy/test3.bin decoded/decodedt3
echo "diff Pan Tadeusz:"
diff testy/pan-tadeusz-czyli-ostatni-zajazd-na-litwie.txt decoded/decodedpt

echo "diff mytest:"
diff decoded/decodedmy testy/my.txt