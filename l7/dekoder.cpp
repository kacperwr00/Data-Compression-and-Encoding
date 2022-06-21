//USAGE koder in out

#include "dekoder.hpp"

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cout << "Error parsing arguments" << std::endl;
        std::cout << "USAGE: dekoder in out" << std::endl;
        return -1;
    }

    hammingDecodeFile(argv[1], argv[2]);

    return 0;
}