/*
 @ 0xCCCCCCCC
*/

#ifndef BILILIVE_BILILIVE_UI_RESOURCE_HELPER_H_
#define BILILIVE_BILILIVE_UI_RESOURCE_HELPER_H_

#include "ui/base/resource/resource_bundle.h"

namespace bililive {

inline std::wstring LocalStr(int res_id)
{
    return ResourceBundle::GetSharedInstance().GetLocalizedString(res_id);
}

}   // namespace bililive

#endif  // BILILIVE_BILILIVE_UI_RESOURCE_HELPER_H_
