#pragma once
#include "image.hpp"
#include <cstdint>
#include <vector>
#include <cmath>
#include <random>
#include <set>

#define THREAD_COUNT 8

#define AVG(a, b) (a + b) / 2
#define STDDEV(a, b) (a - b) / 2

//Mersenne twister rng
std::mt19937 rng;
//uniform distribution on [0, 255] range
std::uniform_int_distribution<uint8_t> uint_dist(0, UINT8_MAX);
std::uniform_int_distribution<int16_t> uint_dist2(-UINT8_MAX, UINT8_MAX);

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
    printf("Codebook of size %ld:\n", codebook.size());        
    for (int i = 0; i < codebook.size(); i++)
    {
        printf("%d ", codebook[i]);        
    }
    printf("\n");
}

void printCodebook(std::vector<int16_t> codebook)
{
    printf("Codebook of size %ld:\n", codebook.size());        
    for (int i = 0; i < codebook.size(); i++)
    {
        printf("%d ", codebook[i]);        
    }
    printf("\n");
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

inline double mseSingle(uint8_t first, uint8_t second)
{
    int tmp = first - second;
    return (double)(tmp * tmp);
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


std::vector<uint8_t> initLbg(std::vector<uint8_t> img, unsigned pixelCount)
{
    uint16_t centroid;

    for (int i = 0; i < pixelCount; i++)
    {
        centroid += img[i];
        centroid >>= 1; 
    }

    std::vector<uint8_t> result;
    result.push_back((uint8_t) centroid);
    
    return result;
}

//codebook structure: {color1comp1, color1comp2, color1comp3, color2comp1, ...}
std::vector<uint8_t> lbgRound(std::vector<uint8_t> inputCodebook, std::vector<uint8_t> img, unsigned pixelCount, double epsilon, 
        const bool removeEmptyRegions)
{
    unsigned k = -1;
    double distortion, newDistortion = 1.0;
    
    int codebookSize = inputCodebook.size();
    int areaCount = codebookSize; //should always be divisible
    
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
            
            //find the new centroid
            #pragma omp for 
            for (int i = 0; i < areaCount; i++)
            {
                uint16_t centroid = 0;

                if (areas[i].size() > 1)
                {
                    for (int m = 0; m < areas[i].size(); m++)
                    {
                        centroid += areas[i][m];
                        centroid >>= 1;
                    }
                    inputCodebook[i] = centroid;
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

                for (int j = 0; j < codebookSize; j++)
                {
                    uint16_t dist;

                    if (img[i] >= inputCodebook[j])
                        dist = img[i] - inputCodebook[j];
                    else 
                        dist = inputCodebook[j] - img[i];

                    if (dist < minDist)
                    {
                        minDist = dist;
                        minDistIndex = j;
                    }
                }

                #pragma omp critical
                {
                    areas[minDistIndex].push_back(img[i]);
                }
            }

            //calculate distortion
            #pragma omp for
            for (int j = 0; j < areas.size(); j++)
            {
                double avgDistortion = 0.0;
                if (areas[j].size() > 1)
                {
                    for (int k = 0; k < areas[j].size(); k++)
                    {
                        int tmp = mseSingle(areas[j][k], inputCodebook[j]);
                        #pragma omp atomic
                        avgDistortion += tmp;  
                    }

                    avgDistortion /= (areas[j].size());
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
                            inputCodebook.erase(inputCodebook.begin() + j);
                            areas.erase(areas.begin() + j);
                        }
                    }
                    codebookSize = inputCodebook.size();
                    areaCount = codebookSize;
                }
            }
        } while (((distortion - newDistortion) / distortion) > epsilon);
    }

    return inputCodebook;
}

