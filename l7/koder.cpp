//USAGE koder in out
#include "koder.hpp"

//whole project assumes false = 0 and true = 1 for speed and simplicity
int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cout << "Error parsing arguments" << std::endl;
        std::cout << "USAGE: koder in out" << std::endl;
        return -1;
    }

    hammingEncodeFile(argv[1], argv[2]);

    return 0;
}