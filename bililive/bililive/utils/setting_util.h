#ifndef BILILIVE_BILILIVE_UTILS_SETTING_UTIL_H
#define BILILIVE_BILILIVE_UTILS_SETTING_UTIL_H

#include "base/strings/string16.h"
#include "base/strings/stringprintf.h"


namespace livehime
{
    // 传入配置文件里面实际记录的编码器标识，返回UI界面上展示的友好的对应编码器名称
    // 比如配置文件里记录的是“nv_turing_enc”，其对应的友好名称是“英伟达图灵编码”
    base::string16 GetVideoEncoderFriendlyName(const std::string& name);

    // 返回配置文件里记录的编码质量(1~3），返回友好的名称（均衡、高性能、高质量等）
    base::string16 GetVideoQualityFriendlyName(int quality);

    // 判断编码器名称是不是我站自身的产品
    int CheckThirdpartyStreamEncoderType(const std::string& enc_info);

    // 将码率按照固定档位进行取整
    int AdjustBitRate(bool video, int bitrate);

    // 获取OutputController所支持的编码器列表，返回的编码器名称对应的是配置文件里面的名称
    struct CodecInfo
    {
        CodecInfo() = default;
        CodecInfo(const char* pref_name, const char* sdk_name, int resource_id, bool hardware);

        std::string pref_name;  // 编码器对应的配置文件中记录的名称
        std::string sdk_name;   // 编码器在底层的名称
        int resource_id = 0;        // 编码器在UI进行友好显示的
        base::string16 ui_name; //编码器在UI进行友好显示的名称
        bool is_hardware = false; //是否是硬件编码器
    };
    std::vector<CodecInfo> GetSupportVideoEncoders();

    // 动态获取自动音视频参数配置
    struct AutoMediaConfig
    {
        int fps;
        int width;
        int height;
        std::string codec;
    };
    AutoMediaConfig GetDynamicAutoMediaConfig();
    void ApplyAutoMediaConfig();

    // 程序每次启动都对当前配置里面的编码器进行判断，不支持编码器但配置文件里面还记录有的，强行修改配置文件记录
    void CheckProfileVideoCodec();
}

#endif