std::vector<uint8_t> lbg(std::vector<uint8_t> img, unsigned imgLength, double epsilon, const int colorBits)
{
    auto vector = initLbg(img, imgLength);
    
    // std::vector<uint8_t> turbulenceVector = {10, -10, 10};
    // std::vector<uint8_t> turbulenceVector = {10, 10, 10};
    //
    //hybrid mode will try to find half of the desired colors - we lose one bit for every pixel, but gain a lot of speed 
    //compared to nonempty variant - fast variant loses even 5 bits for 16 bit call
    // if (removeHybrid)
    // {
    //     int i = vector.size();
    //     //at least half of the colors first
    //     unsigned bound = 1 << (colorBits - 1);
    //     bound *= COLOR_COMPONENTS;
    //     while(i < bound)
    //     {
    //         //split
    //         for (int j = 0; j < i; j++)
    //         {
    //             vector.push_back(vector[j] + uint_dist(rng));
    //         }
    //
    //         vector = lbgRound(vector, img, epsilon, true, false, nullptr);
    //         i = vector.size();
    //     }
    //     bound <<= 1;
    //
    //     int bnd = vector.size();
    //
    //     for (int j = 0; j < bnd && vector.size() < bound; j++)
    //     {
    //         vector.push_back(vector[j] + uint_dist(rng));
    //     }
    //     vector = lbgRound(vector, img, epsilon, false, true, output);
    //
    //     return vector;
    // }

    //random turbulanceVector
    int i = vector.size();
    unsigned bound = 1 << (colorBits - 1);
    unsigned iterationsWithoutImprovement = 0;

    while(i < bound && iterationsWithoutImprovement < 1000)
    {
        //split
        for (int j = 0; j < i; j++)
        {
            vector.push_back(vector[j] + uint_dist(rng));
        }
    
        vector = lbgRound(vector, img, imgLength, epsilon, true);
        i = vector.size();

        // std::cout << "Iterations " << i << std::endl;

        if (i != vector.size())
        {
            i = vector.size();
            iterationsWithoutImprovement = 0;
        }
        else
        {
            iterationsWithoutImprovement++;
        }

    }
    bound <<= 1;
    while (vector.size() < bound)
    {
        int bnd = vector.size();

        for (int j = 0; j < bnd && vector.size() < bound; j++)
        {
            vector.push_back(vector[j] + uint_dist(rng));
        }
        vector = lbgRound(vector, img, imgLength, epsilon, false);
    }
    
    return vector;
}


std::vector<int16_t> initLbg(std::vector<int16_t> img, unsigned pixelCount)
{
    uint16_t centroid;

    for (int i = 0; i < pixelCount; i++)
    {
        centroid += img[i];
        centroid >>= 1; 
    }

    std::vector<int16_t> result;
    result.push_back((int16_t) centroid);
    
    return result;
}

//codebook structure: {color1comp1, color1comp2, color1comp3, color2comp1, ...}
std::vector<int16_t> lbgRound(std::vector<int16_t> inputCodebook, std::vector<int16_t> img, unsigned pixelCount, double epsilon, 
        const bool removeEmptyRegions)
{
    unsigned k = -1;
    double distortion, newDistortion = 1.0;
    
    int codebookSize = inputCodebook.size();
    int areaCount = codebookSize; //should always be divisible
    
    std::vector<std::vector<int16_t>> areas;
    for (int i = 0; i < areaCount; i++)
    {
        areas.emplace_back(std::vector<int16_t>());
    }

    #pragma omp parallel num_threads(THREAD_COUNT)
    {
        do 
        {
            k++;
            distortion = newDistortion;
            newDistortion = 0.0;
            
            //find the new centroid
            #pragma omp for 
            for (int i = 0; i < areaCount; i++)
            {
                uint16_t centroid = 0;

                if (areas[i].size() > 1)
                {
                    for (int m = 0; m < areas[i].size(); m++)
                    {
                        centroid += areas[i][m];
                        centroid >>= 1;
                    }
                    inputCodebook[i] = centroid;
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

                for (int j = 0; j < codebookSize; j++)
                {
                    uint16_t dist;

                    if (img[i] >= inputCodebook[j])
                        dist = img[i] - inputCodebook[j];
                    else 
                        dist = inputCodebook[j] - img[i];

                    if (dist < minDist)
                    {
                        minDist = dist;
                        minDistIndex = j;
                    }
                }

                #pragma omp critical
                {
                    areas[minDistIndex].push_back(img[i]);
                }
            }

            //calculate distortion
            #pragma omp for
            for (int j = 0; j < areas.size(); j++)
            {
                double avgDistortion = 0.0;
                if (areas[j].size() > 1)
                {
                    for (int k = 0; k < areas[j].size(); k++)
                    {
                        int tmp = mseSingle(areas[j][k], inputCodebook[j]);
                        #pragma omp atomic
                        avgDistortion += tmp;  
                    }

                    avgDistortion /= (areas[j].size());
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
                            inputCodebook.erase(inputCodebook.begin() + j);
                            areas.erase(areas.begin() + j);
                        }
                    }
                    codebookSize = inputCodebook.size();
                    areaCount = codebookSize;
                }
            }
        } while (((distortion - newDistortion) / distortion) > epsilon);
    }

    return inputCodebook;
}

