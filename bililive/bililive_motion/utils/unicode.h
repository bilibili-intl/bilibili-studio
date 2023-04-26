#ifndef BILILIVE_MOTION_UTILS_UNICODE_H_
#define BILILIVE_MOTION_UTILS_UNICODE_H_

#include <string>


namespace livehime {
namespace utl {

    std::wstring UTF8ToUTF16(const std::string& text);
    std::string UTF16ToUTF8(const std::wstring& text);

}
}

#endif  // BILILIVE_MOTION_UTILS_UNICODE_H_