
#ifndef BILILIVE_SECRET_PUBLIC_APP_SERVICE_H_
#define BILILIVE_SECRET_PUBLIC_APP_SERVICE_H_

#include <functional>
#include <string>
#include <vector>
#include "base/files/file_path.h"
#include "base/values.h"

#include "bililive/secret/public/request_connection_proxy.h"

namespace secret {

class AppService {
public:
    enum UpdateResult : int {
        NetworkError = 0,
        DataError,
        AlreadyUpToDate,
        NewVerAvailable,
    };

    enum DynamicAudioRenderType : int {
        String = 0,
        Regex = 1,
    };

    virtual ~AppService() {}

#pragma region(获取已推送的更新信息)

    // 已推送的更新信息
    struct ReleaseUpdateInfo {
        int version_code = 0;// build号
        std::string version_name;// 友好的版本号，比如“Livehime-Win-beta-3.12.2.1548”
        std::string update_url; // 安装包的下载地址
        std::string file_size ;  // 安装包大小（字节）
        std::string file_md5; // 安装包MD5
        std::string update_desc;// 更新说明
        std::string channel;// （当前不使用这个字段）
        bool  is_force_upgrade;  //(true表示强制升级； false：不强制升级)
        bool  is_push_window;    //(true表示推送窗口； false：不推送窗口(就是端上不更新))


        std::string publish_list_md5; // 更新项列表记录文件MD5
        std::string publish_list_url; // 更新项列表记录文件的下载地址
        int64 publish_list_size = 0;  // 更新项列表记录文件大小（字节）
    };

    using CheckForReleaseUpdateHandler = std::function<void(UpdateResult result, const ReleaseUpdateInfo& update_info)>;

    // 获取已推送的更新信息
    virtual RequestProxy CheckForReleaseUpdate(int64_t mid,CheckForReleaseUpdateHandler handler) = 0;

#pragma endregion

#pragma region(获取正在灰度（未推送）的信息)

    // 当前灰度的更新信息
    struct BetaUpdateInfo {
        std::string version;// 友好的版本号，比如“Livehime-Win-beta-3.12.2.1548”
        int inner_ver_num = 0;// build号
        std::string dl_url; // 安装包的下载地址
        std::string ver_size ; // 安装包大小（字节）
        std::string ver_md5; // 安装包MD5
        std::string ver_desc;// 更新说明

        std::string publish_list_md5; // 更新项列表记录文件MD5
        std::string publish_list_url; // 更新项列表记录文件的下载地址
        int64 publish_list_size = 0;  // 更新项列表记录文件大小（字节）
    };

    using CheckForBetaUpdateHandler = std::function<void(UpdateResult result, const BetaUpdateInfo& info)>;

    // 获取正在灰度（未推送）的信息
    virtual RequestProxy CheckForBetaUpdate(CheckForBetaUpdateHandler handler) = 0;

#pragma endregion

#pragma region(获取推送更新的方式)

    // 当前推送更新的方式，是强制更新还是可选更新（截止目前我们都只用强制更新）
    struct UpgradeInfo {
        std::string force_upgrade;  // 1表示强制升级； 0：不强制升级
        std::string version;    // 友好的版本号，比如“Livehime-Win-beta-3.12.2.1548”, force_upgrade为0时为空字符串
        std::string inner_ver_num;  // build号，force_upgrade为0时返回0
        std::string ver_desc;   // 更新说明，force_upgrade为0时返回空字符串
    };

    using UpgradeCheckHandler = std::function<void(bool valid, int code, const UpgradeInfo& info)>;

    // 获取推送更新的方式（用以判断已推送的更新信息是需要强制更新的还是允许用户选择更新与否的）
    virtual RequestProxy UpgradeCheck(UpgradeCheckHandler handler) = 0;

#pragma endregion

#pragma region(获取新版本的文件列表，用于增量更新机制)

    using GetPublishListInfoHandler = std::function<void(bool valid, const std::string& md5, int64 size)>;
    // 获取新版本文件列表信息（充当更新接口，服务端没人力修改更新接口的替代方案）
    virtual RequestProxy GetPublishListInfo(const std::string& url, GetPublishListInfoHandler handler) = 0;

#pragma endregion

#pragma region(上传崩溃文件到服务器)
    struct UploadInfo {
        std::string url;
    };

    using DumpUploadedHandler = std::function<void(bool valid, int code, const UploadInfo& info)>;

    virtual RequestProxy UploadDumpFile(const std::string& content, DumpUploadedHandler handler) = 0;
#pragma endregion

#pragma region(下载动态配置信息)
    struct DynamicInfo {
        int file_size = 0;
        std::string save_path;
        std::string download_url;
        std::string file_md5;
        std::string save_type;
        std::string task_type;
        std::string dynamic_type;
        std::shared_ptr<base::DictionaryValue> dict_value;
    };

    using GetDynamicInfoHandler = std::function<void(
        bool success, const DynamicInfo& info)>;

    virtual RequestProxy GetDynamicInfo(const std::string& url,
        GetDynamicInfoHandler handler) = 0;
#pragma endregion


};

}   // namespace secret

#endif  // BILILIVE_SECRET_PUBLIC_APP_SERVICE_H_
