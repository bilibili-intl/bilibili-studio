#ifndef BILIBASE_STRING_ENCODING_CONVERSIONS_H_
#define BILIBASE_STRING_ENCODING_CONVERSIONS_H_

#include <string>

namespace bilibase {

std::string WideToUTF8(const std::wstring& wide_str);
std::wstring UTF8ToWide(const std::string& utf_str);

// Don't use these functions to convert strings that might contain non-ASCII characters.
std::wstring ASCIIToWide(const std::string& ascii_str);
std::string WideToASCII(const std::wstring& wide_str);

std::string UrlDecode(const std::string& str);

}   // namespace bilibase

#endif  // BILIBASE_STRING_ENCODING_CONVERSIONS_H_