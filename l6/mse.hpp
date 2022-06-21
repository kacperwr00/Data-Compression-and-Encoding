#pragma once

#include "image.hpp"

inline double mseElement(uint8_t* first, uint8_t* second)
{
    int result = 0;

    for (int i = 0; i < COLOR_COMPONENTS; i++)
    {
        int tmp = first[i] - second[i];
        result += (tmp * tmp); 
    }

    return (double)result;
}

inline double mseSingle(uint8_t first, uint8_t second)
{
    int tmp = first - second;
    return (double)(tmp * tmp);
}

inline double snrSingle(uint8_t first)
{
    return (double)(first * first);
}

inline double snrElement(uint8_t* first)
{
    int result = 0;

    for (int i = 0; i < COLOR_COMPONENTS; i++)
    {
        int tmp = first[i];
        result += (tmp * tmp); 
    }

    return (double)result;
}

void calculateMseSnr(const char* filepathA, const char* filepathB)
{
    image* outputImage = new image();
    image *inputImage = new image();

    if (parseImage(filepathB, outputImage) && parseImage(filepathA, inputImage))
    {
        int pixelCount = outputImage->height * outputImage->width;
        double mseAcc = 0.0, snrAcc = 0.0;
        double mseColorsAcc[COLOR_COMPONENTS] = {};
        double snrColorsAcc[COLOR_COMPONENTS] = {};

        #pragma omp parallel for
        for(int i = 0; i < pixelCount; i++)
        {
            double tmp1 = mseElement(&outputImage->bmp[i * COLOR_COMPONENTS], &inputImage->bmp[i * COLOR_COMPONENTS]);
            double tmp2 = snrElement(&outputImage->bmp[i * COLOR_COMPONENTS]);

            // if (tmp1 > 64000)
            // std::cout << "i = " << i << " mse = " << tmp1 << std::endl;

            #pragma omp atomic
            mseAcc += tmp1;

            #pragma omp atomic
            snrAcc += tmp2;

            for (int j = 0; j < COLOR_COMPONENTS; j++)
            {
                double tmp3 = mseSingle(outputImage->bmp[i * COLOR_COMPONENTS + j], inputImage->bmp[i * COLOR_COMPONENTS + j]);
                #pragma omp atomic
                mseColorsAcc[j] += tmp3;
            }

            for (int j = 0; j < COLOR_COMPONENTS; j++)
            {
                double tmp3 = snrSingle(outputImage->bmp[i * COLOR_COMPONENTS + j]);
                #pragma omp atomic
                snrColorsAcc[j] += tmp3;
            }
        }

        mseAcc /= (pixelCount * COLOR_COMPONENTS);
        snrAcc /= (pixelCount * COLOR_COMPONENTS);
        snrAcc /= mseAcc;
        for (int i = 0; i < COLOR_COMPONENTS; i++)
        {
            mseColorsAcc[i] /= pixelCount;
            snrColorsAcc[i] /= pixelCount;
            snrColorsAcc[i] /= mseColorsAcc[i]; 
        }

        std::cout << "File: " << inputImage->fileName << std::endl << " Mse: " << std::endl << mseAcc << std::endl;
        
        for (int i = 0; i < COLOR_COMPONENTS; i++)
        {
            std::cout << mseColorsAcc[i] << " ";
        }

        std::cout << std::endl << " Snr: " << snrAcc << std::endl;
        for (int i = 0; i < COLOR_COMPONENTS; i++)
        {
            std::cout << snrColorsAcc[i] << " ";
        }
        std::cout << std::endl;
        

        

        return;
    }

    std::cout << "Error parsing images: " << filepathA << " or " << filepathB << std::endl;
}