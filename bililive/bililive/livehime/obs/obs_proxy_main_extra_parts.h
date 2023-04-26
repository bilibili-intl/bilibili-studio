/*
 @ 0xCCCCCCCC
*/

#ifndef BILILIVE_BILILIVE_LIVEHIME_OBS_OBS_PROXY_MAIN_EXTRA_PARTS_H_
#define BILILIVE_BILILIVE_LIVEHIME_OBS_OBS_PROXY_MAIN_EXTRA_PARTS_H_

#include "bililive/public/bililive/bililive_main_extra_parts.h"

namespace livehime {

class OBSProxyMainExtraParts : public BililiveMainExtraParts {
public:
    OBSProxyMainExtraParts() = default;

    ~OBSProxyMainExtraParts() = default;

    void PreBililiveStart() override;

    void PostBililiveStart() override;

    void BililiveTearDown() override;
};

}

#endif  // BILILIVE_BILILIVE_LIVEHIME_OBS_OBS_PROXY_MAIN_EXTRA_PARTS_H_
