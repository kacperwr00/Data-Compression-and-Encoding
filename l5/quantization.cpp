#include "lbg.hpp"

#include <cstring>

int main(int argc, char** argv)
{
    bool removeEmptyRegions = false, removeHybrid = false;
    if (argc != 4)
    {
        if (argc != 5)
        {
            std::cout << "Usage: ./quantization inputFilePath outputFilePath colorBits removeEmptyRegions" << std::endl;
            return -1;        
        }
        if (strcmp(argv[4], "NonEmpty") == 0)
            removeEmptyRegions = true;
        else if (strcmp(argv[4], "Hybrid") == 0)
            removeHybrid = true;
    }

    int colorBits = atoi(argv[3]);
    if (colorBits < 0 || colorBits > COLOR_COMPONENTS * sizeof(uint8_t) * 8)
    {
        std::cout << "Usage: ./quantization inputFilePath outputFilePath colorBits" << std::endl;
        std::cout << "Color count should be between 0 and " << COLOR_COMPONENTS * sizeof(uint8_t) * 8 << std::endl;
        return -1;            
    }

    image* img = new image();
    if (!parseImage(argv[1], img))
    {
        std::cout << "Error parsing inputImage" << std::endl;
        return -1; 
    }


    image* outputImg = new image();
    initImage(img->height, img->width, argv[2], outputImg);

    std::vector<uint8_t> vector = lgb(img, outputImg, 0.001, colorBits, removeEmptyRegions, removeHybrid);
    // printCodebook(vector);

    calculateMseSnr(outputImg, img);

    FILE* output = fopen(argv[2], "w");

    writeImageToFile(output, outputImg, argv[1]);


    fclose(output);
    deleteImage(img);
    deleteImage(outputImg);

    return 0;
}