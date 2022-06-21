#include <stdio.h>
#include <math.h>
#include <iostream>
#include <map>

#include <stdio.h>



int main(int argc, char const *argv[])
{
    std::map<uint8_t, int> characterFrequency;
    std::map<std::pair<uint8_t, uint8_t>, int> conditionalCharacterFrequency;

    if (argc != 2)
    {
        return -1;
    }

    FILE* inputFile = fopen(argv[1], "r");
    uint8_t curChar, lastChar = 0;
    int temp;
    int totalCharCount = 0;

    while ((temp = fgetc(inputFile)) != EOF)
    {
        curChar = (uint8_t)temp;
        characterFrequency[curChar]++;
        conditionalCharacterFrequency[std::make_pair(lastChar, curChar)]++;

        lastChar = curChar;
        totalCharCount++;
    }

    if (totalCharCount == 0)
    {
        std::cout << argv[1] << ": etropia: 0, etropia warunkowa: 0" << std::endl;
    }

    double entropy = 0.0, conditionalEntropy = 0.0;
    double logTotalCharCount = log2(totalCharCount);
    

    for (int i = 0; i <= 255; i++)
    {
        auto tmp = characterFrequency.find(i);
        if (tmp != characterFrequency.end())
        {
            entropy += (*tmp).second * (logTotalCharCount - log2((*tmp).second));
        }
    }
    entropy /= totalCharCount;

    for (int i = 0; i <= 255; i++)
    {
        double logXi = log2(characterFrequency[i]);
        for (int j = 0; j <= 255; j++)
        {
            auto tmp = conditionalCharacterFrequency.find(std::make_pair(i, j));
            if (tmp != conditionalCharacterFrequency.end())
            {
                conditionalEntropy += (*tmp).second * (logXi - log2((*tmp).second));
            }
        }
    }
    conditionalEntropy /= totalCharCount;

    std::cout << argv[1] << ": entropia: " << entropy << ", entropia warunkowa: " << conditionalEntropy << std::endl;

    printf("entropia warunkowa %.24f \n", conditionalEntropy);

    fclose(inputFile);
    return 0;
}
