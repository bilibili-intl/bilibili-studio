#include "bililive/bililive/livehime/third_party_streaming/third_party_output_controller.h"
#include "base/ext/callable_callback.h"
#include "base/strings/utf_string_conversions.h"
#include "bililive/bililive/livehime/danmaku_hime/danmakuhime_data_handler.h"
#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"

namespace livehime
{

    void ThirdPartyOutputController::SetSeiInfo(const uint8_t* data, int len)
    {
        if (processor_)
        {
            processor_->SetSeiInfo(data, len);
        }
    }

}