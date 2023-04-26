#include "bililive/bililive/livehime/volume/sing_identify_property_presenter.h"

#include "base/strings/utf_string_conversions.h"
#include "base/path_service.h"

#include "bililive/bililive/livehime/event_report/event_report_center.h"
#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/livehime/volume/bililive_volume_controllers_devices_contract.h"
#include "bililive/bililive/livehime/volume/bililive_volume_controllers_devices_present_impl.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/common/bililive_paths.h"
#include "obs/obs_proxy/public/proxy/obs_scene_collection.h"
#include "obs/obs_proxy/public/common/pref_constants.h"
#include "obs/obs_proxy/public/common/pref_names.h"
#include "obs/obs_proxy/utils/obs_wrapper_impl_cast.h"
#include "bililive/secret/public/event_tracking_service.h"

namespace {
    static const char  kSingIdentifyReceiverStatus[] = "sing_identify_receiver_status";
    static const char  kSingIdentifyResult[] = "status";

    //唱歌识别2秒检测一次
#define SingIdentifyStep 2
}

SingIdentifyPropertyPresenter::SingIdentifyPropertyPresenter()
    : sing_identify_timer_(nullptr)
    , micphone_helper_(nullptr)
    , short_time_size_(0)
    , short_time_sing_(0)
    , long_time_size_(0)
    , long_time_sing_(0)
    , last_short_result_(SingIdentifyResult::IDENTIFY_RESULT_NULL)
    , last_result_(SingIdentifyResult::IDENTIFY_RESULT_NULL)
    , re_check_(false)
    , switch_area_(false)
{
    std::unique_ptr<contracts::BililiveAudioDevicesContract> audio_device(std::make_unique<BililiveAudioDevicesPresenterImpl>());
    audio_device->SelectAudioDevice(base::UTF8ToUTF16(obs_proxy::kDefaultInputAudio));

    if (audio_device->DeviceIsValid()){
        micphone_helper_ = std::make_unique<MicphoneDeviceHelper>(audio_device->GetAudioDevice());
    }
    else {  //没有可用的麦克风失败，功能不启用
        LOG(INFO) << "SingIdentifyPropertyPresenter::audio_device->DeviceIsValid() = false,micphone_helper_ = null";
    }
    DCHECK(micphone_helper_);
}

SingIdentifyPropertyPresenter::~SingIdentifyPropertyPresenter()
{
    LOG(INFO) << __FUNCTION__;
    if (micphone_helper_) {
        if (re_check_) {//广播状态重新检测
            if (switch_area_) {
                livehime::BehaviorEventReportViaServerNew(secret::LivehimeViaServerBehaviorEventNew::SingIdentifyLeave, "");
            }
            else {
                livehime::BehaviorEventReportViaServerNew(secret::LivehimeViaServerBehaviorEventNew::SingIdentifyCallbackStop, "");
            }
        }
        else {
            if (last_result_ == SingIdentifyResult::IDENTIFY_RESULT_SINGING) {
                if (switch_area_) {
                    livehime::BehaviorEventReportViaServerNew(secret::LivehimeViaServerBehaviorEventNew::SingIdentifyLeave, "");
                }
                else {
                    livehime::BehaviorEventReportViaServerNew(secret::LivehimeViaServerBehaviorEventNew::SingIdentifyStop, "");
                }
            }
        }
        SingIdentifyTimer(false);
        micphone_helper_->SingIdentifyEnable(false);
    }
}

void SingIdentifyPropertyPresenter::OnReceiverSignal(void* data, calldata_t* params)
{
    if (data == nullptr) return;

    SingIdentifyPropertyPresenter* pthis = static_cast<SingIdentifyPropertyPresenter*>(data);
    int status = static_cast<int>(calldata_int(params, kSingIdentifyResult));
    if (pthis) {
        std::lock_guard<std::mutex> lk(pthis->queue_mtx_);
        pthis->result_queue_.push((SingIdentifyResult)status);
    }
}

void SingIdentifyPropertyPresenter::RegistSignal()
{
    if (micphone_helper_) {
        auto source = impl_cast(micphone_helper_->GetItem())->LeakUnderlyingSource();
        auto filter = micphone_helper_->GetItem()->GetFilter("sing_identify_filter");
        obs_source_t* filter_source = obs_source_get_filter_by_name(source, "sing_identify_filter");
        if (filter_source) {
            auto signal_handle = obs_source_get_signal_handler(filter_source);
            receiver_signal_.Connect(
                signal_handle,
                kSingIdentifyReceiverStatus,
                &SingIdentifyPropertyPresenter::OnReceiverSignal,
                this);
        }
    }
}

void SingIdentifyPropertyPresenter::SingIdentifyTimer(bool start)
{
    if (start) {
        if (nullptr == sing_identify_timer_) {
            sing_identify_timer_ = std::make_unique<base::Timer>(
                FROM_HERE,
                base::TimeDelta::FromMilliseconds(SingIdentifyStep*1000),
                base::Bind(&SingIdentifyPropertyPresenter::CheckSingIdentifyResult, base::Unretained(this)),
                true);
            sing_identify_timer_->Reset();
        }
    }
    else {
        if (sing_identify_timer_ != nullptr &&
            sing_identify_timer_.get() != nullptr)
        {
            sing_identify_timer_.reset();
            sing_identify_timer_ = nullptr;
        }
        ClearQueueData();
    }
}


// 短时判断逻辑：15秒有10s唱歌为 唱歌状态
// 长时判断逻辑：60秒有36秒唱歌为 唱歌状态
// 两个判断都为否的时候 为非唱歌状态
void SingIdentifyPropertyPresenter::CheckSingIdentifyResult()
{
    SingIdentifyResult value;
    {
        std::lock_guard<std::mutex> lk(queue_mtx_);
        if (!result_queue_.empty()) {
            value = result_queue_.front();
            result_queue_.pop();
        }
        else  return;
    }
    bool short_update = false;
    bool long_update = false;

    //LOG(INFO) << "SingResult:" << (int)value;

}

void SingIdentifyPropertyPresenter::ClearQueueData()
{
    std::lock_guard<std::mutex> lk(queue_mtx_);
    std::queue<SingIdentifyResult> empty;
    result_queue_.swap(empty);
    short_time_size_ = 0;
    short_time_sing_ = 0;
    long_time_size_ = 0;
    long_time_sing_ = 0;
    last_short_result_ = SingIdentifyResult::IDENTIFY_RESULT_NULL;
    last_result_ = SingIdentifyResult::IDENTIFY_RESULT_NULL;
}
