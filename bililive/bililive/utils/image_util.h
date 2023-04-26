#ifndef BILILIVE_BILILIVE_UTILS_IMAGE_UTIL_H
#define BILILIVE_BILILIVE_UTILS_IMAGE_UTIL_H


#include "base/files/file_path.h"


namespace bililive
{
    enum class ImageType
    {
        IT_UNKNOW,
        IT_JPG,
        IT_PNG,
        IT_GIF,
    };

    ImageType GetImageTypeFromFile(const base::FilePath &file_path);
    ImageType GetImageTypeFromBinary(const unsigned char * const data, unsigned int len);
    std::vector<char> GetBinaryFromFile(const base::FilePath &file_path);
}

#endif