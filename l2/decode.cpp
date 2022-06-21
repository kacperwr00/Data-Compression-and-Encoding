#include <iostream>
#include "encode.hpp"
#include "decode.hpp"

int main(int argc, char** argv) 
{
    if (argc == 3)
    {
        FILE* inputFile = fopen(argv[1], "rb");

        encoded* result = new encoded;

        size_t read;

        read = fread(&result->originalCharCount, sizeof(result->originalCharCount), 1, inputFile);
        read = fread(&result->encodedCharCount, sizeof(result->encodedCharCount), 1, inputFile);

        result->code = new bool[result->encodedCharCount]();

        int i = 0;
        uint8_t current = 0;
        while (i < result->encodedCharCount)
        {
            read = fread(&current, sizeof(current), 1, inputFile);
            if (read == 0)
            {
                return -1;
            }

            for (int j = 7; j >= 0; j--)
            {
                if (current & (1 << j))
                {
                    result->code[i+(7-j)] = true;
                }
            }

            i += 8;
        }

        fclose(inputFile);

        char* decoded = decode(result);

        FILE* decodedOutputFile = fopen(argv[2], "w");
        fwrite(decoded, sizeof(*decoded), result->originalCharCount, decodedOutputFile);
        fclose(decodedOutputFile);

        delete[](decoded);
        delete[](result->code);
        delete(result);

        return 0;
    }
    return -1;
}