#!/bin/bash

# usage ./test.sh colorBits [filenameSuffix]
rm -r encoded
rm -r decoded

mkdir encoded 
mkdir decoded

./encode$2 testy/example0.tga encoded/example0-$1Colors$2 $1;
./encode$2 testy/example1.tga encoded/example1-$1Colors$2 $1;
./encode$2 testy/example2.tga encoded/example2-$1Colors$2 $1;
./encode$2 testy/example3.tga encoded/example3-$1Colors$2 $1;

./decode encoded/example0-$1Colors$2 decoded/example0-$1Colors$2.tga;# $1;
./decode encoded/example1-$1Colors$2 decoded/example1-$1Colors$2.tga;# $1;
./decode encoded/example2-$1Colors$2 decoded/example2-$1Colors$2.tga;# $1;
./decode encoded/example3-$1Colors$2 decoded/example3-$1Colors$2.tga;# $1;

./mse testy/example0.tga decoded/example0-$1Colors$2.tga;
./mse testy/example1.tga decoded/example1-$1Colors$2.tga;
./mse testy/example2.tga decoded/example2-$1Colors$2.tga;
./mse testy/example3.tga decoded/example3-$1Colors$2.tga;