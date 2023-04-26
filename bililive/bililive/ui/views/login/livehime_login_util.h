#ifndef BILILIVE_UI_VIEWS_LOGIN_UTIL_H
#define BILILIVE_UI_VIEWS_LOGIN_LOGIN_UTIL_H

#include <string>

#include "base/basictypes.h"

#include "cef_proxy_dll/public/livehime_cef_proxy_calldata.h"

namespace livehime
{
    std::string GetAvatarBinaryDataByMid(int64 mid);

    std::string CefCookiesToString(const cef_proxy::cookies& cookies);

    cef_proxy::cookies StringToCefCookies(const std::string& str);
}

#endif