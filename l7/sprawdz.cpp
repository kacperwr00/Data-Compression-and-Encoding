//USAGE sprawdz in1 in2
#include "sprawdz.hpp"

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cout << "Error parsing arguments" << std::endl;
        std::cout << "USAGE: sprawdz in out" << std::endl;
        return -1;
    }

    checkBlocks(argv[1], argv[2]);

    return 0;
}