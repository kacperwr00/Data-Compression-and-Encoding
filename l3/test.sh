#!/bin/bash
rm -r decoded
rm -r encoded

mkdir encoded
mkdir decoded


echo "fib"
./lzwEncode testy/test1.bin encoded/encodedt1 --fib
./lzwEncode testy/test2.bin encoded/encodedt2 --fib
./lzwEncode testy/test3.bin encoded/encodedt3 --fib
./lzwEncode testy/pan-tadeusz-czyli-ostatni-zajazd-na-litwie.txt encoded/encodedpt --fib
./lzwEncode testy/my.txt encoded/encodedmy --fib

./lzwDecode encoded/encodedt1 decoded/decodedt1.bin --fib
./lzwDecode encoded/encodedt2 decoded/decodedt2.bin --fib
./lzwDecode encoded/encodedt3 decoded/decodedt3.bin --fib
./lzwDecode encoded/encodedpt decoded/decodedpt --fib
./lzwDecode encoded/encodedmy decoded/decodedmy --fib

echo "diff test1:"
diff testy/test1.bin decoded/decodedt1.bin
echo "diff test2:"
diff testy/test2.bin decoded/decodedt2.bin
echo "diff test3:"
diff testy/test3.bin decoded/decodedt3.bin
echo "diff Pan Tadeusz:"
diff testy/pan-tadeusz-czyli-ostatni-zajazd-na-litwie.txt decoded/decodedpt

echo "diff mytest:"
diff decoded/decodedmy testy/my.txt