std::vector<int16_t> lbg(std::vector<int16_t> img, unsigned imgLength, double epsilon, const int colorBits)
{
    auto vector = initLbg(img, imgLength);
    
    // std::vector<int16_t> turbulenceVector = {10, -10, 10};
    // std::vector<int16_t> turbulenceVector = {10, 10, 10};
    //
    //hybrid mode will try to find half of the desired colors - we lose one bit for every pixel, but gain a lot of speed 
    //compared to nonempty variant - fast variant loses even 5 bits for 16 bit call
    // if (removeHybrid)
    // {
    //     int i = vector.size();
    //     //at least half of the colors first
    //     unsigned bound = 1 << (colorBits - 1);
    //     bound *= COLOR_COMPONENTS;
    //     while(i < bound)
    //     {
    //         //split
    //         for (int j = 0; j < i; j++)
    //         {
    //             vector.push_back(vector[j] + uint_dist(rng));
    //         }
    //
    //         vector = lbgRound(vector, img, epsilon, true, false, nullptr);
    //         i = vector.size();
    //     }
    //     bound <<= 1;
    //
    //     int bnd = vector.size();
    //
    //     for (int j = 0; j < bnd && vector.size() < bound; j++)
    //     {
    //         vector.push_back(vector[j] + uint_dist(rng));
    //     }
    //     vector = lbgRound(vector, img, epsilon, false, true, output);
    //
    //     return vector;
    // }

    //random turbulanceVector
    int i = vector.size();
    unsigned bound = 1 << (colorBits - 1);
    unsigned j = 0;

    while(i < bound && j < 1000)
    {
        //split
        for (int j = 0; j < i; j++)
        {
            vector.push_back(vector[j] + uint_dist2(rng));
        }
    
        vector = lbgRound(vector, img, imgLength, epsilon, true);
    }
    bound <<= 1;
    while (vector.size() < bound)
    {
        int bnd = vector.size();

        for (int j = 0; j < bnd && vector.size() < bound; j++)
        {
            vector.push_back(vector[j] + uint_dist2(rng));
        }
        vector = lbgRound(vector, img, imgLength, epsilon, true);
    }
    
    return vector;
}

//działamy na unsigned więc z błędami nie będzie problemów, przynajmniej jeśli rozmiar pary >= 2;
inline unsigned getAreaCenter(std::pair<unsigned, unsigned> input)
{
    return ((input.second + input.first) >> 1);
}

//
inline unsigned getAreaSize(std::pair<unsigned, unsigned> input)
{
    return input.second - input.first;
}

