#include "image_util.h"

#include <memory>

#include "base/threading/thread_restrictions.h"
#include "base/file_util.h"


namespace
{
    static const char gif87a[6] = { 'G', 'I', 'F', '8', '7', 'a' };
    static const char gif89a[6] = { 'G', 'I', 'F', '8', '9', 'a' };
    static const char jpg_sign[3] = { (char)0xFF, (char)0xD8, (char)0xFF };
    static const char png_sign[8] = { (char)0x89, (char)0x50, (char)0x4e,
        (char)0x47, (char)0x0d, (char)0x0a,
        (char)0x1a, (char)0x0a };
}

namespace bililive
{
    ImageType GetImageTypeFromFile(const base::FilePath &file_path)
    {
        ImageType type = ImageType::IT_UNKNOW;
        base::ThreadRestrictions::ScopedAllowIO allow;
        base::FilePath path(file_path);
        int64 file_size = 0;
        if (file_util::GetFileSize(path, &file_size))
        {
            std::unique_ptr<unsigned char[]> data(new unsigned char[file_size]);
            if (file_util::ReadFile(path, (char*)data.get(), static_cast<int>(file_size)) == file_size)
            {
                type = GetImageTypeFromBinary(data.get(), static_cast<unsigned int>(file_size));
            }
        }
        return type;
    }

    ImageType GetImageTypeFromBinary(const unsigned char * const data, unsigned int len)
    {
        ImageType type = ImageType::IT_UNKNOW;

        if (data && len > 0)
        {
            if (memcmp(data, jpg_sign, sizeof(jpg_sign)) == 0)
            {
                type = ImageType::IT_JPG;
            }
            else if (memcmp(data, png_sign, sizeof(png_sign)) == 0)
            {
                type = ImageType::IT_PNG;
            }
            else if (memcmp(data, gif87a, sizeof(gif87a)) == 0 || memcmp(data, gif89a, sizeof(gif89a)) == 0)
            {
                type = ImageType::IT_GIF;
            }
        }
        return type;
    }

    std::vector<char> GetBinaryFromFile(const base::FilePath &file_path)
    {
        base::ThreadRestrictions::ScopedAllowIO allow;
        int64 file_size = 0;
        if (file_util::GetFileSize(file_path, &file_size))
        {
            std::vector<char> data(file_size);
            if (file_util::ReadFile(file_path, &data[0], static_cast<int>(file_size)) == file_size)
            {
                return std::move(data);
            }
        }
        return std::vector<char>();
    }
}