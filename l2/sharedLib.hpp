#pragma once

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define ALPHABET_SIZE 256
//should be a power of two
#define CUPDATE_PERIOD 2 * ALPHABET_SIZE
//max 10 MB - our tests are circa 1MB
#define MAX_TEXT_LENGTH 80000000

typedef struct encoded {
    int encodedCharCount;
    int originalCharCount;
    bool* code;
} encoded;

int countCharacterOccurances(int* characterCount, const char* pathToFile)
{
    FILE* inputFile = fopen(pathToFile, "r");
    uint8_t curChar;
    int temp;
    int totalCharCount = 0;

    while ((temp = fgetc(inputFile)) != EOF)
    {
        curChar = (uint8_t)temp;
        characterCount[curChar]++;
        totalCharCount++;
    }

    return totalCharCount;
}

double calculateEntropy(int characterCount[ALPHABET_SIZE], int totalCharacterCount)
{
    double entropy = 0.0;
    double logTotalCharCount = log2(totalCharacterCount);

    for (int i = 0; i < ALPHABET_SIZE; i++)
    {
        int tmp = characterCount[i];
        if (tmp != 0)
        {
            entropy += tmp * (logTotalCharCount - log2(tmp));
        }
    }
    
    return entropy / totalCharacterCount;
}

void updateDistribution(const uint8_t characters[CUPDATE_PERIOD], unsigned* c)
{
    for (int i = 0; i < CUPDATE_PERIOD; i++)
    {
        uint8_t character = characters[i];
        for (int m = character + 1; m <= ALPHABET_SIZE; m++)
        {
            c[m] += 1;
        }
    }
}


void updateDistribution(const uint8_t character, unsigned* c)
{
    for (int m = character + 1; m <= ALPHABET_SIZE; m++)
    {
        c[m] += 1;
    }
}
