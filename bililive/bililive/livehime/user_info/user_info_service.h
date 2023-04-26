#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_USER_INFO_USER_INFO_SERVICE_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_USER_INFO_USER_INFO_SERVICE_H_

#include "base/basictypes.h"
#include "base/memory/weak_ptr.h"

#include "bililive/public/secret/bililive_secret.h"

extern const int kVirtualLiveChannelID;
extern const int kRadioLiveChannelID;

class UserInfoService {
public:
    UserInfoService();
    virtual ~UserInfoService();

    struct CreateRoomInfo {
        int code = -1;
        int64_t  room_id = 0;
        std::string err_msg = "";
        bool valid_response = false;
    };

    struct LiveRoomInfo {
        static const int kIDNoChannel = -1;

        std::string title;
        int channel_id = kIDNoChannel;
        std::string area_name;
        int64 tag_id = -1;
        std::string tag_name;

        LiveRoomInfo() = default;

        LiveRoomInfo(const std::string& title,
            int channel_id,
            const std::string& area_name,
            int64 tag_id,
            const std::string& tag_name)
            : title(title)
            , channel_id(channel_id)
            , area_name(area_name)
            , tag_id(tag_id)
            , tag_name(tag_name)
        {}

        explicit operator bool() const
        {
            return !title.empty();
        }
    };

	struct AreaInfo {
		int id;
		int parent_id;
		std::string name;
        std::string parent_name;
	};

public:
    void GetUserInfo();

    void GetUserSan();

    void CreateLiveRoom(int from_type);

    void GetRoomInfo();

    void UpdateLiveRoomInfo(const LiveRoomInfo& info,bool from_preset_material = false);

    void GetAreaList();

    int GetParentId(int id);

    std::string GetParentName(int id);

    int GetCurrentAreaId();

    int GetCurrentAreaParentId();

    std::string GetAreaNameById(int area_id);

private:
    base::FilePath GetAvatarPath();

    void OnGetViddupUserInfo(bool valid_response, int code,
        const secret::UserAccountService::ViddupUserInfoData& info);

    void GetUserAvatar(const std::string& url);

    void OnGetUserAvatar(bool success,
        const secret::UserAccountService::UserAvatarInfo& avatar_info);

    void OnGetUserSan(bool successed, int64_t san);

    void OnCreateLiveRoom(
        bool valid_response, int code, const std::string& err_msg, int64_t roomid);

    void OnGetIntlRoomInfo(bool valid_response, int code, const std::string& err_msg,
        const secret::LiveStreamingService::IntlRoomInfo& info);

    void OnOpenLiveSteam(bool valid_response, int code, const std::string& err_msg,
        const secret::LiveStreamingService::IntlRoomInfo& info);

    void OnResponseUpdateRoomInfo(bool success, int err_code, const std::string& error_msg);

private:
    int area_id_ = -1;
    int area_parent_id_ = -1;
    std::string area_name_;
    std::map<int, AreaInfo> area_info_map_;

    bool liveroom_creating_ = false;
    int liveroom_create_type_ = -1;

    bool from_preset_material_ = false;//是否从预设素材请求的更改默认分区，为true时请求失败时不提示

    base::WeakPtrFactory<UserInfoService> weak_ptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(UserInfoService);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_USER_INFO_USER_INFO_SERVICE_H_