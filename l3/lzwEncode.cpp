#include <cstring>

#include "universalEncoding.hpp"
#include "lzw.hpp"

int main(int argc, char** argv)
{
    if (argc < 3)
    {        
        std::cout << "Usage: ./programName inputPath outputPath [OPTIONS]" << std::endl;
    }

    UniversalEncoder* encoder = new UniversalEncoder();
    for (int i = 3; i < argc; i++)
    {
        if (!strcmp(argv[i], "--gamma"))
        {
            encoder->setEncoding(UniversalEncoder::EncodingType::Gamma);
        }
        else if (!strcmp(argv[i], "--delta"))
        {
            encoder->setEncoding(UniversalEncoder::EncodingType::Delta);
        }
        else if (!strcmp(argv[i], "--omega"))
        {
            encoder->setEncoding(UniversalEncoder::EncodingType::Omega);
        }
        else if (!strcmp(argv[i], "--fib"))
        {
            encoder->setEncoding(UniversalEncoder::EncodingType::Fibonacci);
        }
    }

    lzwCompress(argv[1], argv[2], encoder);

    delete(encoder);
    return 0;
}