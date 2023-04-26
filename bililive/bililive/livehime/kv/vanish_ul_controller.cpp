#include "vanish_ul_controller.h"

#include "base/ext/callable_callback.h"

#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"


namespace livehime {

    // static
    bool VanishULController::ul_vanished_ = false;

    // static
    void VanishULController::RequestULVanishingStatus() {

    }

    // static
    bool VanishULController::IsULVanished() {
        return ul_vanished_;
    }

    // static
    void VanishULController::OnGetULVanishingSwitch(
        bool valid, int code, const std::string& err_msg, bool vanished)
    {
        if (valid && code == 0) {
            ul_vanished_ = vanished;
        }
    }

}
