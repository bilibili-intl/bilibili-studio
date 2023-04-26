#include "bililive_path.h"
#include "bililive/common/bililive_context.h"

std::wstring BiliMakeExecFullPath(const std::wstring& filename)
{
    std::wstring r = BililiveContext::Current()->GetMainDirectory().Append(base::FilePath::FromUTF16Unsafe(filename.c_str())).AsUTF16Unsafe();
    return r;
}

std::wstring BiliMakeExecFullPathWithQuotes(const std::wstring& filename)
{
    return L"\"" + BiliMakeExecFullPath(filename) + L"\"";
}
