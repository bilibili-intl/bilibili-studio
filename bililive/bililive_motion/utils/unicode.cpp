#include "unicode.h"

#include <fstream>

#include <Windows.h>


namespace livehime {
namespace utl {

    std::wstring UTF8ToUTF16(const std::string& text) {
        int count = ::MultiByteToWideChar(CP_UTF8, 0, text.c_str(), text.length(), nullptr, 0);
        if (count <= 0) {
            return {};
        }

        std::wstring out;
        out.resize(count);
        count = ::MultiByteToWideChar(CP_UTF8, 0, text.c_str(), text.length(), &out[0], out.size());
        if (count <= 0) {
            return {};
        }

        return out;
    }

    std::string UTF16ToUTF8(const std::wstring& text) {
        int count = ::WideCharToMultiByte(
            CP_UTF8, 0, text.c_str(), text.length(), nullptr, 0, nullptr, nullptr);
        if (count <= 0) {
            return {};
        }

        std::string out;
        out.resize(count);
        count = ::WideCharToMultiByte(
            CP_UTF8, 0, text.c_str(), text.length(), &out[0], out.size(), nullptr, nullptr);
        if (count <= 0) {
            return {};
        }

        return out;
    }

}
}