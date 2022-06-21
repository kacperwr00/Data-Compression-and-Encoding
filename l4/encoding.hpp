#pragma once

#include "image.hpp"
#include <cmath>
#include <tuple>
#include <vector>
#include <cstring>

#define MAX(A, B) (A > B ? A : B)
#define MIN(A, B) (A < B ? A : B)

#define ALPHABET_SIZE 256

typedef std::tuple<uint8_t, uint8_t, uint8_t> (*predictor)(const image*img, uint8_t i, uint8_t j);

//step allows to calculate entropy for a specific color if == 3, or traditional entropy for step == 1
void countCharacterOccurances(int* characterCount, const uint8_t* text, const int textLength, uint8_t step)
{
    uint8_t curChar;

    for (int i = 0; i < textLength; i += step)
    {
        characterCount[text[i]]++;
    }
}

double calculateEntropy(int characterCount[ALPHABET_SIZE], int totalCharacterCount)
{
    double entropy = 0.0;
    double logTotalCharCount = log2(totalCharacterCount);

    for (int i = 0; i < ALPHABET_SIZE; i++)
    {
        int tmp = characterCount[i];
        if (tmp != 0)
        {
            entropy += tmp * (logTotalCharCount - log2(tmp));
        }
    }
    
    return entropy / totalCharacterCount;
}

uint8_t maximum(std::tuple<uint8_t, uint8_t, uint8_t> tuple)
{
    uint8_t result = get<0>(tuple);
    if (get<1>(tuple) > result)
    {
        result = get<1>(tuple);
    }
    if (get<2>(tuple) > result)
    {
        return get<2>(tuple);
    }
    return result;
}

std::tuple<uint8_t, uint8_t, uint8_t> predictorOne(const image* img, uint8_t i, uint8_t j)
{
    if (!i--)
    {
        return std::make_tuple(0, 0, 0);
    }

    return std::make_tuple(img->bmp[(i * img->height + j) * COLOR_COMPONENTS], img->bmp[(i * img->height + j) * COLOR_COMPONENTS + 1], img->bmp[(i * img->height + j) * COLOR_COMPONENTS + 2]);
} 

std::tuple<uint8_t, uint8_t, uint8_t> predictorTwo(const image* img, uint8_t i, uint8_t j)
{
    if (!j--)
    {
        return std::make_tuple(0, 0, 0);
    }

    return std::make_tuple(img->bmp[(i * img->height + j) * COLOR_COMPONENTS], img->bmp[(i * img->height + j) * COLOR_COMPONENTS + 1], img->bmp[(i * img->height + j) * COLOR_COMPONENTS + 2]);
} 

std::tuple<uint8_t, uint8_t, uint8_t> predictorThree(const image* img, uint8_t i, uint8_t j)
{
    if (!i-- || !j--)
    {
        return std::make_tuple(0, 0, 0);
    }

    return std::make_tuple(img->bmp[(i * img->height + j) * COLOR_COMPONENTS], img->bmp[(i * img->height + j) * COLOR_COMPONENTS + 1], img->bmp[(i * img->height + j) * COLOR_COMPONENTS + 2]);
} 

std::tuple<uint8_t, uint8_t, uint8_t> predictorFour(const image* img, uint8_t i, uint8_t j)
{
    std::tuple<uint8_t, uint8_t, uint8_t> N, W, NW;
    if (!i)
    {
        W = std::make_tuple(0, 0, 0);
    }
    else 
    {
        W = std::make_tuple(img->bmp[((i - 1) * img->height + j) * COLOR_COMPONENTS], img->bmp[((i - 1) * img->height + j) * COLOR_COMPONENTS + 1], img->bmp[((i - 1) * img->height + j) * COLOR_COMPONENTS + 2]);
    }
    
    if (!j)
    {
        N = std::make_tuple(0, 0, 0);
    }
    else 
    {
        N = std::make_tuple(img->bmp[(i * img->height + (j - 1)) * COLOR_COMPONENTS], img->bmp[(i * img->height + (j - 1)) * COLOR_COMPONENTS + 1], img->bmp[(i * img->height + (j - 1)) * COLOR_COMPONENTS + 2]);
    }

    if (!j-- || !i--)
    {
        NW = std::make_tuple(0, 0, 0);
    }
    else
    {
        NW = std::make_tuple(img->bmp[(i * img->height + j) * COLOR_COMPONENTS], img->bmp[(i * img->height + j) * COLOR_COMPONENTS + 1], img->bmp[(i * img->height + j) * COLOR_COMPONENTS + 2]);
    }
   
    return std::make_tuple(get<0>(N) + get<0>(W) - get<0>(NW), get<1>(N) + get<1>(W) - get<1>(NW), get<2>(N) + get<2>(W) - get<2>(NW));
} 

