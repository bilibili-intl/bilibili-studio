#include "bililive/bililive/utils/bililive_filetype_exts.h"

#include "base/basictypes.h"
#include "base/strings/stringprintf.h"

#include "bililive/bililive/livehime/obs/source_creator.h"

namespace bililive
{
    static const wchar_t *mediaFileExts[] =
    {
        L"avi", L"mp4", L"flv", L"rmvb", L"rm", L"mpg", L"mpeg", L"mkv", L"webm", L"3gp", L"wmv", L"asf", L"mp3", L"ogg", L"aac", L"wma", 0
    };

    const wchar_t **GetMediaFileExts()
    {
        return mediaFileExts;
    }

    static const wchar_t *imageFileExts[] =
    {
        L"jpg", L"jpeg", L"gif", L"png", L"bmp", 0
    };

    const wchar_t **GetImageFileExts()
    {
        return imageFileExts;
    }

    static const wchar_t *textFileExts[] =
    {
        L"txt", L"log", 0
    };

    const wchar_t **GetTextFileExts()
    {
        return textFileExts;
    }

    bool GuessSourceType(const base::FilePath& filePath, SourceType& source_type)
    {
        for (int i = 0; i < arraysize(mediaFileExts); i++)
        {
            if (filePath.MatchesExtension(base::StringPrintf(L".%ls", mediaFileExts[i])))
            {
                source_type = SourceType::Media;
                return true;
            }
        }

        for (int i = 0; i < arraysize(imageFileExts); i++)
        {
            if (filePath.MatchesExtension(base::StringPrintf(L".%ls", imageFileExts[i])))
            {
                source_type = SourceType::Image;
                return true;
            }
        }

        for (int i = 0; i < arraysize(textFileExts); i++)
        {
            if (filePath.MatchesExtension(base::StringPrintf(L".%ls", textFileExts[i])))
            {
                source_type = SourceType::Text;
                return true;
            }
        }

        return false;
    }
}