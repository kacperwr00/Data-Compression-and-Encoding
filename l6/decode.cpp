#include "decode.hpp"

#include <cstring>

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cout << "Usage: ./quantization inputFilePath outputFilePath" << std::endl;
        return -1;        
    }

    decodeImage(argv[1], argv[2]);

    return 0;
}