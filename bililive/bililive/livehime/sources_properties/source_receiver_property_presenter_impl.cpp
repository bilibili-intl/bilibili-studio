#include "bililive/bililive/livehime/sources_properties/source_receiver_property_presenter_impl.h"

#include <map>
#include <regex>
#include <sstream>

#include "base/ext/bind_lambda.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/process/launch.h"

#include "obs/obs_proxy/utils/obs_wrapper_impl_cast.h"

#include "bililive/bililive/livehime/event_report/event_report_center.h"
#include "bililive/bililive/utils/setting_util.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/secret/public/event_tracking_service.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"


namespace
{
    enum receiver_status
    {
        RECEIVER_START = 0,
        RECEIVER_RUNNING,
        RECEIVER_LISTEN,
        RECEIVER_RESET
    };

    enum ios_event_type
    {
        // 苹果服务状态
        IOS_EV_SRV_STATUS,
        // 获取到的设备数量
        IOS_EV_DEV_COUNT,
        // 连接状态
        IOS_EV_CONN_STATUS,
        // 流状态
        IOS_EV_RECV_STATUS,
    };

    enum ios_streaming_status
    {
        // 流出错
        IOS_SS_FAILED,
        // 开始接收流数据
        IOS_SS_STARTED,
        // 停止接收流数据
        IOS_SS_STOPPED,
        // 连接中断
        IOS_SS_INTR,
    };

    static const char kReceiverStatus[] = "screen_cast_receiver_status";
    static const char kRtmpStatus[] = "rtmp_status";

    static const char kIOSReceiverStatus[] = "ios_screen_cast_receiver_status";
    static const char kIOSEventType[] = "ios_ev_type";
    static const char kIOSIntValue[] = "ios_int_val";

} // namespace

bool SourceReceiverPropertyPresenterImpl::is_first_server_normal_post_ = true;
bool SourceReceiverPropertyPresenterImpl::is_first_connect_success_post_ = true;
bool SourceReceiverPropertyPresenterImpl::is_first_connect_break_post_ = true;

SourceReceiverPropertyPresenterImpl::SourceReceiverPropertyPresenterImpl(obs_proxy::SceneItem* scene_item,
    contracts::SourceReceiverCallback* callback)
    : contracts::SourceReceiverPropertyPresenter(scene_item)
    , receiver_scene_item_(scene_item)
    , callback_(callback)
	
{
    canary_.reset(new int(0));
}

SourceReceiverPropertyPresenterImpl::~SourceReceiverPropertyPresenterImpl()
{
  	    receiver_signal_.Disconnect();
        ios_receiver_signal_.Disconnect();
    
   
}

bool SourceReceiverPropertyPresenterImpl::Initialize() {

	source_ = impl_cast(receiver_scene_item_.GetItem())->AsSource();
    auto item_name = impl_cast(receiver_scene_item_.GetItem())->name();
	RegistSignal();

    return true;
}

int SourceReceiverPropertyPresenterImpl::GetRtmpPort()
{
    return receiver_scene_item_.GetRtmpPort();
}

std::string SourceReceiverPropertyPresenterImpl::GetEncoderInfo()
{
    return receiver_scene_item_.GetEncoderInfo();
}

float SourceReceiverPropertyPresenterImpl::GetVolumeValue()
{
    return MapFloatFromInt(receiver_scene_item_.Volume());
}

void SourceReceiverPropertyPresenterImpl::SetVolumeValue(float value)
{
    return receiver_scene_item_.Volume(MapFloatToInt(value));
}


bool SourceReceiverPropertyPresenterImpl::IsMuted()
{
    return receiver_scene_item_.IsMuted();
}

bool SourceReceiverPropertyPresenterImpl::IsComputerMic() {
    return receiver_scene_item_.IsComputerMic();
}

void SourceReceiverPropertyPresenterImpl::SeComputerMic(bool muted) {
    receiver_scene_item_.SetComputerMic(muted);
}

void SourceReceiverPropertyPresenterImpl::SetMuted(bool muted)
{
    receiver_scene_item_.SetMuted(muted);
}

void SourceReceiverPropertyPresenterImpl::SetLandscapeModel(bool is_landscape_model)
{
    receiver_scene_item_.SetLandscapeModel(is_landscape_model);
}

void SourceReceiverPropertyPresenterImpl::SetRecvType(ReceiverItemHelper::ReceiverType type) {
    receiver_scene_item_.SetRecvType(type);
}

ReceiverItemHelper::ReceiverType SourceReceiverPropertyPresenterImpl::GetRecvType() {
    return receiver_scene_item_.GetRecvType();
}

void SourceReceiverPropertyPresenterImpl::RegistSignal()
{
    auto signal_handle = obs_source_get_signal_handler(source_);
    receiver_signal_.Connect(
        signal_handle,
        kReceiverStatus,
        &SourceReceiverPropertyPresenterImpl::OnReceiverSignal, this);
    ios_receiver_signal_.Connect(
        signal_handle,
        kIOSReceiverStatus,
        &SourceReceiverPropertyPresenterImpl::OnIOSReceiverSignal, this);
}

