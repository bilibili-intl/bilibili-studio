
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

#pragma region(��ȡ�����͵ĸ�����Ϣ)

    // �����͵ĸ�����Ϣ
    struct ReleaseUpdateInfo {
        int version_code = 0;// build��
        std::string version_name;// �Ѻõİ汾�ţ����硰Livehime-Win-beta-3.12.2.1548��
        std::string update_url; // ��װ�������ص�ַ
        std::string file_size ;  // ��װ����С���ֽڣ�
        std::string file_md5; // ��װ��MD5
        std::string update_desc;// ����˵��
        std::string channel;// ����ǰ��ʹ������ֶΣ�
        bool  is_force_upgrade;  //(true��ʾǿ�������� false����ǿ������)
        bool  is_push_window;    //(true��ʾ���ʹ��ڣ� false�������ʹ���(���Ƕ��ϲ�����))


        std::string publish_list_md5; // �������б��¼�ļ�MD5
        std::string publish_list_url; // �������б��¼�ļ������ص�ַ
        int64 publish_list_size = 0;  // �������б��¼�ļ���С���ֽڣ�
    };

    using CheckForReleaseUpdateHandler = std::function<void(UpdateResult result, const ReleaseUpdateInfo& update_info)>;

    // ��ȡ�����͵ĸ�����Ϣ
    virtual RequestProxy CheckForReleaseUpdate(int64_t mid,CheckForReleaseUpdateHandler handler) = 0;

#pragma endregion

#pragma region(��ȡ���ڻҶȣ�δ���ͣ�����Ϣ)

    // ��ǰ�Ҷȵĸ�����Ϣ
    struct BetaUpdateInfo {
        std::string version;// �Ѻõİ汾�ţ����硰Livehime-Win-beta-3.12.2.1548��
        int inner_ver_num = 0;// build��
        std::string dl_url; // ��װ�������ص�ַ
        std::string ver_size ; // ��װ����С���ֽڣ�
        std::string ver_md5; // ��װ��MD5
        std::string ver_desc;// ����˵��

        std::string publish_list_md5; // �������б��¼�ļ�MD5
        std::string publish_list_url; // �������б��¼�ļ������ص�ַ
        int64 publish_list_size = 0;  // �������б��¼�ļ���С���ֽڣ�
    };

    using CheckForBetaUpdateHandler = std::function<void(UpdateResult result, const BetaUpdateInfo& info)>;

    // ��ȡ���ڻҶȣ�δ���ͣ�����Ϣ
    virtual RequestProxy CheckForBetaUpdate(CheckForBetaUpdateHandler handler) = 0;

#pragma endregion

#pragma region(��ȡ���͸��µķ�ʽ)

    // ��ǰ���͸��µķ�ʽ����ǿ�Ƹ��»��ǿ�ѡ���£���ֹĿǰ���Ƕ�ֻ��ǿ�Ƹ��£�
    struct UpgradeInfo {
        std::string force_upgrade;  // 1��ʾǿ�������� 0����ǿ������
        std::string version;    // �Ѻõİ汾�ţ����硰Livehime-Win-beta-3.12.2.1548��, force_upgradeΪ0ʱΪ���ַ���
        std::string inner_ver_num;  // build�ţ�force_upgradeΪ0ʱ����0
        std::string ver_desc;   // ����˵����force_upgradeΪ0ʱ���ؿ��ַ���
    };

    using UpgradeCheckHandler = std::function<void(bool valid, int code, const UpgradeInfo& info)>;

    // ��ȡ���͸��µķ�ʽ�������ж������͵ĸ�����Ϣ����Ҫǿ�Ƹ��µĻ��������û�ѡ��������ģ�
    virtual RequestProxy UpgradeCheck(UpgradeCheckHandler handler) = 0;

#pragma endregion

#pragma region(��ȡ�°汾���ļ��б������������»���)

    using GetPublishListInfoHandler = std::function<void(bool valid, const std::string& md5, int64 size)>;
    // ��ȡ�°汾�ļ��б���Ϣ���䵱���½ӿڣ������û�����޸ĸ��½ӿڵ����������
    virtual RequestProxy GetPublishListInfo(const std::string& url, GetPublishListInfoHandler handler) = 0;

#pragma endregion

#pragma region(�ϴ������ļ���������)
    struct UploadInfo {
        std::string url;
    };

    using DumpUploadedHandler = std::function<void(bool valid, int code, const UploadInfo& info)>;

    virtual RequestProxy UploadDumpFile(const std::string& content, DumpUploadedHandler handler) = 0;
#pragma endregion

#pragma region(���ض�̬������Ϣ)
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
