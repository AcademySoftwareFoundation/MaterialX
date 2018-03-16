#include <MaterialXView/Handlers/ImageHandler.h>
#include <cmath>

namespace MaterialX
{

bool ImageHandler::createDefaultImage(unsigned int& width,
                                      unsigned int& height,
                                      unsigned char** buffer)
{
    // Create a ramp texture as the default
    //
    const unsigned int imageSize = 256;
    unsigned int middle = imageSize / 2;
    *buffer = new unsigned char[imageSize * imageSize * 4];
    unsigned char* pixel = *buffer;
    for (unsigned int i = 0; i<imageSize; i++)
    {
        for (unsigned int j = 0; j<imageSize; j++)
        {
            float fi = (float)i;
            float fj = (float)j;
            float dist = std::sqrt(std::pow((middle - fj), 2) + std::pow((middle - fi), 2));
            dist /= imageSize;
            float mdist = (1.0f - dist);

            *pixel++ = (unsigned char)(65.0f * dist + 255.0f * mdist);
            *pixel++ = (unsigned char)(205.0f * dist + 147.0f * mdist);
            *pixel++ = (unsigned char)(255.0f * dist + 75.0f * mdist);
            *pixel++ = 255;
        }
    }
    width = imageSize;
    height = imageSize;
    return true;
}

}