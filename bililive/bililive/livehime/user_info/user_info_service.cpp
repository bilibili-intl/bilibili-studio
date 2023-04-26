#include "bililive/bililive/livehime/user_info/user_info_service.h"

#include "bilibase/scope_guard.h"

#include "bililive/bililive/livehime/danmaku_hime/danmakuhime_data_handler.h"
#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/livehime/server_broadcast/broadcast_service.h"
#include "bililive/bililive/livehime/function_control/app_function_controller.h"
#include "bililive/bililive/ui/bililive_commands.h"
#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/common/bililive_constants.h"
#include "bililive/public/common/pref_names.h"
#include "bililive/secret/core/anchor_info_impl.h"

#include "base/ext/callable_callback.h"
#include "base/file_util.h"
#include "base/logging.h"
#include "base/notification/notification_service.h"
#include "base/prefs/pref_service.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/strings/string_number_conversions.h"
#include "base/threading/thread_restrictions.h"

#include "ui/base/resource/resource_bundle.h"

#include "grit/generated_resources.h"


const int kVirtualLiveChannelID = 9;  //第三方使用虚拟摄像头在虚拟主播分区开播的不显示胜利礼物，本身直播姬虚拟开播是不显示胜利礼物的
const int kRadioLiveChannelID = 5;    //电台开播不显示胜率礼物

namespace {
    UserInfoService *g_user_info_service_ = nullptr;

    using StartLiveInfo = secret::LiveStreamingService::StartLiveInfo;

    ResourceBundle& res()
    {
        return ResourceBundle::GetSharedInstance();
    }

    const int kRadioChannelID = 5;// 电台分区不显示摄像头画面
}

UserInfoService::UserInfoService() : weak_ptr_factory_(this) {
    DCHECK(g_user_info_service_ == nullptr);
    g_user_info_service_ = this;
}

UserInfoService::~UserInfoService() {
    g_user_info_service_ = nullptr;
}

void UserInfoService::GetUserInfo() {

    GetBililiveProcess()->secret_core()->user_account_service()->GetViddupUserInfo({},
        base::MakeCallable(
            base::Bind(&UserInfoService::OnGetViddupUserInfo, weak_ptr_factory_.GetWeakPtr()))).Call();

}

void UserInfoService::OnGetViddupUserInfo(bool valid_response, int code,
    const secret::UserAccountService::ViddupUserInfoData& info) {
    if (valid_response && code == 0) {
        if (info.face.empty()) {
            return;
        }

        GetBililiveProcess()->secret_core()->user_info().set_nickname(info.uname);
        GetUserAvatar(info.face);
    }
}

void UserInfoService::GetUserAvatar(const std::string& url) {
    if (url.empty()) {
        return;
    }

    // 从本地配置文件读取以前服务端返回的头像的缓存标识，用于服务端判断是否需要实际下发头像数据
    std::string avatar_etag = GetBililiveProcess()->profile()->GetPrefs()->GetString(prefs::kAvatarCacheTag);
    // 查看一下本地缓存的文件还在不在，不在了就直接无视本地的头像缓存标识，否则如果配置文件还记录着缓存
    // 但是本地文件却因为某种原因没了，而用户又没有进行头像更新，那么本地始终不会重新下载/GET到头像数据
    {
        int64 file_size = 0;
        auto avatar_path = GetAvatarPath();
        base::ThreadRestrictions::ScopedAllowIO allow_io;
        file_util::GetFileSize(avatar_path, &file_size);
        if (0 == file_size)
        {
            avatar_etag.clear();
            GetBililiveProcess()->profile()->GetPrefs()->SetString(prefs::kAvatarCacheTag, "");
        }
    }
    GetBililiveProcess()->secret_core()->user_account_service()->GetUserAvatar(url, avatar_etag,
        base::MakeCallable(
        base::Bind(&UserInfoService::OnGetUserAvatar, weak_ptr_factory_.GetWeakPtr()))).Call();
}

