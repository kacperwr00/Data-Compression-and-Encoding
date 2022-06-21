#pragma once
#include <iostream>
#include <vector>
#include <stdint.h>

//limit on max calculated fibonacci number - 92 fit on 64bit unsigned 
//if we start counting from the second one like we do here
#define FIB_BOUND 92

//x86 and similar only
static inline uint64_t bsr(const uint64_t x) {
  uint64_t y;
  asm ( "\tbsr %1, %0\n"
      : "=r"(y)
      : "r" (x)
  );
  return y;
}

class UniversalEncoder 
{
private:
    //use elias omega encoding by default
    bool* (*encodingAlgorithm)(uint64_t input) = &eliasOmegaEncode;
    uint64_t(*decodingAlgorithm)(bool* input) = &eliasOmegaDecode;
    inline static int lastSymbolLength;

    //cachedFib[0] contains second Fib number
    inline static uint64_t* cachedFib;

    static bool* eliasGammaEncode(uint64_t input)
    {
        bool* res;
        if (input == 0)
        {
            std::cout << "Error: Cannot encode zero, returning 0" << std::endl;
            return 0;
        }

        //using x86 bsr which is 0 based counting system
        int n = bsr(input) + 1;

        // 0-initialize res
        res = new bool[n * 2 - 1]();

        uint64_t one = 1;

        for (uint64_t i = 0; i < n; i++)
        {
            //start setting ones in res from the end
            res[n * 2 - 2 - i] = (input & (one << i));
        }
        lastSymbolLength = n * 2 - 1;
        return res;
    }

    static uint64_t eliasGammaDecode(bool* input)
    {
        uint64_t res = 0;
        int n = 1, i = 0;
        
        while(!input[i])
        {
            n++;
            i++;
        }
        if (n > sizeof(res) * 8)
        {
            std::cout << "Error: Encoded number " << n << " is too big to cast to long long unsigned, returning 0" << std::endl;
            return 0;
        }

        uint64_t one = 1;

        for (int j = 0; j < n; j++)
        {
            if (input[i++])
            {
                res += (one << (n - j - 1));
            }
        }
        
        lastSymbolLength = n * 2 - 1;
        return res;
    }

    static bool* eliasDeltaEncode(uint64_t input)
    {
        bool* res;
        if (input == 0)
        {
            std::cout << "Error: Cannot encode zero, returning 0" << std::endl;
            return 0;
        }

        //using x86 bsr which is 0 based counting system
        int n = bsr(input) + 1;
        int k = bsr(n) + 1;

        // 0-initialize res
        res = new bool[k - 1 + k + n - 1]();

        uint64_t one = 1;

        for (uint64_t i = 0; i < n - 1; i++)
        {
            //start setting ones in res from the end (input)
            res[k - 1 + k + n - 2 - i] = (input & (one << i));
        }

        for (uint64_t i = 0; i < k; i++)
        {
            //start setting ones in res from the end (n)
            res[k - 1 + k + n - 2 - i - n + 1] = (n & (one << i));
        }
        
        lastSymbolLength = k - 1 + k + n - 1;
        return res;
    }
    
    static uint64_t eliasDeltaDecode(bool* input)
    {
        uint64_t res = 0;
        int k = 1, i = 0, n = 0;
        
        while(!input[i])
        {
            k++;
            i++;
        }

        uint64_t one = 1;

        for (int j = 0; j < k; j++)
        {
            if (input[i++])
            {
                n += (one << (k - j - 1));
            }
        }
        if (n > sizeof(res) * 8)
        {
            std::cout << "Error: Encoded number is too big to cast to long long unsigned, returning 0" << std::endl;
            return 0;
        }

        res += (one << (n - 1));
        for (int j = 0; j < n - 1; j++)
        {
            if (input[i++])
            {
                res += (one << (n - 1 - j - 1));
            }
        }

        lastSymbolLength = k - 1 + k + n - 1;;
        return res;
    }

    static bool* eliasOmegaEncode(uint64_t input)
    {
        //start with a zero
        bool* res = new bool[1]();
        int currentResLength = 1;
        uint64_t one = 1;

        while (input > 1)
        {
            //dopisz nowa liczbe na poczatek
            int n = bsr(input) + 1;
            bool* newRes = new bool[currentResLength + n];
            for (int i = 0; i < n; i++)
            {
                newRes[i] = (input & (one << (n - i - 1))); 
            }
            for (int i = n; i < currentResLength + n; i++)
            {
                newRes[i] = res[i - n];
            }
            input = n - 1;
            currentResLength += n;
            delete[](res);
            res = newRes;
        }

        lastSymbolLength = currentResLength;
        return res;
    }

