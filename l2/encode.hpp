#pragma once

#include "sharedLib.hpp"


void intervalUpdate(const uint8_t character, double* leftBound, double* rightBound, const unsigned* c)
{
    double delta = *rightBound / c[ALPHABET_SIZE];
    double y;
    if (character == ALPHABET_SIZE - 1)
    {
        y = *leftBound + *rightBound;
    }
    else
    {
        y = *leftBound + delta * c[character + 1];
    }

    *leftBound += delta * c[character];
    *rightBound = y - *leftBound;
}

// void intervalUpdate(const uint8_t character, double* leftBound, double* rightBound, const unsigned* c)
// {
//     // double delta = *rightBound / c[ALPHABET_SIZE];
//     double y;
//     if (character == ALPHABET_SIZE - 1)
//     {
//         y = *leftBound + *rightBound;
//     }
//     else
//     {
//         y = *leftBound + c[character + 1] * *rightBound / c[ALPHABET_SIZE];
//     }
//     *leftBound += c[character] * *rightBound / c[ALPHABET_SIZE];
//     *rightBound = y - *leftBound;
// }

void propagateCarry(const int bitCounter, bool* d)
{
    int n = bitCounter;
    while (d[n])
    {
        d[n--] = false;
    }
    d[n] = true;
}

void encoderRenormalization(double* leftBound, double* rightBound, int* bitCounter, bool* d)
{
    while (*rightBound <= 0.5)
    {
        *rightBound *= 2;
        *bitCounter += 1;

        if (*leftBound >= 0.5)
        {
            d[*bitCounter] = true;
            *leftBound = 2 * (*leftBound - 0.5);
        }
        else
        {
            d[*bitCounter] = false;
            *leftBound *= 2;
        }
    }
}

void codeValueSelection(double* leftBound, int* bitCounter, bool* d)
{
    *bitCounter += 1;

    if (*leftBound <= 0.5)
    {
        d[*bitCounter] = true;
    }
    else 
    {
        d[*bitCounter] = false;
        propagateCarry(*bitCounter - 1, d);
    }
}



// N - liczba znaków do zakodowania
// M - InputAlphabetSize
// S - ciąg znaków do zakodowania
// D - OutputAlphabetSize
// b - lewa granica interwału, B = D^P * b
// l, L analogicznie prawa
// C(s) = D^P * c(s)
// c(s) = sum from 0 to M -1 (p(s))
// p(s) prawdopodobieństwo, że k-ty symbol to 
// d - output buffer?
// if a coding method generates the sequence of bits 0011000101100, then we have
// Code sequence d = 0011000101100
// Code value v = 0.0011000101100 _2 = 0.19287109375
int arithmeticEncoder(FILE* inputFile, unsigned* c, bool* d)
{
    double leftBound = 0.0;
    double rightBound = 1.0;
    int bitCounter = 0;

    int updateCounter = 0;
    uint8_t characters[CUPDATE_PERIOD];

    int temp;
    while ((temp = fgetc(inputFile)) != EOF)
    {
        uint8_t currentCharacter = temp;

        intervalUpdate(currentCharacter, &leftBound, &rightBound, c);

        if (leftBound >= 1)
        {
            leftBound -= 1;
            propagateCarry(bitCounter, d);
        }
        if (rightBound <= 0.5)
        {
            encoderRenormalization(&leftBound, &rightBound, &bitCounter, d);
        }
        characters[updateCounter++] = currentCharacter;
        if (updateCounter & CUPDATE_PERIOD)
        {
            updateDistribution(characters, c);
            updateCounter = 0;
        }
    }
    codeValueSelection(&leftBound, &bitCounter, d);

    return bitCounter;
}

encoded* encode(char* fileName)
{
    FILE* inputFile = fopen(fileName, "r");
    encoded* result = new encoded;
    bool* d = new bool[MAX_TEXT_LENGTH];
    unsigned* c = new unsigned[ALPHABET_SIZE + 1];
    
    for (int i = 0; i <= ALPHABET_SIZE; i++)
    {
        c[i] = i;
    }

    result->encodedCharCount = arithmeticEncoder(inputFile, c, d);

    result->code = new bool[result->encodedCharCount];
    for (int i = 0; i < result->encodedCharCount; i++)
    {
        result->code[i] = d[i];
    }

    fclose(inputFile);
    delete[](c);
    delete[](d);

    return result;
}
