#include "encodeRownomiernie.hpp"

int main(int argc, char** argv)
{
    bool removeEmptyRegions = false, removeHybrid = false;
    if (argc != 4)
    {
        std::cout << "Usage: ./encode inputFilePath outputFilePath quantizationBits" << std::endl;
        return -1;
    }

    int colorBits = atoi(argv[3]);
    if (colorBits < 0 || colorBits > sizeof(uint8_t) * 8)
    {
        std::cout << "Usage: ./encode inputFilePath outputFilePath quantizationBits" << std::endl;
        std::cout << "Color count should be between 0 and " << sizeof(uint8_t) * 8 << std::endl;
        return -1;            
    }

    encodeImage(argv[2], argv[1], colorBits);

    return 0;
}