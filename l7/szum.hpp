#include <iostream>
#include <cstdlib>
#include <random>

void addNoise(double p, char* inFileName, char* outFileName)
{
    //seeded for deterministic results
    std::mt19937 rng(123);
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    FILE* inFile = fopen(inFileName, "rb");
    FILE* outFile = fopen(outFileName, "wb");

    int tmp;
    while ((tmp = fgetc(inFile)) != EOF)
    {
        uint8_t byte = tmp;

        for (int i = 0; i < 8; i++)
        {
            if (dist(rng) < p)
            {
                byte ^= (1 << i); 
            }
        }
        fwrite(&byte, sizeof(byte), 1, outFile);
    }

    fclose(inFile);
    fclose(outFile);
}
