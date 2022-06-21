#pragma once

#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <cstdio>

#define COLOR_COMPONENTS 3

typedef struct image {
    std::string fileName;
    unsigned height;
    unsigned width;
    uint8_t* bmp;
} image;

void initImage(const unsigned height, const unsigned width, const std::string name, image* img)
{
    img->fileName = name;
    img->height = height;
    img->width = width;
    img->bmp = new uint8_t[height * width * COLOR_COMPONENTS];
}

void deleteImage(image* img)
{
    if (img)
    {
        if (img->bmp)
        {
            delete[](img->bmp);
        }
        delete(img);
    }
}

void printBitmap(image* img)
{
    if (img && img->bmp)
    {
        for (int i = 0; i < img->height; i++)
        {
            for (int j = 0; j < img->width; j++)
            {
                printf("%x%x%x", img->bmp[(i * img->height + j) * COLOR_COMPONENTS], img->bmp[(i * img->height + j) * COLOR_COMPONENTS + 1], img->bmp[(i * img->height + j) * COLOR_COMPONENTS + 2]);
                // std::cout <<  << " "
                //             << img->bmp[(i * img->height + j) * COLOR_COMPONENTS] << " "
                //             << img->bmp[(i * img->height + j) * COLOR_COMPONENTS] << " ";     
            } 
            std::cout << std::endl;
        }
    }
    else
    {
        std::cout << "Image has not been initialized " << img << std::endl;
    }
}

bool parseImage(const char* fileName, image* img)
{
    FILE* imgFile = fopen(fileName, "rb");

    if (imgFile)
    {
        int tmp = fgetc(imgFile);
        if (tmp != EOF)
        {
            uint8_t IDLength = tmp;

            if ((tmp = fgetc(imgFile)) != EOF)
            {
                uint8_t colorMapType = tmp;
                
                if ((tmp = fgetc(imgFile)) != EOF)
                {
                    uint8_t imageType = tmp;
                    
                    if (!colorMapType)
                    {
                        //read 5 zero bytes - color map specification
                        for (int i = 0; i < 5; i++)
                        {
                            if ((tmp = fgetc(imgFile)) == EOF)
                            {
                                printf("OOPS file ended abruptly");
                                fclose(imgFile);
                                return false;
                            }
                            if (tmp != 0)
                            {
                                printf("OOPS file parsing error when dealing with color map specification");
                                fclose(imgFile);
                                return false;
                            }
                        }

                        //read 10 bytes - image specification
                        uint8_t buffer[2];
                        if ((fread(buffer, sizeof(*buffer), 2, imgFile)) == 2)
                        {
                            uint16_t imgXOrigin = buffer[0] | (buffer[1] << 8);

                            if ((fread(buffer, sizeof(*buffer), 2, imgFile)) == 2)
                            {
                                uint16_t imgYOrigin = buffer[0] | (buffer[1] << 8);

                                if ((fread(buffer, sizeof(*buffer), 2, imgFile)) == 2)
                                {
                                    uint16_t imgWidth = buffer[0] | (buffer[1] << 8);

                                    if ((fread(buffer, sizeof(*buffer), 2, imgFile)) == 2)
                                    {
                                        uint16_t imgHeight = buffer[0] | (buffer[1] << 8);

                                        if ((tmp = fgetc(imgFile)) != EOF)
                                        {
                                            uint8_t pixelDepth = tmp;

                                            if ((tmp = fgetc(imgFile)) != EOF)
                                            {
                                                uint8_t imageDescriptor = tmp;

                                                //header successfully parsed
                                                initImage(imgHeight, imgWidth, fileName, img);

                                                //ignore Image ID field - should not even exist in our examples
                                                for (int i = 0; i < IDLength; i++)
                                                {
                                                    if ((tmp = fgetc(imgFile)) == EOF)
                                                    {
                                                        printf("OOPS file ended abruptly");
                                                        fclose(imgFile);
                                                        deleteImage(img);
                                                        return false;
                                                    }
                                                }

                                                //parse bitmap
                                                for (int i = 0; i < imgWidth; i++)
                                                {
                                                    for (int j = 0; j < imgHeight; j++)
                                                    {
                                                        if (fread(&img->bmp[(i * imgHeight + j) * COLOR_COMPONENTS], sizeof(*img->bmp), COLOR_COMPONENTS, imgFile) != COLOR_COMPONENTS)
                                                        {
                                                            printf("Error occured while parsing bitmap %d, %d, %d, %d", i, j, imgWidth, imgHeight);
                                                            fclose(imgFile);
                                                            deleteImage(img);
                                                            return false;
                                                        }
                                                    } 
                                                }

                                                //bitmap parsed succesfully

                                                //ignore the footer - at least for now
                                                fclose(imgFile);
                                                return true;  
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        printf("OOPS color map is included");
                        deleteImage(img);
                        fclose(imgFile);
                        return false;
                    }
                }
            }
        }

        printf("Error occured while parsing image");
        deleteImage(img);
        fclose(imgFile);
        return false;
    }
    else
    {
        std::cout << "Error loading image: " << fileName << std::endl;
        return false;
    }
}