std::tuple<uint8_t, uint8_t, uint8_t> predictorFive(const image* img, uint8_t i, uint8_t j)
{
    std::tuple<uint8_t, uint8_t, uint8_t> N, W, NW;
    if (!i)
    {
        W = std::make_tuple(0, 0, 0);
    }
    else 
    {
        W = std::make_tuple(img->bmp[((i - 1) * img->height + j) * COLOR_COMPONENTS], img->bmp[((i - 1) * img->height + j) * COLOR_COMPONENTS + 1], img->bmp[((i - 1) * img->height + j) * COLOR_COMPONENTS + 2]);
    }
    
    if (!j)
    {
        N = std::make_tuple(0, 0, 0);
    }
    else 
    {
        N = std::make_tuple(img->bmp[(i * img->height + (j - 1)) * COLOR_COMPONENTS], img->bmp[(i * img->height + (j - 1)) * COLOR_COMPONENTS + 1], img->bmp[(i * img->height + (j - 1)) * COLOR_COMPONENTS + 2]);
    }

    if (!j-- || !i--)
    {
        NW = std::make_tuple(0, 0, 0);
    }
    else
    {
        NW = std::make_tuple(img->bmp[(i * img->height + j) * COLOR_COMPONENTS], img->bmp[(i * img->height + j) * COLOR_COMPONENTS + 1], img->bmp[(i * img->height + j) * COLOR_COMPONENTS + 2]);
    }
   
    return std::make_tuple(get<0>(N) + (get<0>(W) - get<0>(NW)) / 2, get<1>(N) + (get<1>(W) - get<1>(NW)) / 2, get<2>(N) + (get<2>(W) - get<2>(NW)) / 2);
} 

std::tuple<uint8_t, uint8_t, uint8_t> predictorSix(const image* img, uint8_t i, uint8_t j)
{
    std::tuple<uint8_t, uint8_t, uint8_t> N, W, NW;
    if (!i)
    {
        W = std::make_tuple(0, 0, 0);
    }
    else 
    {
        W = std::make_tuple(img->bmp[((i - 1) * img->height + j) * COLOR_COMPONENTS], img->bmp[((i - 1) * img->height + j) * COLOR_COMPONENTS + 1], img->bmp[((i - 1) * img->height + j) * COLOR_COMPONENTS + 2]);
    }
    
    if (!j)
    {
        N = std::make_tuple(0, 0, 0);
    }
    else 
    {
        N = std::make_tuple(img->bmp[(i * img->height + (j - 1)) * COLOR_COMPONENTS], img->bmp[(i * img->height + (j - 1)) * COLOR_COMPONENTS + 1], img->bmp[(i * img->height + (j - 1)) * COLOR_COMPONENTS + 2]);
    }

    if (!j-- || !i--)
    {
        NW = std::make_tuple(0, 0, 0);
    }
    else
    {
        NW = std::make_tuple(img->bmp[(i * img->height + j) * COLOR_COMPONENTS], img->bmp[(i * img->height + j) * COLOR_COMPONENTS + 1], img->bmp[(i * img->height + j) * COLOR_COMPONENTS + 2]);
    }
   
    return std::make_tuple(get<0>(W) + (get<0>(N) - get<0>(NW)) / 2, get<1>(W) + (get<1>(N) - get<1>(NW)) / 2, get<2>(W) + (get<2>(N) - get<2>(NW)) / 2);
} 

