#ifndef BILILIVE_BILILIVE_LIVEHIME_VOLUME_SING_IDENTIFY_PROPERTY_PRESENTER_H_
#define BILILIVE_BILILIVE_LIVEHIME_VOLUME_SING_IDENTIFY_PROPERTY_PRESENTER_H_

#include "bililive/bililive/livehime/obs/srcprop_ui_util.h"
#include "obs/obs-studio/libobs/obs.hpp"
#include "bililive/secret/public/live_streaming_service.h"
#include "bililive/bililive/livehime/server_broadcast/broadcast_service.h"

#include "base/timer/timer.h"
#include <queue>
#include <mutex>


class SingIdentifyPropertyPresenter
{
public:
    enum class SingIdentifyResult
    {
        IDENTIFY_RESULT_NULL = -1,
        IDENTIFY_RESULT_NOT_SING,
        IDENTIFY_RESULT_SINGING
    };

    SingIdentifyPropertyPresenter();

    ~SingIdentifyPropertyPresenter();

     static void OnReceiverSignal(void* data, calldata_t* params);

     void SwitchArea(bool switch_area) { switch_area_ = switch_area; }
private:
    void RegistSignal();
    void SingIdentifyTimer(bool start);
    void CheckSingIdentifyResult();
    void ClearQueueData();

private:
    std::unique_ptr<MicphoneDeviceHelper> micphone_helper_;
    std::unique_ptr<base::Timer>          sing_identify_timer_;
    std::queue<SingIdentifyResult>        result_queue_;
    std::mutex                            queue_mtx_;
    bool                                  re_check_;
    bool                                  switch_area_;
    int short_time_size_; //短时判断 时间范围
    int short_time_sing_; //短时判断 唱歌时间
    int long_time_size_;  //长时判断 时间范围
    int long_time_sing_;  //长时判断 唱歌时间

    SingIdentifyResult                    last_result_;
    SingIdentifyResult                    last_short_result_;

    OBSSignal                             receiver_signal_;

    DISALLOW_COPY_AND_ASSIGN(SingIdentifyPropertyPresenter);
};

#endif  // BILILIVE_BILILIVE_LIVEHIME_VOLUME_SING_IDENTIFY_PROPERTY_PRESENTER_H_