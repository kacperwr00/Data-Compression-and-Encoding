#pragma once
#include "image.hpp"
#include <cstdint>
#include <vector>
#include <cmath>
#include <random>

#define THREAD_COUNT 8

//Mersenne twister rng
std::mt19937 rng;
//uniform distribution on [0, 255] range
std::uniform_int_distribution<uint8_t> uint_dist(0, UINT8_MAX);

std::pair<std::vector<uint8_t>, std::vector<uint8_t>> findYandZ(image* img, uint8_t j)
{
    unsigned size = img->width * img->height / 2;
    std::vector<uint8_t> y(size, 0);
    std::vector<uint8_t> z(size, 0);


    
        // y.reserve(size);
        // z.reserve(size);
    

    //invert encoding
    
        y[0] = img->bmp[j];
        z[0] = img->bmp[COLOR_COMPONENTS + j];
        uint8_t currentY = y[0];

        // printf("%d")

        for (int i = 1; i < size; i++)
        {
            y[i] = img->bmp[(i * 2) * COLOR_COMPONENTS + j] + currentY;
            currentY = y[i];
            z[i] = img->bmp[(i * 2 + 1) * COLOR_COMPONENTS + j];
        }
    

    return std::make_pair(y, z);
} 

void decodeImage(const char* encodedFileName, const char* outputFileName)
{
    image* img = new image();
    if (!parseImage(encodedFileName, img))
    {
        std::cout << "Error parsing inputImage" << std::endl;
        return; 
    }
    std::vector<uint8_t> y[COLOR_COMPONENTS];
    std::vector<uint8_t> z[COLOR_COMPONENTS];
    
    for (int i = 0; i < COLOR_COMPONENTS; i++)
    {
        auto result = findYandZ(img, i);
        y[i] = result.first;
        z[i] = result.second;
    }

    if ((img->width % 2) && (img->height % 2))
    {
        //TODO
    }

    for(int j = 0; j < COLOR_COMPONENTS; j++)
    {
        for (int i = 0; i < img->width * img->height; i += 2)
        {
            // printf("i = %d, i / 2 = %d, j = %d\n", i, i / 2, j);
            // auto tmp =
            auto a = y[j][i / 2];
            auto b = z[j][i / 2];

            img->bmp[i * COLOR_COMPONENTS + j] = uint8_t(a - b);
            if ((i + 1) * COLOR_COMPONENTS + j >= img->width * img->height * COLOR_COMPONENTS)
            {
                printf("%d %d %d", (i + 1) * COLOR_COMPONENTS + j, (i + 1) * COLOR_COMPONENTS + j, img->width * img->height * COLOR_COMPONENTS);
            }
            img->bmp[(i + 1) * COLOR_COMPONENTS + j] = uint8_t(a + b);
        }
    }

    writeImageToFile(outputFileName, img, encodedFileName);

    deleteImage(img);
}


//use already sorted areas to assign colors to pixels
void createOutputImage(int* pixelToAreas, std::vector<uint8_t> codebook, unsigned height, unsigned width, const image* outputImg)
{
    int pixelCount = height * width;

    #pragma omp parallel for
    for(int i = 0; i < pixelCount; i++)
    {
        for (int j = 0; j < COLOR_COMPONENTS; j++)
        {
            outputImg->bmp[i * COLOR_COMPONENTS + j] = codebook[pixelToAreas[i] + j];
        }
    }
}


//for debugging
void printCodebook(std::vector<uint8_t> codebook)
{
    for (int i = 0; i < codebook.size(); i += COLOR_COMPONENTS)
    {
        for (int j = 0; j < COLOR_COMPONENTS; j++)
        {
            printf("%d ", codebook[i + j]);
        }
        printf("\n");
    }
}

//first and second should be pointers to the first components of the colors
inline uint16_t pixelTaxicabDistance(uint8_t* first, uint8_t* second)
{
    uint16_t result = 0;

    for (int i = 0; i < COLOR_COMPONENTS; i++)
    {
        bool tmp = (first[i] > second[i]);
        result += tmp * (first[i] - second[i]) + !tmp * (second[i] - first[i]); 
    }

    return result;
}

// inline uint16_t intsqrt(uint16_t input)
// {
//     uint16_t candidate = input >> 1;
//
//     while (candidate * candidate > input)
//     {
//         candidate >>= 1;
//     }
//     while (candidate * candidate < input)
//     {
//         candidate++;
//     }
//     // std::cout << input << " " << candidate << std::endl;
//     return --candidate;
// }
//
// inline uint16_t pixelTaxicabDistance(uint8_t* first, uint8_t* second)
// {
//     uint16_t result = 0;
//
//     for (int i = 0; i < COLOR_COMPONENTS; i++)
//     {
//         int tmp = (int)first[i] - (int)second[i];
//         tmp *= tmp;
//         // printf("%d %d %d\n", first[i], second[i], tmp);
//         result += tmp; 
//     }
//
//     return intsqrt(result);
// }