void UserInfoService::OnGetUserAvatar(bool success,
    const secret::UserAccountService::UserAvatarInfo& avatar_info) {
    if (!success) {
        return;
    }

    if (avatar_info.incomplete) {
        auto event_msg = base::StringPrintf("contentlength:%d;datasize:%d",
            avatar_info.content_length, avatar_info.avatar.size());

        auto secret_core = GetBililiveProcess()->secret_core();
        secret_core->event_tracking_service()->ReportLivehimeBehaviorEvent(
            secret::LivehimeBehaviorEvent::LivehimeIncompleteAvatar,
            secret_core->account_info().mid(), event_msg).Call();

        return;
    } else {
        if (!avatar_info.etag.empty()) {
            GetBililiveProcess()->profile()->GetPrefs()->SetString(prefs::kAvatarCacheTag, avatar_info.etag);
        }

        auto avatar_path = GetAvatarPath();
        if (!avatar_info.avatar.empty()) {
            base::ThreadRestrictions::ScopedAllowIO allow_io;
            file_util::WriteFile(avatar_path, avatar_info.avatar.data(), avatar_info.avatar.size());

            GetBililiveProcess()->secret_core()->user_info().set_avatar(
                reinterpret_cast<const unsigned char*>(avatar_info.avatar.data()), avatar_info.avatar.size());
        } else {
            base::ThreadRestrictions::ScopedAllowIO allow_io;
            if (base::PathExists(avatar_path)) {
                std::string content;
                if (file_util::ReadFileToString(avatar_path, &content)) {
                    GetBililiveProcess()->secret_core()->user_info().set_avatar(
                        reinterpret_cast<const unsigned char*>(content.data()), content.length());
                }
            }
        }

        base::NotificationService::current()->Notify(bililive::NOTIFICATION_LIVEHIME_UPDATE_AVATAR,
            base::NotificationService::AllSources(),
            base::NotificationService::NoDetails());
    }
}

void UserInfoService::GetUserSan() {

}

void UserInfoService::OnGetUserSan(bool successed, int64_t san) {
    if (!successed) {
        return;
    }

    GetBililiveProcess()->secret_core()->user_info().set_san(san);

    base::NotificationService::current()->Notify(bililive::NOTIFICATION_LIVEHIME_UPDATE_SAN,
        base::NotificationService::AllSources(),
        base::NotificationService::NoDetails());
}

void UserInfoService::CreateLiveRoom(int from_type)
{
    liveroom_create_type_ = from_type;

    if (liveroom_creating_){
        return;
    }
    liveroom_creating_ = true;
}

void UserInfoService::OnCreateLiveRoom(bool valid_response, int code, const std::string& err_msg, int64_t roomid)
{
    CreateRoomInfo info;
    info.valid_response = valid_response;
    if (valid_response && code == secret::LiveStreamingService::OK){
        DCHECK(roomid > 0);
        LOG_IF(WARNING, roomid <= 0) << "create liveroom success but roomid invalid " << roomid;
        if (roomid > 0){
            // 有房间号了，刷新一下房间信息。把建房的来源传递一下，在后续的房间信息加载完毕之后要继续执行
            // 建房时发起的业务流程
            GetRoomInfo();
        }
        else{
            liveroom_creating_ = false;
            info.code = code;
            info.room_id = roomid;
            info.err_msg = err_msg;
            base::NotificationService::current()->Notify(bililive::NOTIFICATION_LIVEHIME_CREATE_LIVEROOM_FAILED,
                base::NotificationService::AllSources(),
                base::Details<CreateRoomInfo>(&info));
        }
    }
    else{
        LOG(WARNING) << "create liveroom failed " << err_msg;

        liveroom_creating_ = false;
        info.code = code;
        info.room_id = roomid;
        info.err_msg = err_msg;
        base::NotificationService::current()->Notify(bililive::NOTIFICATION_LIVEHIME_CREATE_LIVEROOM_FAILED,
            base::NotificationService::AllSources(),
            base::Details<CreateRoomInfo>(&info));
    }
}

// 海外开播 - 增加宏区别海外开播接口
void UserInfoService::GetRoomInfo() {
    auto mid = GetBililiveProcess()->secret_core()->account_info().mid();

    GetBililiveProcess()->secret_core()->live_streaming_service()->GetIntlRoomInfo(mid,
        base::MakeCallable(base::Bind(&UserInfoService::OnGetIntlRoomInfo, weak_ptr_factory_.GetWeakPtr()))).Call();

}

// 海外开播 - 房间信息接口
void UserInfoService::OnGetIntlRoomInfo(bool valid_response, int code, const std::string& err_msg,
    const secret::LiveStreamingService::IntlRoomInfo& info) {

    ON_SCOPE_EXIT{ liveroom_creating_ = false; };

    if (!valid_response || code != 0) {
        return;
    }
    StartLiveInfo start_live_info;
    start_live_info.rtmp_info.protocol = "rtmp";
    start_live_info.rtmp_info.live_key = info.stream_key;
    start_live_info.rtmp_info.new_link = info.backup_address;
    start_live_info.rtmp_info.addr = info.server_address;
    start_live_info.rtmp_info.key = info.stream_key;
    start_live_info.protocol_list.push_back(start_live_info.rtmp_info);

    if (code == secret::LiveStreamingService::OK) {
        // 海外开播 - 通过接口获取房间号
        GetBililiveProcess()->secret_core()->user_info().set_room_id(info.room_id);
        //bililive::OutputController::GetInstance()->StartStreaming(start_live_info);

        //update roominfo
        static bool first_time = true;
        base::NotificationService* notification_service = base::NotificationService::current();
        notification_service->Notify(bililive::NOTIFICATION_LIVEHIME_LOAD_ROOMINFO_SUCCESS,
            base::NotificationService::AllSources(),
            base::Details<bool>(&first_time));
        first_time = false;

    }
}