unsigned distortion(std::vector<uint8_t> sortedInputValues, uint8_t a, uint8_t b)
{
    unsigned result = 0;
    uint8_t c = getAreaCenter(std::make_pair(a, b));

    //binary search
    auto pointer = std::find(sortedInputValues.begin(), sortedInputValues.end(), a);

    while (*pointer <= c)
    {
        // result += mseSingle(c, *(pointer++));

        result += (c - *(pointer++));
        
        // auto tmp = (c - *(pointer++));
        // result += tmp * tmp;
    }
    while (pointer < sortedInputValues.end() && *pointer <= b)
    {
        // result += mseSingle(*(pointer++), c);
        // auto tmp = (*(pointer++) - c);
        // result += tmp * tmp;

        result += (*(pointer++) - c);
    }
    return result;
}



std::vector<uint8_t> logQuantizer(std::vector<uint8_t> inputValues, const int colorBits)
{
    std::vector<uint8_t> outputAlphabet;

    std::sort(inputValues.begin(), inputValues.end());
    std::set<uint8_t> sortedInputValues;
    for (auto value: inputValues)
    {
        sortedInputValues.insert(value);
    }

    //(indexLeftBound, indexRightBound)
    std::vector<std::pair<unsigned, unsigned>> areaBounds;
    areaBounds.push_back(std::make_pair(0, sortedInputValues.size() - 1));

    int desiredColorCount = (1 << colorBits);

    while (areaBounds.size() < desiredColorCount && areaBounds.size() < sortedInputValues.size())
    {
        //TODO accelarate
        //find biggest area
        int maxSize = 0;
        int index = -1;
        unsigned tmp;

        for (int i = 0; i < areaBounds.size(); i++)
        {
            tmp = getAreaSize(areaBounds[i]);
            if (tmp > 0)
            {
                auto a = sortedInputValues.begin(), b = a;
                std::advance(a, areaBounds[i].first);
                std::advance(b, areaBounds[i].second);
                //waga
                tmp = distortion(inputValues, *a, *b);
                if (tmp > maxSize)
                {
                    maxSize = tmp;
                    index = i;
                }
            }
        }

        //if the biggest area has at least 2 elements split it to two parts
        //(2 elements - distance = 1)
        if (maxSize > 0)
        {
            tmp = getAreaSize(areaBounds[index]);

            unsigned minDistortion = UINT32_MAX;
            unsigned bestCandidate = areaBounds[index].first;

            auto a = sortedInputValues.begin(), b = a, c = a;
            std::advance(a, areaBounds[index].first);
            std::advance(c, areaBounds[index].first);
            std::advance(b, areaBounds[index].second);

            //find the best partition
            for (int i = areaBounds[index].first; i < areaBounds[index].second - 1; i++)
            {
                unsigned currentDistortion = distortion(inputValues, *a, *c);
                std::advance(c, 1);
                currentDistortion += distortion(inputValues, *c, *b);
                if (currentDistortion < minDistortion)
                {
                    minDistortion = currentDistortion;
                    bestCandidate = i;
                }
            }

            areaBounds.push_back(std::make_pair(bestCandidate + 1, areaBounds[index].second));
            areaBounds[index].second = bestCandidate;
        }
    }

    //find each area find center and return them
    for (auto bounds: areaBounds)
    {
        auto a = sortedInputValues.begin(), b = a;
        std::advance(a, bounds.first);
        std::advance(b, bounds.second);
        outputAlphabet.push_back(getAreaCenter(std::make_pair(*a, *b)));
    }

    std::cout << "OutputAlphabet size: " << outputAlphabet.size() << std::endl;

    return outputAlphabet;
}


int16_t findBestInQuantizator(const int16_t value, const std::vector<int16_t> quantizator)
{
    int minDistIndex = -1;
    uint16_t minDist = UINT16_MAX;

    for (int j = 0; j < quantizator.size(); j++)
    {
        uint16_t dist;

        if (value >= quantizator[j])
            dist = value - quantizator[j];
        else 
            dist = quantizator[j] - value;

        if (dist < minDist)
        {
            minDist = dist;
            minDistIndex = j;
        }
    }

    if (minDistIndex > 0)
    {
        return quantizator[minDistIndex];
    }

    //never claim
    return quantizator[0];
}

