//USAGE szum p in out

#include "szum.hpp"

int main(int argc, char** argv)
{
    if (argc != 4)
    {
        std::cout << "Error parsing arguments" << std::endl;
        std::cout << "USAGE: szum p in out" << std::endl;
        return -1;
    }

    addNoise(atof(argv[1]), argv[2], argv[3]);

    return 0;
}