    static uint64_t eliasOmegaDecode(bool* input)
    {
        uint64_t n = 1;

        int i = 0;
        uint64_t one = 1;

        while (input[i])
        {
            int k = n + 1;
            n = 0;
            for (int r = 0; r < k; r++)
            {
                if (input[i++])
                {
                    n += (one << (k - r - 1));
                }
            }
        }

        lastSymbolLength = i + 1;
        return n;
    }

    //cached fib needs to be zero-alloced before calling this
    static int largestPossibleFib(uint64_t input)
    {
        for (int i = 0; i < FIB_BOUND; i++)
        {
            if (getFib(i) > input)
            {
                return i - 1;
            }
        }

        std::cout << "Error: FIB_BOUND too low" << std::endl;
        return -1;
    }

    //fibonacci can only encode up to 12200160415121876737 - needs a bigger fib number that fits on uint64_t
    static bool* fibonacciEncode(uint64_t input)
    {
        int index = largestPossibleFib(input);

        bool* res = new bool[index + 2];
        //aditional '1' bit - end marker
        res[index + 1] = 1;
        lastSymbolLength = index + 2;
        
        while (input)
        {
            res[index] = 1;
            input -= getFib(index);

            for (int i = index - 1; i >= 0 && getFib(i) > input; )
            {
                res[i] = 0;
                index = --i;
            }
        }

        return res;
    }

    static uint64_t fibonacciDecode(bool* input)
    {
        uint64_t res = 0;
        bool shouldEnd = false;

        for (int i = 0; i <= FIB_BOUND; i++)
        {

            if (input[i])
            {
                if (shouldEnd)
                {
                    lastSymbolLength = i + 1;
                    return res;
                }

                res += getFib(i);
                shouldEnd = true;
            }
            else
            {
                shouldEnd = false;
            }
        }

        std::cout << "Error: FIB_BOUND too low" << std::endl;
        return 0;
    }

public:
    enum class EncodingType {Gamma, Delta, Omega, Fibonacci};

    void setEncoding(EncodingType encodingType)
    {
        switch (encodingType)
        {
            case EncodingType::Gamma:
                encodingAlgorithm = &eliasGammaEncode;
                decodingAlgorithm = &eliasGammaDecode;
                break;
            case EncodingType::Delta:
                encodingAlgorithm = &eliasDeltaEncode;
                decodingAlgorithm = &eliasDeltaDecode;
                break;
            case EncodingType::Omega:
                encodingAlgorithm = &eliasOmegaEncode;
                decodingAlgorithm = &eliasOmegaDecode;
                break;
            case EncodingType::Fibonacci:
                encodingAlgorithm = &fibonacciEncode;
                decodingAlgorithm = &fibonacciDecode;
                break;
            default:
                std::cout << "Wrong encoding type selected, returning";
                return;    
        }
    }

    void printEncodedGamma(bool* encoded)
    {
        int n = 1, i = 0;
        
        while(!encoded[i])
        {
            n++;
            i++;
            std::cout << "0";
        }

        for (int j = 0; j < n; j++)
        {
            std::cout << encoded[i++];
        }
        std::cout << std::endl;
    }

    void printEncodedDelta(bool* encoded)
    {
        int k = 1, i = 0, n = -1;
        
        while(!encoded[i])
        {
            k++;
            i++;
            std::cout << "0";
        }
        uint64_t one = 1;

        for (int j = 0; j < k; j++)
        {
            if (encoded[i])
                n += (one << (k - j - 1));
            std::cout << encoded[i++];
        }

        std::cout << "1";
        for (int j = 0; j < n; j++)
        {
            std::cout << encoded[i++];
        }
        std::cout << "\n";
    }

    void printEncodedOmega(bool* encoded)
    {
        uint64_t n = 1;

        int i = 0;
        uint64_t one = 1;

        while (encoded[i])
        {
            int k = n + 1;
            n = 0;
            for (int r = 0; r < k; r++)
            {
                if (encoded[i++])
                {
                    n += (one << (k - r - 1));
                    std::cout << "1";
                }
                else
                    std::cout << "0";
            }
        }
        std::cout << "0\n";
    }