uint8_t findBestInQuantizator(const uint8_t value, const std::vector<uint8_t> quantizator)
{
    int minDistIndex = -1;
    uint8_t minDist = UINT8_MAX;

    for (int j = 0; j < quantizator.size(); j++)
    {
        uint8_t dist;

        if (value >= quantizator[j])
            dist = value - quantizator[j];
        else 
            dist = quantizator[j] - value;

        if (dist < minDist)
        {
            minDist = dist;
            minDistIndex = j;
        }
    }

    if (minDistIndex > 0)
    {
        return quantizator[minDistIndex];
    }

    //never claim
    return quantizator[0];
}



//kiedy przychodzi nowa liczba podziel jej obszar na dwa  (jesli wiekszy niz 1)
//zlacz dwa obok siebie najrzadziej uzywane
uint8_t findBestInDynamicQuantizator(const uint8_t value, std::vector<std::pair<uint8_t, uint8_t>>* quantizator, 
    std::vector<unsigned>* frequency, unsigned* charFrequency)
{
    charFrequency[value]++;
    // std::cout <<  " " << (*frequency).size() << std::endl;
    //znajdz normalnie wartosc
    uint8_t result;
    int toSplit = -1;

    for (unsigned i = 0; i < (*quantizator).size(); i++)
    {
        auto bounds = (*quantizator)[i];
        if (bounds.first <= value && bounds.second >= value)
        {
            if (bounds.second - bounds.first == 0)
            {
                //nie da się podzielić obszaru - ok
                ((*frequency)[i])++;
                return getAreaCenter(bounds);
            }
            result = getAreaCenter(bounds);
            toSplit = i;
            break;
        }
    }


    //popraw kwantyzator
    unsigned minFrequency = UINT32_MAX;
    unsigned minIndex;

    //zacznij szukać od środka, żeby tak rozwiązywać remisy
    // (frequency.size() zawsze parzyste)
    for (int i = (*frequency).size() / 2; i >= 0; i--)
    {
        unsigned tmp = (*frequency)[i] + (*frequency)[i + 1];
        if (tmp < minFrequency)
        {
            minFrequency = tmp;
            minIndex = i;
            // std::cout << minIndex << " b " << (*frequency).size() << std::endl;
        }
        tmp = (*frequency)[(*frequency).size() - 1 - i] + (*frequency)[(*frequency).size() - 2 - i];
        if (tmp < minFrequency)
        {
            minFrequency = tmp;
            minIndex = (*frequency).size() - 2 - i;
    // std::cout << minIndex << " a " << (*frequency).size() << std::endl;
        }
    }

    if (minIndex == toSplit)
    {
        ((*frequency)[toSplit])++;
        return result;
    }

    // std::cout << minIndex << " " << (*frequency).size() << std::endl;

    //łączenie dwóch przedziałów
    (*quantizator)[minIndex] = std::make_pair((*quantizator)[minIndex].first, (*quantizator)[minIndex + 1].second);
    (*frequency)[minIndex] += (*frequency)[minIndex + 1];
    (*quantizator).erase((*quantizator).begin() + minIndex + 1);
    (*frequency).erase((*frequency).begin() + minIndex + 1);
    
    //index sie obnizyl po usunieciu
    if (minIndex < toSplit)
    {
        toSplit--;
    }

    //dzielenie przedziału
    int newBound = getAreaCenter((*quantizator)[toSplit]);
    auto a = std::make_pair((*quantizator)[toSplit].first, newBound);  
    auto b = std::make_pair(newBound + 1, (*quantizator)[toSplit].second);

    (*quantizator)[toSplit] = a;
    (*frequency)[toSplit] = 0;
    //CZY NA PEWNO +1?
    (*quantizator).emplace((*quantizator).begin() + toSplit + 1, b);
    (*frequency).emplace((*frequency).begin() + toSplit + 1, 0);
    

    for (int i = a.first; i <= a.second; i++)
    {
        (*frequency)[toSplit] += charFrequency[i];
    }
    for (int i = b.first; i <= b.second; i++)
    {
        (*frequency)[toSplit + 1] += charFrequency[i];
    }

    return result;
}