std::tuple<uint8_t, uint8_t, uint8_t> predictorSeven(const image* img, uint8_t i, uint8_t j)
{
    std::tuple<uint8_t, uint8_t, uint8_t> N, W;
    if (!i)
    {
        W = std::make_tuple(0, 0, 0);
    }
    else 
    {
        W = std::make_tuple(img->bmp[((i - 1) * img->height + j) * COLOR_COMPONENTS], img->bmp[((i - 1) * img->height + j) * COLOR_COMPONENTS + 1], img->bmp[((i - 1) * img->height + j) * COLOR_COMPONENTS + 2]);
    }
    
    if (!j)
    {
        N = std::make_tuple(0, 0, 0);
    }
    else 
    {
        N = std::make_tuple(img->bmp[(i * img->height + (j - 1)) * COLOR_COMPONENTS], img->bmp[(i * img->height + (j - 1)) * COLOR_COMPONENTS + 1], img->bmp[(i * img->height + (j - 1)) * COLOR_COMPONENTS + 2]);
    }
   
    return std::make_tuple((get<0>(W) + get<0>(N)) / 2, (get<1>(W) + get<1>(N)) / 2, (get<2>(W) + get<2>(N)) / 2);
} 

std::tuple<uint8_t, uint8_t, uint8_t> predictorEight(const image* img, uint8_t i, uint8_t j)
{
    std::tuple<uint8_t, uint8_t, uint8_t> N, W, NW;
    if (!i)
    {
        W = std::make_tuple(0, 0, 0);
    }
    else 
    {
        W = std::make_tuple(img->bmp[((i - 1) * img->height + j) * COLOR_COMPONENTS], img->bmp[((i - 1) * img->height + j) * COLOR_COMPONENTS + 1], img->bmp[((i - 1) * img->height + j) * COLOR_COMPONENTS + 2]);
    }
    
    if (!j)
    {
        N = std::make_tuple(0, 0, 0);
    }
    else 
    {
        N = std::make_tuple(img->bmp[(i * img->height + (j - 1)) * COLOR_COMPONENTS], img->bmp[(i * img->height + (j - 1)) * COLOR_COMPONENTS + 1], img->bmp[(i * img->height + (j - 1)) * COLOR_COMPONENTS + 2]);
    }

    if (!j-- || !i--)
    {
        NW = std::make_tuple(0, 0, 0);
    }
    else
    {
        NW = std::make_tuple(img->bmp[(i * img->height + j) * COLOR_COMPONENTS], img->bmp[(i * img->height + j) * COLOR_COMPONENTS + 1], img->bmp[(i * img->height + j) * COLOR_COMPONENTS + 2]);
    }

    uint8_t a, b, c;
    auto aMax = MAX(get<0>(N), get<0>(W));
    auto aMin = MIN(get<0>(N), get<0>(W));
    
    auto bMax = MAX(get<1>(N), get<1>(W));
    auto bMin = MIN(get<1>(N), get<1>(W));
    
    auto cMax = MAX(get<2>(N), get<2>(W));
    auto cMin = MIN(get<2>(N), get<2>(W));

    unsigned tmp = get<0>(NW);
    if (tmp >= aMax)
    {
        a = aMin;
    }
    else if (tmp <= aMin)
    {
        a = aMax;
    }
    else
    {
        a = aMax + aMin - tmp;
    }

    tmp = get<1>(NW);
    if (tmp >= bMax)
    {
        b = bMin;
    }
    else if (tmp <= bMin)
    {
        b = bMax;
    }
    else
    {
        b = bMax + bMin - tmp;
    }

    tmp = get<2>(NW);
    if (tmp >= cMax)
    {
        c = cMin;
    }
    else if (tmp <= cMin)
    {
        c = cMax;
    }
    else
    {
        c = cMax + cMin - tmp;
    }

    return std::make_tuple(a, b, c);
} 

std::vector<predictor> getAvailablePredictors()
{
    return std::vector<predictor>{predictorOne, predictorTwo, predictorThree, predictorFour,
                                    predictorFive, predictorSix, predictorSeven, predictorEight};
}

