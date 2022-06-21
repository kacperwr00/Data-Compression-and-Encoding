#include "encoding.hpp"

int main(int argc, char** argv)
{
    image* img = new image();
    if (parseImage(argv[1], img))
    {
        // encoding 

        // calculateEntropy
        // printBitmap(img);
        // printBitmap(img);
        findBestEncoding(img);


        // deleteImage(img);
    }
    else
    {
        std::cout << "Error parsing image" << std::endl;
        return -1;
    }
    
    return 0;
}