base::FilePath UserInfoService::GetAvatarPath() {
    return GetBililiveProcess()->profile()->GetPath().Append(bililive::kUserFaceFileName);
}

void UserInfoService::UpdateLiveRoomInfo(const LiveRoomInfo& info, bool from_preset_material)
{
    area_id_ = info.channel_id;
    area_name_ = info.area_name;
    from_preset_material_ = from_preset_material;
    // 通过分区选择页调用的UpdateRoomInfo要把tag信息记录下来
    GetBililiveProcess()->secret_core()->anchor_info().set_tag_info(info.tag_id, info.tag_name);

    if (LivehimeLiveRoomController::GetInstance()->IsStreaming()) {
       GetBililiveProcess()->secret_core()->anchor_info().set_switch_tag_status(true);
    }
    else {
        GetBililiveProcess()->secret_core()->anchor_info().set_switch_tag_status(false);
    }

    auto room_id = GetBililiveProcess()->secret_core()->user_info().room_id();

    // 通知房间信息更新中
    base::NotificationService::current()->Notify(bililive::NOTIFICATION_LIVEHIME_ROOMINFO_UPDATING,
        base::NotificationService::AllSources(),
        base::NotificationService::NoDetails());
}

void UserInfoService::OnResponseUpdateRoomInfo(
    bool success, int err_code, const std::string& error_msg)
{
    if (!success || err_code != 0)
    {
        LOG(WARNING) << "Update room information failed!";

        base::string16 error;
        if (error_msg.empty())
        {
            error = res().GetLocalizedString(IDS_TOOLBAR_NET_ERROR);
        }
        else
        {
            //error = base::StringPrintf(L"%d ", err_code);
            error = base::UTF8ToUTF16(error_msg);
        }

        if (from_preset_material_)
        {
            error.clear();//这样处理，后面就不会弹框提醒了
        }

        // 通知房间信息更新失败
        base::NotificationService::current()->Notify(bililive::NOTIFICATION_LIVEHIME_ROOMINFO_UPDATE_FAIL,
            base::NotificationService::AllSources(),
            base::Details<base::string16>(&error));

        return;
    }

    auto parent_id = GetParentId(area_id_);
    area_parent_id_ = parent_id;
    GetBililiveProcess()->secret_core()->anchor_info().set_current_area(area_id_);
    GetBililiveProcess()->secret_core()->anchor_info().set_current_area_name(area_name_);
    GetBililiveProcess()->secret_core()->anchor_info().set_current_parent_area(parent_id);
    GetBililiveProcess()->secret_core()->anchor_info().set_current_parent_area_name(GetParentName(area_id_));

    // 电台分区不显示摄像头画面
    OBSProxyService::GetInstance().GetOBSCoreProxy()->SetCameraFrameHideStatus(parent_id == kRadioChannelID);

    base::NotificationService::current()->Notify(bililive::NOTIFICATION_LIVEHIME_ROOMINFO_UPDATE_SUCCESS,
        base::NotificationService::AllSources(),
        base::NotificationService::NoDetails());
}

void UserInfoService::GetAreaList() {

}

int UserInfoService::GetParentId(int id) {
    auto it = area_info_map_.find(id);

    if (it != area_info_map_.end())
    {
        return it->second.parent_id;
    }

    return 0;
}

std::string UserInfoService::GetParentName(int id) {
    auto it = area_info_map_.find(id);

    if (it != area_info_map_.end())
    {
        return it->second.parent_name;
    }
    return std::string();
}

int UserInfoService::GetCurrentAreaParentId() {
    return area_parent_id_;
}

int UserInfoService::GetCurrentAreaId() {
    return area_id_;
}

std::string UserInfoService::GetAreaNameById(int area_id)
{
	auto it = area_info_map_.find(area_id);
	if (it != area_info_map_.end())
	{
		return it->second.name;
	}
	return std::string("");
}