void findBestEncoding(const image* img)
{
    if (img)
    {
        int* charOccurances = new int[ALPHABET_SIZE]();
        countCharacterOccurances(charOccurances, img->bmp, img->width * img->height * COLOR_COMPONENTS, 1);
        double inputEntropy = calculateEntropy(charOccurances, img->width * img->height * COLOR_COMPONENTS);
        std::fill_n(charOccurances, ALPHABET_SIZE, 0);

        //calculate entropy for all the components of the colors
        std::vector<double> componentEntropies;
        for (int i = 0; i < COLOR_COMPONENTS; i++)
        {
            countCharacterOccurances(charOccurances, img->bmp + i, img->width * img->height * COLOR_COMPONENTS, COLOR_COMPONENTS);
            componentEntropies.emplace_back(calculateEntropy(charOccurances, img->width * img->height));
            std::fill_n(charOccurances, ALPHABET_SIZE, 0);
        }

        std::cout << img->fileName << ": Entropia na wejściu: " << inputEntropy << std::endl << "\tEntropia poszczególnych składowych: ";
        for (auto componentEntropy: componentEntropies)
        {
            std::cout << componentEntropy << " ";
        }
        std::cout << std::endl;

        std::vector<predictor> predictors = getAvailablePredictors();

        int bestPredictor[COLOR_COMPONENTS + 1];
        std::fill_n(bestPredictor, COLOR_COMPONENTS + 1, -1);
        double bestEntropy[COLOR_COMPONENTS + 1];
        std::fill_n(bestEntropy, COLOR_COMPONENTS + 1, ALPHABET_SIZE);

        int predictorNumber = 0;
        for (auto predictor: predictors)
        {
            double currentPredictorEntropy = -1;
            double currentPredictorComponentEntropies[COLOR_COMPONENTS];
            std::fill_n(currentPredictorComponentEntropies, COLOR_COMPONENTS, -1);

            //calculate encoding for the predictor
            uint8_t encodedBitmap[img->width * img->height * COLOR_COMPONENTS];
            memcpy(encodedBitmap, img->bmp, img->width * img->height * COLOR_COMPONENTS * sizeof(*encodedBitmap));

            //TODO check if consistent bounds with parse
            //for each pixel calculate difference
            for (int i = 0; i < img->width; i++)
            {
                for (int j = 0; j < img->height; j++)
                {
                    auto difference = (*predictor)(img, i, j);

                    encodedBitmap[(i * img->height + j) * COLOR_COMPONENTS] -= get<0>(difference);
                    encodedBitmap[(i * img->height + j) * COLOR_COMPONENTS + 1] -= get<1>(difference);
                    encodedBitmap[(i * img->height + j) * COLOR_COMPONENTS + 2] -= get<2>(difference);
                }
            }

            //calculate entropy for the predictor
            countCharacterOccurances(charOccurances, encodedBitmap, img->width * img->height * COLOR_COMPONENTS, 1);
            currentPredictorEntropy = calculateEntropy(charOccurances, img->width * img->height * COLOR_COMPONENTS);
            std::fill_n(charOccurances, ALPHABET_SIZE, 0);

            //calculate entropy for all the components of the colors
            std::vector<double> componentEntropies;
            for (int i = 0; i < COLOR_COMPONENTS; i++)
            {
                countCharacterOccurances(charOccurances, encodedBitmap + i, 
                                                    img->width * img->height * COLOR_COMPONENTS - i, COLOR_COMPONENTS);
                currentPredictorComponentEntropies[i] =  
                        calculateEntropy(charOccurances, img->width * img->height);
                std::fill_n(charOccurances, ALPHABET_SIZE, 0);
            }

            if (currentPredictorEntropy < *bestEntropy)
            {
                *bestPredictor = predictorNumber;
                *bestEntropy = currentPredictorEntropy;
            }
            for (int i = 1; i < COLOR_COMPONENTS + 1; i++)
            {
                if (currentPredictorComponentEntropies[i - 1] < bestEntropy[i])
                {
                    bestPredictor[i] = predictorNumber;
                    bestEntropy[i] = currentPredictorComponentEntropies[i - 1];
                }
            }
            
            std::cout << "  Predictor number " << ++predictorNumber << ":\t" << currentPredictorEntropy << "  ";
            for (double entropy: currentPredictorComponentEntropies)
            {
                std::cout << entropy << " ";
            }
            std::cout << std::endl;
        }
        
        std::cout << "Optimal predictors for this case: ";
        for (int best: bestPredictor)
        {   
            std::cout << best + 1 << " ";
        }
        std::cout << std::endl << std::endl;
    }
}