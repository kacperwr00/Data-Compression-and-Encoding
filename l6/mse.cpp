#include "mse.hpp"

int main(int argc, char** argv)
{
    if (argc == 3)
    {
        calculateMseSnr(argv[1], argv[2]);
        return 0;
    }
    
    std::cout << "Usage: ./mse filepathA filepathB" << std::endl;
    return -1;
}