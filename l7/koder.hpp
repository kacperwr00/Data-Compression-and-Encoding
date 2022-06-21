#include <iostream>
#include <cstdlib>

#define LOWER_HALF(x) (x & 0x0F)
#define UPPER_HALF(x) (x & 0xF0)

//ignores lastbit of input
inline uint8_t parityBit(uint8_t input)
{
    bool result = 0;
    for (int i = 7; i > 0; i--)
    {
        result ^= (bool)(input & (1 << i));
    }
    return result;
}

//ignores first 4 bits of input; assumes input < 16
inline uint8_t hammingCalc(uint8_t bits)
{
    const bool d1 = bits & 8, 
                d2 = bits & 4, 
                d3 = bits & 2, 
                d4 = bits & 1;

    const uint8_t tmp = (d1 << 7) + ((d1 ^ d2) << 6) + ((d2 ^ d3) << 5) + ((d1 ^ d3 ^ d4) << 4) 
                            + ((d2 ^ d4) << 3) + (d3 << 2) + (d4 << 1);

    return tmp + parityBit(tmp);
}

//encodes inFile with hamming(7, 4) with additional parity bit detecting double bit flips
void hammingEncodeFile(char* inFileName, char* outFileName)
{
    FILE* inFile = fopen(inFileName, "r");
    FILE* outFile = fopen(outFileName, "wb");

    int tmp;
    while ((tmp = fgetc(inFile)) != EOF)
    {
        uint8_t byte = tmp;

        uint8_t code[2]; 
        code[0] = hammingCalc(UPPER_HALF(byte) >> 4);
        code[1] = hammingCalc(LOWER_HALF(byte));
        
        fwrite(&code, sizeof(*code), 2, outFile);
    }

    fclose(inFile);
    fclose(outFile);
}