//isn't divided by n yet
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


void calculateMseSnr(image* outputImage, image* inputImage)
{
    int pixelCount = outputImage->height * outputImage->width;
    double mseAcc = 0.0, snrAcc = 0.0;

    #pragma omp parallel for
    for(int i = 0; i < pixelCount; i++)
    {
        double tmp1 = mseElement(&outputImage->bmp[i], &inputImage->bmp[i]);
        double tmp2 = snrElement(&outputImage->bmp[i]);

        #pragma omp atomic
        mseAcc += tmp1;

        #pragma omp atomic
        snrAcc += tmp2;
    }

    mseAcc /= (pixelCount * COLOR_COMPONENTS);
    snrAcc /= (pixelCount * COLOR_COMPONENTS);
    snrAcc /= mseAcc;

    std::cout << "File: " << inputImage->fileName << " mse: " << mseAcc << " snr: " << snrAcc << std::endl;
}


std::vector<uint8_t> initLbg(const image* img)
{
    uint16_t centroid[COLOR_COMPONENTS];

    for (int i = 0; i < img->width; i++)
    {
        for (int j = 0; j < img->height; j++)
        {
            for (int k = 0; k < COLOR_COMPONENTS; k++)
            {
                centroid[k] += img->bmp[(i * img->height + j) * COLOR_COMPONENTS + k];
                centroid[k] >>= 1;
            }   
        } 
    }

    std::vector<uint8_t> result;
    for (int k = 0; k < COLOR_COMPONENTS; k++)
    {
        result.push_back((uint8_t) centroid[k]);
    }     
    
    return result;
}

//codebook structure: {color1comp1, color1comp2, color1comp3, color2comp1, ...}
std::vector<uint8_t> lbgRound(std::vector<uint8_t> inputCodebook, const image* img, double epsilon, 
        bool removeEmptyRegions, const bool lastIteration, const image* outputImage)
{
    unsigned k = -1;
    double distortion, newDistortion = 1.0;
    
    int codebookSize = inputCodebook.size();
    int areaCount = codebookSize / COLOR_COMPONENTS; //should always be divisible
    
    int pixelCount = img->height * img->width;
    //for reusing finding regions here to translate colors in constant time (only used if last iteration, else compiler will optimize it out)
    int* pixelToArea;
    if (lastIteration)
    {
        pixelToArea = new int[pixelCount];
    }
    std::vector<std::vector<uint8_t>> areas;
    for (int i = 0; i < areaCount; i++)
    {
        areas.emplace_back(std::vector<uint8_t>());
    }

    #pragma omp parallel num_threads(THREAD_COUNT)
    {
        do 
        {
            k++;
            distortion = newDistortion;
            newDistortion = 0.0;
            
            //find the new centroids
            #pragma omp for 
            for (int i = 0; i < areaCount; i++)
            {
                uint16_t centroid[COLOR_COMPONENTS] = {};

                if (areas[i].size() > 1)
                {
                    for (int m = 0; m < areas[i].size(); m += COLOR_COMPONENTS)
                    {
                        for (int j = 0; j < COLOR_COMPONENTS; j++)
                        {
                            centroid[j] += areas[i][m + j];
                            centroid[j] >>= 1;
                        }   
                    }
                    for (int j = 0; j < COLOR_COMPONENTS; j++)
                    {
                        inputCodebook[i * COLOR_COMPONENTS + j] = centroid[j];
                    }
                }
            }


            //find new quantization regions
            #pragma omp for 
            for (int i = 0; i < areaCount; i++)
            {
                areas[i].clear();
            }
            #pragma omp for 
            for (int i = 0; i < pixelCount; i++)
            {
                int minDistIndex = -1;
                uint16_t minDist = UINT16_MAX;

                for (int j = 0; j < codebookSize; j += COLOR_COMPONENTS)
                {
                    uint16_t dist = pixelTaxicabDistance(&img->bmp[i * COLOR_COMPONENTS], &inputCodebook[j]);
                    if (dist < minDist)
                    {
                        minDist = dist;
                        minDistIndex = j / COLOR_COMPONENTS;
                    }
                }

                int tmp = i * COLOR_COMPONENTS;
                
                #pragma omp critical
                {
                    areas[minDistIndex].push_back(img->bmp[tmp++]);
                    areas[minDistIndex].push_back(img->bmp[tmp++]);
                    areas[minDistIndex].push_back(img->bmp[tmp]);
                }
                if (lastIteration)
                {
                    pixelToArea[i] = minDistIndex * COLOR_COMPONENTS;
                }
            }

            //calculate distortion
            #pragma omp for
            for (int j = 0; j < areas.size(); j++)
            {
                double avgDistortion = 0.0;
                if (areas[j].size() > 1)
                {
                    for (int k = 0; k < areas[j].size(); k += COLOR_COMPONENTS)
                    {
                        int tmp = mseElement(&areas[j][k], &inputCodebook[j * COLOR_COMPONENTS]);
                        #pragma omp atomic
                        avgDistortion += tmp;  
                    }

                    avgDistortion /= (areas[j].size() * COLOR_COMPONENTS);
                }
                #pragma omp atomic
                newDistortion += avgDistortion;
            }

            // remove empty regions if applies
            #pragma omp single
            {
                if (removeEmptyRegions)
                {
                    for (int j = areas.size() - 1; j >= 0; j--)
                    {
                        if (!(areas[j].size())) 
                        {
                            for (int l = COLOR_COMPONENTS - 1; l >= 0; l--)
                            {
                                inputCodebook.erase(inputCodebook.begin() + (j * COLOR_COMPONENTS) + l);
                            }
                            areas.erase(areas.begin() + j);
                        }
                    }
                    codebookSize = inputCodebook.size();
                    areaCount = codebookSize / COLOR_COMPONENTS; //should always be divisible
                }
            }
        } while (((distortion - newDistortion) / distortion) > epsilon);
    }


    if (lastIteration)
    {
        createOutputImage(pixelToArea, inputCodebook, img->height, img->width, outputImage);
        
        if (!removeEmptyRegions)
        {
            //count non empty regions - number of colors actually used in the output image
            std::sort(pixelToArea, pixelToArea + pixelCount);
            int uniqueCount = std::unique(pixelToArea, pixelToArea + pixelCount) - pixelToArea;
            std::cout << " Colors " << inputCodebook.size() / COLOR_COMPONENTS << " unique colors used for this image: " << uniqueCount << std::endl;
        }
        delete[] pixelToArea;
    }
    // printf("took %d iterations\n", k);
    return inputCodebook;
}

