#ifndef BILILIVE_PATH_H_
#define BILILIVE_PATH_H_

#include <string>

std::wstring BiliMakeExecFullPath(const std::wstring& filename);
std::wstring BiliMakeExecFullPathWithQuotes(const std::wstring& filename);

#endif