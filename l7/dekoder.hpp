#include <iostream>
#include <cstdlib>

static long unsigned doubleErrorCounter;
// static long unsigned iteration;

inline uint8_t parityBit(uint8_t input)
{
    bool result = 0;
    for (int i = 7; i > 0; i--)
    {
        result ^= (bool)(input & (1 << i));
    }
    return result;
}

inline uint8_t hammingDecodeTransmitted(uint8_t code)
{
    //p1 = d1 + d2 + d4         1 + 2 + 8 = 11
    //p2 = d1 + d4 + d3         1 + 8 + 4 = 13
    //p3 = d2 + d4 + d3         2 + 8 + 4 = 14
    const bool b1 = code & 128, 
                b2 = code & 64, 
                b3 = code & 32, 
                b4 = code & 16, 
                b5 = code & 8, 
                b6 = code & 4, 
                b7 = code & 2,
                b8 = code & 1;
                
    // z1 = 3 + 5 + 6 + 7 <- bit positions
    // z2 = 2 + 4 + 5 + 6
    // z3 = 1 + 3 + 4 + 5
    const bool z1 = b3 ^ b5 ^ b6 ^ b7, 
                z2 = b2 ^ b4 ^ b5 ^ b6,
                z3 = b1 ^ b3 ^ b4 ^ b5;

    const bool correctParityBit = (parityBit(code) == b8);
    const uint8_t syndrome = (z1 << 2) + (z2 << 1) + z3;

    //syndrome = 7 -> b5
    //syndrome = 6 -> b6
    //syndrome = 5 -> b3
    //syndrome = 4 -> b7
    //syndrome = 3 -> b4
    //syndrome = 2 -> b2
    //syndrome = 1 -> b1


    if (!syndrome && correctParityBit)
    {
        return (b1 << 3) + ((b2 ^ b1) << 2) + (b6 << 1) + b7;
    }

    //error detected
    if (!correctParityBit)
    {
        //correct the single bit flip

        switch(syndrome)
        {
            case 1:
                return (!b1 << 3) + ((b2 ^ !b1) << 2) + (b6 << 1) + b7;
            case 2:
                return (b1 << 3) + ((!b2 ^ b1) << 2) + (b6 << 1) + b7;
            case 6:
                return (b1 << 3) + ((b2 ^ b1) << 2) + (!b6 << 1) + b7;
            case 4:
                return (b1 << 3) + ((b2 ^ b1) << 2) + (b6 << 1) + !b7;
            default:
                //one of bits got corrupted but we arent using it to calculate value - no need to correct anything
                //or three bit flips - we cant do anything about it either
                return (b1 << 3) + ((b2 ^ b1) << 2) + (b6 << 1) + b7;
        }
    }

    //two bit flips detected - something is not right, yet parity checks out
    //if more then two errors "undefined behaviour"
    // std::cout << iteration << std::endl;
    doubleErrorCounter++;
    //we can't correct this error 
    //hopefully two p bits flipped - data would still be correct then
    return (b1 << 3) + ((b2 ^ b1) << 2) + (b6 << 1) + b7;
}

void hammingDecodeFile(char* inFileName, char* outFileName)
{
    FILE* inFile = fopen(inFileName, "rb");
    FILE* outFile = fopen(outFileName, "w");
    doubleErrorCounter = 0;
    // iteration = 0;

    int tmp;
    while ((tmp = fgetc(inFile)) != EOF)
    {
        uint8_t byte = tmp;
        // iteration++;
        uint8_t original = hammingDecodeTransmitted(byte) << 4;
        //if decoding a file encoded with ./koder - byte count should be even
        byte = fgetc(inFile);
        original += hammingDecodeTransmitted(byte);
        
        fwrite(&original, sizeof(original), 1, outFile);
    }

    std::cout << "Dekodując plik " << inFileName << " do " << outFileName << " napotkano " \
                << doubleErrorCounter << " podwójnych błędów w pojedynczym bloku." << std::endl;

    fclose(inFile);
    fclose(outFile);
}