std::vector<uint8_t> lgb(const image* img, const image* output, double epsilon, const int colorBits, const bool removeEmptyColors, const bool removeHybrid)
{
    auto vector = initLbg(img);
    
    // std::vector<uint8_t> turbulenceVector = {10, -10, 10};
    // std::vector<uint8_t> turbulenceVector = {10, 10, 10};

    //hybrid mode will try to find half of the desired colors - we lose one bit for every pixel, but gain a lot of speed 
    //compared to nonempty variant - fast variant loses even 5 bits for 16 bit call
    if (removeHybrid)
    {
        int i = vector.size();
        //at least half of the colors first
        unsigned bound = 1 << (colorBits - 1);
        bound *= COLOR_COMPONENTS;
        while(i < bound)
        {
            //split
            for (int j = 0; j < i; j++)
            {
                vector.push_back(vector[j] + uint_dist(rng));
            }
        
            vector = lbgRound(vector, img, epsilon, true, false, nullptr);
            i = vector.size();
        }
        bound <<= 1;
        
        int bnd = vector.size();

        for (int j = 0; j < bnd && vector.size() < bound; j++)
        {
            vector.push_back(vector[j] + uint_dist(rng));
        }
        vector = lbgRound(vector, img, epsilon, false, true, output);
        
        return vector;
    }

    //random turbulanceVector
    int i = vector.size();
    unsigned bound = 1 << (colorBits - 1);
    bound *= COLOR_COMPONENTS;

    while(i < bound)
    {
        //split
        for (int j = 0; j < i; j++)
        {
            vector.push_back(vector[j] + uint_dist(rng));
        }
    
        vector = lbgRound(vector, img, epsilon, removeEmptyColors, false, nullptr);
        i = vector.size();
    }
    bound <<= 1;
    while (vector.size() < bound)
    {
        int bnd = vector.size();

        for (int j = 0; j < bnd && vector.size() < bound; j++)
        {
            vector.push_back(vector[j] + uint_dist(rng));
        }
        vector = lbgRound(vector, img, epsilon, removeEmptyColors, true, output);
    }
    
    return vector;
}