// static
void SourceReceiverPropertyPresenterImpl::OnReceiverSignal(void* data, calldata_t* params)
{
    auto This = static_cast<SourceReceiverPropertyPresenterImpl*>(data);
    int status = static_cast<int>(calldata_int(params, kRtmpStatus));

    std::weak_ptr<int> weak_self = This->canary_;
    switch (status)
    {
    case RECEIVER_START:
    {
        BililiveThread::PostTask(
            BililiveThread::UI,
            FROM_HERE,
            base::BindLambda([weak_self, This]
        {
            auto ptr = weak_self.lock();
            if (ptr && This->callback_)
            {
                This->callback_->ReceiverStartCallback();
            }
        }));
    }
    break;
    case RECEIVER_RUNNING:
    {
        BililiveThread::PostTask(
            BililiveThread::UI,
            FROM_HERE,
            base::BindLambda([weak_self, This]
        {
            auto ptr = weak_self.lock();
            if (ptr && This->callback_)
            {
                This->callback_->ReceiverStatusCallback(true);
            }

            // 服务端埋点
            if (ptr)
            {
                std::string ec = This->receiver_scene_item_.GetEncoderInfo();
                int blink = livehime::CheckThirdpartyStreamEncoderType(ec);
                livehime::BehaviorEventReportViaServer(secret::LivehimeViaServerBehaviorEvent::ScanQRCode,
                    base::StringPrintf("screencast_type=%d", (blink == 0) ? 2 : 1));

                LOG(INFO) << "[Receiver Source] receive stream from " << ec;
            }
        }));
    }
    break;
    case RECEIVER_LISTEN:
    {
        BililiveThread::PostTask(
            BililiveThread::UI,
            FROM_HERE,
            base::BindLambda([weak_self, This]
        {
            auto ptr = weak_self.lock();
            if (ptr && This->callback_)
            {
               This->callback_->ReceiverStatusCallback(false);
            }
        }));
    }
    break;
	case RECEIVER_RESET:
	{
		BililiveThread::PostTask(
			BililiveThread::UI,
			FROM_HERE,
			base::BindLambda([weak_self, This]
				{
					auto ptr = weak_self.lock();
					if (ptr && This->callback_)
					{
                        This->callback_->ReceiverResetCallback();
					}
			}));
	}
	break;


    default:
        return;
    }
}

// static
void SourceReceiverPropertyPresenterImpl::OnIOSReceiverSignal(void* data, calldata_t* params) {
    auto This = static_cast<SourceReceiverPropertyPresenterImpl*>(data);
    int ev_type = static_cast<int>(calldata_int(params, kIOSEventType));
    int val = static_cast<int>(calldata_int(params, kIOSIntValue));

    auto room_id = GetBililiveProcess()->secret_core()->user_info().room_id();
    std::weak_ptr<int> weak_self = This->canary_;
    switch (ev_type)
    {
    case IOS_EV_SRV_STATUS:

    {
        // val: 0 表示服务异常，1 表示服务正常
		 BililiveThread::PostTask(
			BililiveThread::UI,
			FROM_HERE,
			base::BindLambda([weak_self,val ,room_id,This]
			{   
				auto ptr = weak_self.lock();
                if (ptr && is_first_server_normal_post_ && (val == 0)){

                       is_first_server_normal_post_ = false;
                       livehime::TechnologyEventReport(secret::LivehimeBehaviorEvent::LivehimeIosWiredStatus, base::StringPrintf("result:%d;type:%d;roomid:%lld",2,1, room_id));
                 }

				

			}));

        break;
    }

    case IOS_EV_DEV_COUNT:
        // val: 表示获取到的设备数
        break;

    case IOS_EV_CONN_STATUS:
    {   // val: 0 表示连接成功，1 表示连接失败

        BililiveThread::PostTask(
            BililiveThread::UI,
            FROM_HERE,
            base::BindLambda([weak_self, val, room_id, This]
            {
                  auto ptr = weak_self.lock();
                  if (ptr && This->callback_){
  
                       This->callback_->ReceiverStatusCallback(true);
                  }
				
				  if (ptr && is_first_connect_success_post_ &&(val == 0)) {

                      is_first_connect_success_post_ = false;
					  livehime::TechnologyEventReport(secret::LivehimeBehaviorEvent::LivehimeIosWiredStatus, base::StringPrintf("result:%d;type:%d;roomid:%lld", 1, 0, room_id));
				  }
                 
             }));


        break;
    }

    case IOS_EV_RECV_STATUS:
    {
      // val: 看上面 ios_streaming_status 枚举
		BililiveThread::PostTask(
			BililiveThread::UI,
			FROM_HERE,
			base::BindLambda([weak_self, val, room_id, This]
			{
			      auto ptr = weak_self.lock();
				  if (ptr && (This->callback_) && (val == IOS_SS_STARTED)){

					   This->callback_->ReceiverStartCallback();
                       
				  }
				  if (ptr && is_first_connect_break_post_ && (val == IOS_SS_INTR)) {

                      is_first_connect_break_post_ = false;
					  livehime::TechnologyEventReport(secret::LivehimeBehaviorEvent::LivehimeIosWiredStatus, base::StringPrintf("result:%d;type:%d;roomid:%lld", 3, 0, room_id));
				  }
                   
			}));
     
    
     break;
    }    

    default:
        break;
    }
}


void SourceReceiverPropertyPresenterImpl::Snapshot()
{
    snapshot_ = livehime::ReceiverPropertySnapshot::NewTake(&receiver_scene_item_);
}

void SourceReceiverPropertyPresenterImpl::Restore()
{
    DCHECK(snapshot_);

    snapshot_->Restore();
}

void SourceReceiverPropertyPresenterImpl::ReconnectIOSSignal()
{
	source_ = impl_cast(receiver_scene_item_.GetItem())->AsSource();
	auto item_name = impl_cast(receiver_scene_item_.GetItem())->name();
}