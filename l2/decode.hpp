#pragma once

#include "sharedLib.hpp"


#define ALPHABET_SIZE 256
#define PRECISION 52

uint8_t intervalSelection(const double value, double* leftBound, double* rightBound, const unsigned* c)
{
    uint8_t s = ALPHABET_SIZE - 1;
    double delta = *rightBound / c[ALPHABET_SIZE];
    double x = *leftBound + delta * c[s];
    double y = *leftBound + *rightBound;

    while (x > value)
    {
        s--;
        y = x;
        x = *leftBound + delta * c[s];
    }

    *leftBound = x;
    *rightBound = y - *leftBound;
    
    return s;
}

// uint8_t intervalSelection(const double value, double* leftBound, double* rightBound, const unsigned* c)
// {
//     uint8_t s = ALPHABET_SIZE - 1;
//     // double delta = *rightBound / c[ALPHABET_SIZE];
//     double x = *leftBound + c[s] * *rightBound / c[ALPHABET_SIZE];
//     double y = *leftBound + *rightBound;

//     while (x > value)
//     {
//         s--;
//         y = x;
//         x = *leftBound + c[s] * *rightBound / c[ALPHABET_SIZE];
//     }

//     *leftBound = x;
//     *rightBound = y - *leftBound;
    
//     return s;
// }

// second pseudocode from sayid implementation below - stops working as C values get large

// uint8_t intervalSelection(const double value, double* leftBound, double* rightBound, const unsigned* c)
// {
//     uint8_t s = ALPHABET_SIZE - 1;
//     double delta = *rightBound / c[ALPHABET_SIZE];
//     double scaledCodeValue = (value - *leftBound) / delta;
//     // double scaledCodeValue = (value - *leftBound) * (c[ALPHABET_SIZE] / *rightBound);

//     while (c[s] > scaledCodeValue)
//     {
//         s--;
//     }

//     //SACRIFICE SPEED FOR NUMERIC SAFETY
//     *leftBound += delta * c[s];
//     *rightBound = delta * (c[s + 1] - c[s]);

//     // *leftBound += c[s] * (*rightBound) / c[ALPHABET_SIZE]; 
//     // *rightBound = (c[s + 1] - c[s]) * (*rightBound) / c[ALPHABET_SIZE];
    
//     return s;
// }

void decoderRenormalization(double* value, double* leftBound, double* rightBound, int* bitCounter, bool* d)
{
    while (*rightBound <= 0.5)
    {
        if (*leftBound >= 0.5)
        {
            *leftBound = 2 * (*leftBound - 0.5);
            *value = 2 * (*value - 0.5);
        }
        else
        {
            *leftBound *= 2;
            *value *= 2;
        }

        *bitCounter += 1;
        if (d[*bitCounter])
            *value += pow(2, -(PRECISION));

        *rightBound *= 2;
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
void arithmeticDecoder(unsigned* c, bool* d, int charCount, char* result)
{
    double leftBound = 0.0;
    double rightBound = 1.0;
    int bitCounter = PRECISION;

    int updateCounter = 0;
    uint8_t characters[CUPDATE_PERIOD];

    double value = 0.0;
    for (int n = 1; n <= PRECISION; n++)
    {
        if (d[(n)])
            value += pow(2, -n);
    }

    for (int k = 0; k < charCount; k++)
    {
        result[k] = intervalSelection(value, &leftBound, &rightBound, c);

        if (leftBound >= 1.0)
        {
            leftBound -= 1; 
            value -= 1;
        }
        if (rightBound  <= 0.5)
        {
            decoderRenormalization(&value, &leftBound, &rightBound, &bitCounter, d);
        }

        characters[updateCounter++] = result[k];
        if (updateCounter & CUPDATE_PERIOD)
        {
            updateDistribution(characters, c);
            updateCounter = 0;
        }
    }
}


char* decode(encoded* encoded)
{
    unsigned* c = new unsigned[ALPHABET_SIZE + 1];
    for (int i = 0; i <= ALPHABET_SIZE; i++)
    {
        c[i] = i;
    }
    char* resultBuf = new char[MAX_TEXT_LENGTH];

    arithmeticDecoder(c, encoded->code, encoded->originalCharCount, resultBuf);

    if (resultBuf[encoded->originalCharCount - 1] < 0)
    {
        resultBuf[encoded->originalCharCount - 1] = '\n';
        resultBuf[encoded->originalCharCount - 2]++;
    }

    delete[](c);

    return resultBuf;
}
