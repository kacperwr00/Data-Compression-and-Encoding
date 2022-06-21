#include <iostream>
#include <cstdlib>

#define LOWER_HALF(x) (x & 0x0F)
#define UPPER_HALF(x) (x & 0xF0)

void checkBlocks(char* inOneFileName, char* inTwoFileName)
{
    FILE* inOneFile = fopen(inOneFileName, "r");
    FILE* inTwoFile = fopen(inTwoFileName, "r");
    long unsigned differentBlockCounter = 0;

    int tmpOne, tmpTwo;
    while ((tmpOne = fgetc(inOneFile)) != EOF && (tmpTwo = fgetc(inTwoFile)) != EOF)
    {
        uint8_t byteOne = (uint8_t)tmpOne, byteTwo = (uint8_t)tmpTwo;

        //branchless implementation that requires false = 0 and true = 1
        differentBlockCounter += (LOWER_HALF(byteOne) != LOWER_HALF(byteTwo));
        differentBlockCounter += (UPPER_HALF(byteOne) != UPPER_HALF(byteTwo));
    }

    //because of lazy && we didnt read the EOF from second file yet
    if (!feof(inTwoFile))
    {
        fgetc(inTwoFile);
        if (feof(inTwoFile))
        {
            std::cout << "Plik " << inOneFileName << " i " << inTwoFileName << " różnią się na " \
                << differentBlockCounter << " 4-bitowych blokach." << std::endl;

            fclose(inOneFile);
            fclose(inTwoFile);
            return;
        }
    }

    //error ocurred during reading files or they were different lengths
    std::cout << "Warning: one or more files still contain bytes" << std::endl;

    std::cout << "Plik " << inOneFileName << " i " << inTwoFileName << " różnią się na " \
                << differentBlockCounter << " 4-bitowych blokach." << std::endl;

    fclose(inOneFile);
    fclose(inTwoFile);
}