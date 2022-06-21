#pragma once
#include "universalEncoding.hpp"
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <cmath>
//max 100 MB - our tests are circa 1MB
#define MAX_TEXT_LENGTH 800000000
#define MAX_ENCODEDED_LEN 256
#define ALPHABET_SIZE 256

int countCharacterOccurances(int* characterCount, const char* pathToFile)
{
    FILE* inputFile = fopen(pathToFile, "rb");

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

//prints bool buffer into output as bytes
void fprintfLzw(const char* outputPath, bool* buffer, unsigned long bufferLen)
{
    FILE* output = fopen(outputPath, "wb");

    int i = 0;
    uint8_t current = 0;
    while (i < bufferLen)
    {
        current = 0;
        for (int j = 0; j < 7; j++)
        {
            if (i + j < bufferLen && buffer[i+j])
            {
                current += 1;
            }
            current <<= 1;
        }
        if (i + 7 < bufferLen && buffer[i+7])
        {
            current += 1;
        }

        fwrite(&current, sizeof(current), 1, output);

        i += 8;
    }

    fclose(output);
}

void lzwCompress(const char* inputPath, const char* outputPath, UniversalEncoder* encoder)
{
    std::unordered_map<std::string, bool*> table;
    FILE* input = fopen(inputPath, "rb");
    bool* buffer = new bool[MAX_TEXT_LENGTH];
    uint64_t* bufferIndex = new uint64_t;
    *bufferIndex = 0;

    if (buffer == nullptr)
    {
        std::cout << "Could not allocate buffer" << std::endl;
        return;
    }

    // std::cout << inputPath << "\t\t" << outputPath << std::endl;

    for (int i = 0; i < ALPHABET_SIZE; i++) 
    {
        table[std::string(1, (char)i)] = encoder->universalEncode(i + 1);
    }

    int tmp = fgetc(input);
    std::string p = {(char)tmp}, c;

    int current = 257;
    while ((tmp = fgetc(input)) != EOF)
    {
        c = {(char)(tmp)};

        if (table.find(p + c) != table.end())
        {
            p = p + c;
        }
        else
        {
            encoder->sprintfEncodedAuto(table[p], buffer, bufferIndex);
            table[p + c] = encoder->universalEncode(current++);
            p = c;
        }
    }
    encoder->sprintfEncodedAuto(table[p], buffer, bufferIndex);
    fclose(input);
    for (auto record: table)
    {
        delete[](record.second);
    }

    int* charOccurances = new int[ALPHABET_SIZE]();
    int len = countCharacterOccurances(charOccurances, inputPath);
    double entropy = calculateEntropy(charOccurances, len);
    std::cout << "Input: " << inputPath << ", długość: " << len << ", entropia: " << entropy << std::endl;

    fprintfLzw(outputPath, buffer, *bufferIndex);

    for (int i = 0; i < ALPHABET_SIZE; i++)
    {
        charOccurances[i] = 0;
    }
    int len2 = countCharacterOccurances(charOccurances, outputPath);
    entropy = calculateEntropy(charOccurances, len2);
    std::cout << "Output: " << outputPath << ", długość: " << len2 << ", entropia: " << entropy
         << ", stopień kompresji: " << (1 - (double)len2 / len) * 100 << "%." << std::endl << std::endl;

    delete[](buffer);
    delete[](charOccurances);


    encoder->deleteFibCache();
}

void lzwDecompress(const char* inputPath, const char* outputPath, UniversalEncoder* encoder)
{
    std::unordered_map<uint64_t, std::string> table;
    FILE* input = fopen(inputPath, "rb");
    FILE* output = fopen(outputPath, "w");

    for (int i = 0; i < ALPHABET_SIZE; i++) 
    {
        table[i + 1] = std::string(1, char(i));
    }

    int tmp;
    int current = 257;
    bool* buffer = new bool[MAX_ENCODEDED_LEN];
    int bufferIndex = 0;
    while (bufferIndex < MAX_ENCODEDED_LEN - 8 && (tmp = fgetc(input)) != EOF)
    {
        for (int k = 7; k >= 0; k--)
        {
            buffer[bufferIndex++] = (tmp & (1 << k));
        }
    }
    uint64_t firstSymbol = encoder->universalDecode(buffer), secondSymbol;

    int l = encoder->getLastSymbolLength();
    if (l == MAX_ENCODEDED_LEN)
    {
        printf("Erorr");
    }
    for (int i = 0; i < MAX_ENCODEDED_LEN - l; i++)
    {
        buffer[i] = buffer[i + l];
    }
    bufferIndex -= l;

    std::string firstTranslation = table[firstSymbol];
    std::string c = {firstTranslation[0]};

    fwrite(firstTranslation.c_str(), sizeof(char), firstTranslation.length(), output);

    while (!feof(input))
    {
        //keep MAX_ENCODED_LEN inside buffer, then decode one number and restore buffer
        while (bufferIndex < MAX_ENCODEDED_LEN - 8 && (tmp = fgetc(input)) != EOF)
        {
            for (int k = 7; k >= 0; k--)
            {
                buffer[bufferIndex++] = (tmp & (1 << k));
            }
        }

        //decode one symbol
        secondSymbol = encoder->universalDecode(buffer);
        //remove it from buffer
        l = encoder->getLastSymbolLength();
        if (l >= MAX_ENCODEDED_LEN)
        {
            printf("Erorr");
        }
        for (int i = 0; i < MAX_ENCODEDED_LEN - l; i++)
        {
            buffer[i] = buffer[i + l];
        }
        bufferIndex -= l;

        //obsluz go
        if (firstSymbol == 0 || secondSymbol == 0)
        {
            std::cout << "alert";
        }

        if (table.find(secondSymbol) == table.end())
        {
            //read code not in the dictionary
            firstTranslation = table[firstSymbol] + c;
        }
        else
        {
            firstTranslation = table[secondSymbol];
        }
        
        c = {firstTranslation[0]};
        table[current++] = table[firstSymbol] + c;
        firstSymbol = secondSymbol;
        fwrite(firstTranslation.c_str(), sizeof(char), firstTranslation.length(), output);
    }

    //obsluz to co pozostalo jeszcze w buffer
    while (bufferIndex > 7)
    {
        secondSymbol = encoder->universalDecode(buffer);
        //remove it from buffer
        l = encoder->getLastSymbolLength();
        buffer += l;
        bufferIndex -= l;

        //obsluz go
        if (table.find(secondSymbol) == table.end())
        {
            //read code not in the dictionary
            firstTranslation = table[firstSymbol] + c;
        }
        else
        {
            firstTranslation = table[secondSymbol];
        }
        c = {firstTranslation[0]};
        table[current++] = table[firstSymbol] + c;
        firstSymbol = secondSymbol;
        fwrite(firstTranslation.c_str(), sizeof(char), firstTranslation.length(), output);
    }
    //16 might not be correcct here but works empirically for the examples I tested
    if (firstTranslation[0] == char(0) && firstTranslation.length() > 16 && firstTranslation.length() < 512)
    {
        fwrite(&firstTranslation[0], sizeof(char), 1, output);
    }
    
    fclose(input);
    fclose(output);
}
