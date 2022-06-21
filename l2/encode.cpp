#include <iostream>

#include "encode.hpp"

int main(int argc, char** argv) 
{
    if (argc == 3)
    {
        int characterOccurances[ALPHABET_SIZE];
        for (int i = 0; i < ALPHABET_SIZE; i++)
        {
            characterOccurances[i] = 0;
        }
        int totalCharInFile = countCharacterOccurances(characterOccurances, argv[1]);

        encoded* result = encode(argv[1]);

        result->originalCharCount = totalCharInFile;

        std::cout << argv[1] << " - entropia: " << calculateEntropy(characterOccurances, totalCharInFile);

        FILE* encodedOutputFile = fopen(argv[2], "wb");

        fwrite(&result->originalCharCount, sizeof(result->originalCharCount), 1, encodedOutputFile);
        fwrite(&result->encodedCharCount, sizeof(result->encodedCharCount), 1, encodedOutputFile);

        int i = 0;
        uint8_t current = 0;
        while (i < result->encodedCharCount)
        {
            current = 0;
            for (int j = 0; j < 7; j++)
            {
                if (i + j < result->encodedCharCount && result->code[i+j])
                {
                    current += 1;
                }
                current <<= 1;
            }
            if (i + 7 < result->encodedCharCount && result->code[i+7])
            {
                current += 1;
            }

            fwrite(&current, sizeof(current), 1, encodedOutputFile);

            i += 8;
        }

        fclose(encodedOutputFile);

        std::cout << " - encryption rate: " << 
            (1 - (double)(sizeof(totalCharInFile) + result->encodedCharCount / 8 + sizeof(result->encodedCharCount)) / (totalCharInFile)) * 100 
            << "%." <<std::endl;

        
        return 0;   
    }
    
    return -1;
}