    void printEncodedFibonacci(bool* encoded)
    {
        bool shouldEnd = false;
        
        for (int i = 0; i <= FIB_BOUND; i++)
        {
            if (encoded[i])
            {
                std::cout << "1";
                if (shouldEnd)
                {
                    std::cout << "\n";
                    return;
                }
                shouldEnd = true;
            }
            else
            {
                std::cout << "0";
                shouldEnd = false;
            }
        }

        std::cout << "Error: FIB_BOUND too low" << std::endl;
    }

    //can only be used if currently set algorithm was the one used to encode
    void printEncodedAuto(bool* encoded)
    {
        if (encodingAlgorithm == &eliasGammaEncode)
            printEncodedGamma(encoded);
        else if (encodingAlgorithm == &eliasDeltaEncode)
            printEncodedDelta(encoded);
        else if (encodingAlgorithm == &eliasOmegaEncode)
            printEncodedOmega(encoded);
        else if (encodingAlgorithm == &fibonacciEncode)
            printEncodedFibonacci(encoded);
    }

    void sprintfEncodedGamma(bool* encoded, bool* buffer, uint64_t* index)
    {
        int n = 1, i = 0;
        
        while(!encoded[i])
        {
            n++;
            i++;
            buffer[(*index)++] = 0;
        }

        for (int j = 0; j < n; j++)
        {
            buffer[(*index)++] = encoded[i++];
        }
    }

    void sprintfEncodedDelta(bool* encoded, bool* buffer, uint64_t* index)
    {
        int k = 1, i = 0, n = -1;
        
        while(!encoded[i])
        {
            k++;
            i++;
            buffer[(*index)++] = 0;
        }
        uint64_t one = 1;

        for (int j = 0; j < k; j++)
        {
            if (encoded[i])
                n += (one << (k - j - 1));
            buffer[(*index)++] = encoded[i++];
        }

        for (int j = 0; j < n; j++)
        {
            buffer[(*index)++] = encoded[i++];
        }
    }

    void sprintfEncodedOmega(bool* encoded, bool* buffer, uint64_t* index)
    {
        uint64_t n = 1;

        int i = 0;
        uint64_t one = 1;

        while (encoded[i])
        {
            int k = n + 1;
            n = 0;
            for (int r = 0; r < k; r++)
            {
                if (encoded[i++])
                {
                    n += (one << (k - r - 1));
                    buffer[(*index)++] = 1;
                }
                else
                    buffer[(*index)++] = 0;
            }
        }
        buffer[(*index)++] = 0;
    }

    void sprintfEncodedFibonacci(bool* encoded, bool* buffer, uint64_t* index)
    {
        bool shouldEnd = false;
        
        for (int i = 0; i <= FIB_BOUND; i++)
        {
            if (encoded[i])
            {
                buffer[(*index)++] = 1;
                if (shouldEnd)
                {
                    return;
                }
                shouldEnd = true;
            }
            else
            {
                buffer[(*index)++] = 0;
                shouldEnd = false;
            }
        }

        std::cout << "Error: FIB_BOUND too low" << std::endl;
    }

    //can only be used if currently set algorithm was the one used to encode
    void sprintfEncodedAuto(bool* encoded, bool* buffer, uint64_t* index)
    {
        if (encodingAlgorithm == &eliasGammaEncode)
            sprintfEncodedGamma(encoded, buffer, index);
        else if (encodingAlgorithm == &eliasDeltaEncode)
            sprintfEncodedDelta(encoded, buffer, index);
        else if (encodingAlgorithm == &eliasOmegaEncode)
            sprintfEncodedOmega(encoded, buffer, index);
        else if (encodingAlgorithm == &fibonacciEncode)
            sprintfEncodedFibonacci(encoded, buffer, index);
    }

    static uint64_t getFib(int index)
    {
        if (index >= FIB_BOUND)
        {
            std::cout << "Error: FIB_BOUND too low" << std::endl;
            return 0;
        }
        if (!cachedFib)
        {
            cachedFib = new uint64_t[FIB_BOUND]();
            cachedFib[0] = 1;
            cachedFib[1] = 2;
        }
        if (!cachedFib[index])
        {
            cachedFib[index] = getFib(index - 1) + getFib(index - 2);
        }
        return cachedFib[index];
    }
    
    void deleteFibCache()
    {
        if (cachedFib)
        {
            delete[](cachedFib);
        }
    }

    int getLastSymbolLength()
    {
        return lastSymbolLength;
    }

    bool* universalEncode(uint64_t input)
    {
        return (*encodingAlgorithm)(input);
    }

    uint64_t universalDecode(bool* input)
    {
        return (*decodingAlgorithm)(input);
    }
};