#include "bilibase/string_encoding_conversions.h"

#include <codecvt>
#include <locale>
#include <algorithm>
#include <assert.h>


namespace bilibase {

std::string WideToUTF8(const std::wstring& wide_str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t, 0x10ffff, std::little_endian>> converter;
    return converter.to_bytes(wide_str);
}

std::wstring UTF8ToWide(const std::string& utf_str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t, 0x10ffff, std::little_endian>> converter;
    return converter.from_bytes(utf_str);
}

std::wstring ASCIIToWide(const std::string& ascii_str)
{
    std::wstring result;
    result.reserve(ascii_str.size());
    std::transform(ascii_str.begin(), ascii_str.end(), 
        std::back_inserter(result),
        [](char c)->wchar_t { return (wchar_t)c; });
    return result;
}

std::string WideToASCII(const std::wstring& wide_str)
{
    std::string result;
    result.reserve(wide_str.size());
    std::transform(wide_str.begin(), wide_str.end(),
        std::back_inserter(result),
        [](wchar_t c)->char { return (char)c; });
    return result;
}

unsigned char ToHex(unsigned char x)
{
    return x > 9 ? x + 55 : x + 48;
}

unsigned char FromHex(unsigned char x)
{
    unsigned char y = '0';

    if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
    else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
    else if (x >= '0' && x <= '9') y = x - '0';
    else assert(0);

    return y;
}

std::string UrlEncode(const std::string& str)
{
    std::string str_temp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        if (isalnum((unsigned char)str[i]) ||
            (str[i] == '-') ||
            (str[i] == '_') ||
            (str[i] == '.') ||
            (str[i] == '~'))
            str_temp += str[i];
        else if (str[i] == ' ')
            str_temp += "+";
        else
        {
            str_temp += '%';
            str_temp += ToHex((unsigned char)str[i] >> 4);
            str_temp += ToHex((unsigned char)str[i] % 16);
        }
    }
    return str_temp;
}

std::string UrlDecode(const std::string& str)
{
    std::string str_temp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        if (str[i] == '+') str_temp += ' ';
        else if (str[i] == '%')
        {
            assert(i + 2 < length);
            unsigned char high = FromHex((unsigned char)str[++i]);
            unsigned char low = FromHex((unsigned char)str[++i]);
            str_temp += high * 16 + low;
        }
        else str_temp += str[i];
    }
    return str_temp;
}

}   // namespace bilibase