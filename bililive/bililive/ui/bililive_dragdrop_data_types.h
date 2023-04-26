#ifndef BILILIVE_BILILIVE_UI_BILILIVE_DRAGDROP_DATA_TYPES_H
#define BILILIVE_BILILIVE_UI_BILILIVE_DRAGDROP_DATA_TYPES_H

#include <string>

namespace bililive
{
    struct DropFileParams
    {
        int preview_width;
        int preview_height;
        int x;
        int y;
        std::wstring file_path;
    };

    struct DropStringParams
    {
        int preview_width;
        int preview_height;
        int x;
        int y;
        std::wstring text;
    };
};

#endif