std::vector<uint8_t> rownomierny(const int colorBits)
{
    int step = 256 >> colorBits;
    std::vector<uint8_t> result;

    for (int i = 0; i < 256; i += step)
    {
        result.push_back(i);
    }

    // printCodebook(result);

    return result;
}

std::vector<std::pair<uint8_t, uint8_t>> rownomiernyPair(const int colorBits)
{
    int step = 256 >> colorBits;
    int areaSize = step - 1;
    std::vector<std::pair<uint8_t, uint8_t>> result;

    for (int i = 0; i < 256; i += step)
    {
        result.push_back(std::make_pair(i, i + areaSize));
    }

    // printCodebook(result);

    return result;
}


//recommended areaSize 2, 4 or 8
std::vector<uint8_t> basicQuantizer(const int colorBits, int areaSize)
{
    std::vector<uint8_t> result;
    unsigned desiredColors = (1 << colorBits);
    result.push_back(0);
    result.push_back(128);
    
    if (colorBits == 1)
    {   
        return result;
    } 

    while (result.size() < desiredColors)
    {
        //weź lewy przedział i zmierz jego długość
        int i = 0; 
        int tmp;
        do 
        {
            tmp = result[i + 1] - result[i];
            i++;
        }
        while (tmp < areaSize && result[i - 1] < 128);// - (12 * areaSize - 12));
        //nie znaleziono juz wystarczająco dużego przedziału
        if (result[i - 1] >= 128)// - (12 * areaSize - 12))
        {
            areaSize >>= 1;
            continue;
        }
        //w obecnym przedziale chcemy wstawić punkt w 1/areaSize odległości lewego brzegu od prawego
        result.emplace(result.begin() + i, result[i - 1] + tmp / 3);
        std::cout << "Pushed " << result[i - 1] + tmp / areaSize << " res " << (int)result[i - 1] << " tmp " << tmp << std::endl;
        //i symetryczny z prawej (dla ujemnych różnic)
        result.emplace(result.end() - i + 1, 256 - (result[i - 1] + tmp / 3));
    }

    printCodebook(result);

    std::cout << "areaSizeatTHe END " << areaSize;

    return result;
}

// void encodeImage(const char* outputFileName, const char* inputFileName, uint8_t quantizationBits)
// {
//     image* img = new image();
//     if (!parseImage(inputFileName, img))
//     {
//         std::cout << "Error parsing inputImage" << std::endl;
//         return; 
//     }

//     int pixelCount = img->height * img->width;
    
//     std::vector<uint8_t> y[COLOR_COMPONENTS];
//     std::vector<uint8_t> z[COLOR_COMPONENTS];

//     // calculate {Y} and {Z}
//     //znajdz kwantyzator dla y
//     std::vector<uint16_t> yQuantizator[COLOR_COMPONENTS];
//     std::vector<uint8_t> zQuantizator[COLOR_COMPONENTS];

//     for (int j = 0; j < COLOR_COMPONENTS; j++)
//     {
//         std::cout << "Calculating filter values for j = " << j << std::endl;

//         int i = 1;
//         for(; i < pixelCount; i += 2)
//         {
//             y[j].push_back(AVG(img->bmp[i * COLOR_COMPONENTS + j], img->bmp[(i - 1) * COLOR_COMPONENTS + j]));
//             z[j].push_back(STDDEV(img->bmp[i * COLOR_COMPONENTS + j], img->bmp[(i - 1) * COLOR_COMPONENTS + j]));
//         }
//         //nieparzysta liczba pikseli, a kodujemy pary
//         if (i == pixelCount)
//         {
//             //dodaj ostatni 
//             y[j].push_back(img->bmp[(i - 1) * COLOR_COMPONENTS + j]);
//             z[j].push_back(img->bmp[(i - 1) * COLOR_COMPONENTS + j]);
//         }
//     }

//     //zakoduj y roznicowo
//     //kwantyzacja w trakcie
//     for (int j = 0; j < COLOR_COMPONENTS; j++)
//     {
//         std::vector<int16_t> yDiff;
//         yDiff.push_back(y[j][0]);
//         for (int i = 1; i < y[j].size(); i++)
//         {
//             yDiff.push_back(y[j][i] - y[j][i - 1]);
//         }

//         yQuantizator[j] = lbg(yDiff, yDiff.size(), 0.001, quantizationBits);
//         zQuantizator[j] = lbg(z[j], z[j].size(), 0.001, quantizationBits);

//         printCodebook(yQuantizator[j]);

//         // y[j][0] = findBestInQuantizator(y[j][0], yQuantizator[j]);
//         // uint8_t currentY = y[j][0];
//         // for (int i = 0; i < y[j].size(); i++)
//         // {
//         //     y[j][i] = findBestInQuantizator( (y[j][i] - currentY) , yQuantizator[j]);

//         //     z[j][i] = findBestInQuantizator(z[j][i], zQuantizator[j]);

//         // }
//     }



//     deleteImage(img);
// }

void encodeImage(const char* outputFileName, const char* inputFileName, uint8_t quantizationBits)
{
    image* img = new image();
    if (!parseImage(inputFileName, img))
    {
        std::cout << "Error parsing inputImage" << std::endl;
        return; 
    }

    int pixelCount = img->height * img->width;
    
    std::vector<uint8_t> y[COLOR_COMPONENTS];
    std::vector<uint8_t> z[COLOR_COMPONENTS];

    // calculate {Y} and {Z}
    //znajdz kwantyzator dla y
    std::vector<uint8_t> yQuantizator[COLOR_COMPONENTS];
    std::vector<uint8_t> zQuantizator[COLOR_COMPONENTS];

    uint8_t lastPixel[COLOR_COMPONENTS] = {};

    for (int j = 0; j < COLOR_COMPONENTS; j++)
    {
        // std::cout << "Calculating filter values for j = " << j << std::endl;

        int i = 1;
        for(; i < pixelCount - 1; i += 2)
        {
            y[j].push_back(AVG(img->bmp[i * COLOR_COMPONENTS + j], img->bmp[(i - 1) * COLOR_COMPONENTS + j]));
            z[j].push_back(STDDEV(img->bmp[i * COLOR_COMPONENTS + j], img->bmp[(i - 1) * COLOR_COMPONENTS + j]));
        }

        //zapisz ostatni pixel jesli potrzeba - kodujemy pary
        if (i == pixelCount - 1)
        {
            lastPixel[j] = img->bmp[i * COLOR_COMPONENTS + j];
        }
    }

    //zakoduj y roznicowo
    //kwantyzacja w trakcie
    for (int j = 0; j < COLOR_COMPONENTS; j++)
    {
        std::vector<uint8_t> yDiff;
        std::vector<uint8_t> newY(y[j].size(), 0);
        yDiff.push_back(y[j][0]);
        for (int i = 1; i < y[j].size(); i++)
        {
            yDiff.push_back(y[j][i] - y[j][i - 1]);
        }

        // yQuantizator[j] = lbg(yDiff, yDiff.size(), 0.001, quantizationBits);
        // zQuantizator[j] = lbg(z[j], z[j].size(), 0.001, quantizationBits);


        // yQuantizator[j] = logQuantizer(yDiff, quantizationBits);
        yQuantizator[j] = rownomierny(quantizationBits);
        // std::vector<std::seed_seq::result_type> yFrequency(yQuantizator[j].size(), 0); 
        // unsigned yCharFrequency[256] = {0}; 
        // yQuantizator[j] = basicQuantizer(quantizationBits, 16);
        zQuantizator[j] = rownomierny(quantizationBits);
        // zQuantizator[j] = rownomierny(quantizationBits);



        // printCodebook(yQuantizator[j]);


        // newY[0] = findBestInDynamicQuantizator(y[j][0], &yQuantizator[j], &yFrequency, yCharFrequency);
        // uint8_t currentY = newY[0];
        // z[j][0] = findBestInQuantizator(z[j][0], zQuantizator[j]);

        // for (int i = 1; i < y[j].size(); i++)
        // {
        //     // std::cout << "y: " << (int)y[j][i] << " currentY: " << (int)currentY;  
        //     auto tmp = findBestInDynamicQuantizator((y[j][i] - currentY), &yQuantizator[j], &yFrequency, yCharFrequency);
        //     // y[j][i] = 

        //     //wykrywanie przekręcenia licznika przez bledy kwantyzacji
        //     for (int k = 1; currentY + tmp < currentY && y[j][i] > y[j][i-1]; k++)
        //     {
        //         tmp = findBestInDynamicQuantizator((y[j][i] - currentY + k), &yQuantizator[j], &yFrequency, yCharFrequency);
        //     }
        //     newY[i] = tmp;
        //     // std::cout << " yDiffQuantized: " << (int)y[j][i] << std::endl;  
        //     currentY += tmp;
        //     z[j][i] = findBestInQuantizator(z[j][i], zQuantizator[j]);
        // }
        // y[j] = newY;


        // yQuantizator[j] = logQuantizer(yDiff, quantizationBits);
        //     // yQuantizator[j] = rownomierny(quantizationBits);
        //     // yQuantizator[j] = basicQuantizer(quantizationBits, 16);
        //     zQuantizator[j] = logQuantizer(z[j], quantizationBits);
            // zQuantizator[j] = rownomierny(quantizationBits);

            // printCodebook(yQuantizator[j]);

        newY[0] = findBestInQuantizator(y[j][0], yQuantizator[j]);
        uint8_t currentY = newY[0];
        z[j][0] = findBestInQuantizator(z[j][0], zQuantizator[j]);

        for (int i = 1; i < y[j].size(); i++)
        {
            // std::cout << "y: " << (int)y[j][i] << " currentY: " << (int)currentY;  
            auto tmp = findBestInQuantizator((y[j][i] - currentY), yQuantizator[j]);
            // y[j][i] = 

            //wykrywanie przekręcenia licznika przez bledy kwantyzacji
            for (int k = 1; currentY + tmp < currentY && y[j][i] > y[j][i-1]; k++)
            {
                tmp = findBestInQuantizator((y[j][i] - currentY + k), yQuantizator[j]);
            }
            newY[i] = tmp;
            // std::cout << " yDiffQuantized: " << (int)y[j][i] << std::endl;  
            currentY += tmp;
            z[j][i] = findBestInQuantizator(z[j][i], zQuantizator[j]);
        }
        y[j] = newY;
    }

    //utwórz plik wynikowy
    for (int j = 0; j < COLOR_COMPONENTS; j++)
    {
        for (int i = 0; i < y[j].size(); i++)
        {
            img->bmp[(i * 2) * COLOR_COMPONENTS + j] = y[j][i];
            img->bmp[(i * 2 + 1) * COLOR_COMPONENTS + j] = z[j][i];
        }
    }

    //nieparzysta liczba pikseli, a kodujemy pary
    if ((img->width % 2) && (img->height % 2))
    {
        //dodaj ostatni pixel
        //tu jeszcze nikt nic nowego nie zapisał
        for (int j = 0; j < COLOR_COMPONENTS; j++)
        {    
            img->bmp[y[j].size() * 2 + j] = lastPixel[j];
        }
    }

    writeImageToFile(outputFileName, img, inputFileName);

    deleteImage(img);
}