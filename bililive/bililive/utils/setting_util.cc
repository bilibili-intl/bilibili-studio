#include "setting_util.h"

#include <regex>

#include "base/path_service.h"
#include "base/prefs/pref_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"

#include "ui/base/resource/resource_bundle.h"
#include "grit/theme_resources.h"
#include "grit/generated_resources.h"

#include "bililive/bililive/livehime/check_processor/encoder_check_processor.h"
#include "bililive/bililive/livehime/common_pref/common_pref_service.h"
#include "bililive/bililive/livehime/function_control/app_function_controller.h"
#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/public/bililive/bililive_process.h"

#include "obs/obs_proxy/public/common/pref_constants.h"
#include "obs/obs_proxy/public/common/pref_names.h"
#include "obs/obs_proxy/public/proxy/obs_scene_collection.h"


namespace livehime
{
    base::string16 GetVideoEncoderFriendlyName(const std::string& name)
    {
        std::string raw_name = name;
        if (raw_name.empty())
        {
            raw_name = prefs::kVideoStreamX264;
        }
        ResourceBundle& rb = ResourceBundle::GetSharedInstance();
        struct
        {
            const char* key; int textId;
        } encoderList[] = {
            prefs::kVideoStreamX264, IDS_CONFIG_VIDEO_CMB_ENCODER_SOFTWARE,
            prefs::kVideoStreamNVENC, IDS_CONFIG_VIDEO_CMB_ENCODER_NVIDIA,
            prefs::kVideoStreamQSV, IDS_CONFIG_VIDEO_CMB_ENCODER_INTEL,
            prefs::kVideoStreamAMD, IDS_CONFIG_VIDEO_CMB_ENCODER_AMD,
            prefs::kVideoStreamNVTURINGENC,IDS_CONFIG_VIDEO_CMB_ENCODER_NVIDIA_TURING,
        };
        for (auto& encoder : encoderList)
        {
            if (raw_name.compare(encoder.key) == 0)
            {
                base::string16 text = rb.GetLocalizedString(encoder.textId);
                return text;
            }
        }
        return {};
    }

    base::string16 GetVideoQualityFriendlyName(int quality)
    {
        ResourceBundle& rb = ResourceBundle::GetSharedInstance();
        switch (quality)
        {
        case 1:
            return rb.GetLocalizedString(IDS_CONFIG_VIDEO_CMB_PERFORMANCE_BALANCE_1);
            break;
        case 2:
            return rb.GetLocalizedString(IDS_CONFIG_VIDEO_CMB_PERFORMANCE_BALANCE_2);
            break;
        case 3:
            return rb.GetLocalizedString(IDS_CONFIG_VIDEO_CMB_PERFORMANCE_BALANCE_3);
            break;
        default:
            break;
        }
        return {};
    }

    int CheckThirdpartyStreamEncoderType(const std::string& enc_info)
    {
        // Railgun/6.20.5 bili/6.21.0(Android)  // 最新粉上投屏
        // Railgun/1.0.84 bilibililive/5.4.3(Android)   // blink的
        // Railgun/1.0.57-apink bili/6.19.0(Android)    // 粉6.19版本之前的投屏
        // BGMLiveStreaming/1.8.3 直播姬/5.4.1(iOS)
        // BGMLiveStreaming/1.8.3 哔哩哔哩/6.19.0(iOS)
        // obs-output module (libobs version 26.0.0)
        static std::string zbj = base::WideToUTF8(L"直播姬");
        static std::string bili = base::WideToUTF8(L"哔哩哔哩");

        std::regex android_express(R"(Railgun/\d+\.\d+\.\d+[\s\S]+/\d+\.\d+\.\d+\(Android\))");
        std::regex ios_blink_express(R"(BGMLiveStreaming/\d+\.\d+\.\d+ )" + zbj + R"(/\d+\.\d+\.\d+\(iOS\))");
        std::regex ios_pink_express(R"(BGMLiveStreaming/\d+\.\d+\.\d+ )" + bili + R"(/\d+\.\d+\.\d+\(iOS\))");
        std::smatch err_mat;
        if (std::regex_search(enc_info, err_mat, android_express))
        {
            return 1;
        }
        else if (std::regex_search(enc_info, err_mat, ios_blink_express))
        {
            return 1;
        }
        else if (std::regex_search(enc_info, err_mat, ios_pink_express))
        {
            return 1;
        }

        return 0;
    }

    int AdjustBitRate(bool video, int bitrate)
    {
        enum {
            kCoreRate800 = 800,
            kCoreRate1000 = 1000,
            kCoreRate1200 = 1200,
            kCoreRate1500 = 1500,
            kCoreRate2000 = 2000,
            kCoreRate3000 = 3000,
            kCoreRate4000 = 4000,
            kCoreRate5000 = 5000,
            kCoreRate6000 = 6000,
            kCoreRate8000 = 8000,
            kCoreRate10000 = 10000,
            kCoreRate12000 = 12000,
            kCoreRate14000 = 14000,
        };

        int rate = 0;

        if (bitrate <= kCoreRate800) {
            rate = kCoreRate800;
        }
        else if (bitrate <= kCoreRate1000) {
            rate = kCoreRate1200;
        }
        else if (bitrate <= kCoreRate1500) {
            rate = kCoreRate2000;   //去除1500码率的选项，兼容老版本
        }
        else if (bitrate <= kCoreRate2000) {
            rate = kCoreRate2000;
        }
        else if (bitrate <= kCoreRate3000) {
            rate = kCoreRate3000;
        }
        else if (bitrate <= kCoreRate4000) {
            rate = kCoreRate4000;
        }
        else if (bitrate <= kCoreRate5000) {
            rate = kCoreRate6000;
        }
        else if (bitrate <= kCoreRate8000) {
            rate = kCoreRate8000;
        }
        else if (bitrate <= kCoreRate10000) {
            rate = kCoreRate10000;
        }
        //else if (bitrate <= kCoreRate12000) {
        //    rate = kCoreRate12000;
        //}
        //else if (bitrate <= kCoreRate14000) {
        //    rate = kCoreRate14000;
        //}
        else {
            rate = kCoreRate8000;
        }

        return rate;
    }


    CodecInfo::CodecInfo(const char* p_name, const char* s_name, int res_id, bool hardware)
        : pref_name(p_name)
        , sdk_name(s_name)
        , resource_id(res_id)
        , is_hardware(hardware)
    {
        ui_name = ResourceBundle::GetSharedInstance().GetLocalizedString(res_id);
    }

    std::vector<CodecInfo> GetSupportVideoEncoders()
    {
        /*static struct {
            const char* key; int textId; int flag; bool alwaysShow;
        } encoderList[] = {
            prefs::kVideoStreamX264, IDS_CONFIG_VIDEO_CMB_ENCODER_SOFTWARE, 0, true,
            prefs::kVideoStreamNVENC, IDS_CONFIG_VIDEO_CMB_ENCODER_NVIDIA, kHWEncNVSupported, false,
            prefs::kVideoStreamQSV, IDS_CONFIG_VIDEO_CMB_ENCODER_INTEL, kHWEncQSVSupported, false,
            prefs::kVideoStreamAMD, IDS_CONFIG_VIDEO_CMB_ENCODER_AMD, kHWEncAMDSupported, false,
            prefs::kVideoStreamNVTURINGENC,IDS_CONFIG_VIDEO_CMB_ENCODER_NVIDIA_TURING,kHWEncNVTURINGSupported,false
        };*/

        // 性能等级，这个等级不是说一定和实际编码性能相关，只是我们推荐用户使用的推荐等级
        enum
        {
            kSWEnc264Supported,
            kHWEncQSVSupported,
            kHWEncNVSupported,
            kHWEncAMDSupported,
            kHWEncNVTURINGSupported,
        };

        const static std::map<int, CodecInfo> all_codecs {
            {kSWEnc264Supported, {prefs::kVideoStreamX264, "obs_x264", IDS_CONFIG_VIDEO_CMB_ENCODER_SOFTWARE, false}},
            {kHWEncQSVSupported, {prefs::kVideoStreamQSV, "obs_qsv11", IDS_CONFIG_VIDEO_CMB_ENCODER_INTEL, true}},
            {kHWEncNVSupported, {prefs::kVideoStreamNVENC, "ffmpeg_nvenc", IDS_CONFIG_VIDEO_CMB_ENCODER_NVIDIA, true}},
            {kHWEncAMDSupported, {prefs::kVideoStreamAMD, "amd_amf_h264", IDS_CONFIG_VIDEO_CMB_ENCODER_AMD, true}},
            {kHWEncNVTURINGSupported, {prefs::kVideoStreamNVTURINGENC, "jim_nvenc", IDS_CONFIG_VIDEO_CMB_ENCODER_NVIDIA_TURING, true}},
        };

        static std::vector<CodecInfo> codecs;

        if (codecs.empty())
        {
            gpus_info gi = obs_get_gpus_info();
            for (const auto& iter : all_codecs)
            {
                if (OBSProxyService::GetInstance().GetOBSCoreProxy()->EncoderIsSupported(iter.second.sdk_name.c_str()) &&
                    EncoderCheckProcessor::EncoderIsSupported(iter.second.sdk_name))
                {
                    // 在显卡黑名单内的硬编编码器不要加入支持列表
                    bool in_blacklist = false;
                    std::string graphic;
                    if (AppFunctionController::GetInstance())
                    {
                        for (int i = 0; (i < gi.gpu_count) && !in_blacklist; i++)
                        {
                            auto& cbl = AppFunctionController::GetInstance()->media_settings().codec_blacklist;
                            if (cbl.find(iter.second.pref_name) != cbl.end())
                            {
                                auto& codecs = cbl.at(iter.second.pref_name);
                                auto f_iter = std::find_if(codecs.begin(), codecs.end(), [&](const std::string& codec) -> bool {
                                    return (_stricmp(codec.c_str(), gi.gpus[i].gpu_name) == 0);
                                });
                                if (f_iter != codecs.end())
                                {
                                    in_blacklist = true;
                                    graphic = *f_iter;
                                }
                            }
                        }
                    }

                    if (!in_blacklist)
                    {
                        codecs.push_back(iter.second);
                    }
                    else
                    {
                        LOG(INFO) << "[AMC] graphic hardware '" << graphic << "' not support codec '" << iter.second.sdk_name << "'.";
                    }
                }
            }

            DCHECK(!codecs.empty());
            if (codecs.empty())
            {
                LOG(WARNING) << "[AMC] this machine don't support any encoder, even x264!";
                codecs.push_back(all_codecs.at(kSWEnc264Supported));
            }
        }

        // TODO: 加个机制，在本地记录一下各个硬编的推流失败和成功的次数，从底层拿到支持的编码器列表之后，
        // 再依据本地记录的失败次数，对列表进行排序，底层支持且实际没问题的编码器优先级最高
        //////////////////////////////////////////////////////////////////////////

        return codecs;
    }

    AutoMediaConfig GetDynamicAutoMediaConfig()
    {
        AutoMediaConfig cfg;

        // 自动模式帧率固定30
        cfg.fps = 30;

        // 选择“性能较优”的硬编
        std::vector<CodecInfo> codecs = GetSupportVideoEncoders();
        DCHECK(!codecs.empty());
        if (!codecs.empty())
        {
            cfg.codec = codecs.back().pref_name;

            // 硬编下选择1080p，否则720p
            if (cfg.codec.compare("x264") == 0)
            {
                cfg.width = 1280;
                cfg.height = 720;
            }
            else
            {
                cfg.width = 1920;
                cfg.height = 1080;
            }
        }

        return cfg;
    }

    void ApplyAutoMediaConfig()
    {
        PrefService *pref = GetBililiveProcess()->profile()->GetPrefs();

        if (pref->GetBoolean(prefs::kAutoVideoSwitch))
        {
            auto cfg = GetDynamicAutoMediaConfig();

            pref->SetString(prefs::kVideoFPSCommon, std::to_string(cfg.fps));
            pref->SetString(prefs::kOutputStreamVideoEncoder, cfg.codec);

            // 1、用户已经主动的从自定义切到自动了，那么每次自动计算的视频参数都直接应用；
            // 2、由于当前推流输出分辨率output和渲染分辨率base统一采用一个设置，所以修改分辨率时必然会影响主界面的预览区的
            // 场景项布局，用户不得不重新修改场景布局，为了避免过度影响用户，定策略：
            // 单场景中源项小于等于n个时，强切分辨率，否则不要动分辨率，这个判断是一次性的，不随后续的场景项变更而重新判断；
            // 后续可以再次将渲染分辨率和输出分辨率分离，随便输出分辨率怎么变，预览场景都不会变化
            bool change_size = false;
            bool override_custom_size = false;
            if (pref->GetBoolean(prefs::kChangedCustomToAutoOnceBefore))
            {
                LOG(INFO) << "[AMC] change custom to auto once before, apply auto video size every time.";
                change_size = true;
            }
            else if (!pref->GetBoolean(prefs::kAutoVideoResolutionInited))
            {
                pref->SetBoolean(prefs::kAutoVideoResolutionInited, true);

                change_size = true;
                override_custom_size = true;

                auto pProxy = OBSProxyService::GetInstance().GetOBSCoreProxy();
                if (pProxy)
                {
                    auto collection = pProxy->GetCurrentSceneCollection();
                    if (collection)
                    {
                        std::vector<obs_proxy::Scene*> scenes = collection->GetScenes();
                        for (auto scene : scenes)
                        {
                            if (scene->GetItems().size() > 7)
                            {
                                change_size = false;
                                break;
                            }
                        }
                    }
                }

                LOG_IF(INFO, change_size) << "[AMC] override custom video size.";
            }

            if (change_size)
            {
                pref->SetInteger(prefs::kVideoOutputCX, cfg.width);
                pref->SetInteger(prefs::kVideoOutputCY, cfg.height);

                // 用自动模式计算出的分辨率来覆盖掉自定义的分辨率，
                // 这样当用户从自动切到自定义时不会再发生分辨率变更的情况（如果用户不主动去改分辨率的话）
                if (override_custom_size)
                {
                    pref->SetInteger(prefs::kCustomVideoOutputCX, cfg.width);
                    pref->SetInteger(prefs::kCustomVideoOutputCY, cfg.height);
                }
            }
            else
            {
                cfg.width = pref->GetInteger(prefs::kVideoOutputCX);
                cfg.height = pref->GetInteger(prefs::kVideoOutputCY);
            }

            LOG(INFO) << "[AMC] auto media config:\n"
                "\t video_size: " << cfg.width << "x" << cfg.height << "\n"
                "\t video_fps: " << cfg.fps << "\n"
                "\t video_codec: " << cfg.codec;
        }
    }

    void CheckProfileVideoCodec()
    {
        PrefService *pref = GetBililiveProcess()->profile()->GetPrefs();

        // 编码器过滤和强行修改只针对自定义模式，因为自动模式下每次程序启动已经在上面的ApplyAutoMediaConfig中得到过滤了
        if (!pref->GetBoolean(prefs::kAutoVideoSwitch))
        {
            std::vector<CodecInfo> codecs = GetSupportVideoEncoders();
            std::string pref_codec = pref->GetString(prefs::kOutputStreamVideoEncoder);
            auto iter = std::find_if(codecs.begin(), codecs.end(), [&](const CodecInfo& codec_info)->bool {
                return codec_info.pref_name == pref_codec;
            });
            if (iter == codecs.end())
            {
                // 默认选的是软编的话配置文件里的记录不会在这里匹配不到的，肯定是原先选的是某个硬编，
                // 然后当前这个硬编不支持了，所以这里尽量换另一个“硬编”，即从编码器数组的最后一个元素取
                pref->SetString(prefs::kOutputStreamVideoEncoder, codecs.back().pref_name);
                pref->SetString(prefs::kCustomOutputStreamVideoEncoder, codecs.back().pref_name);

                LOG(WARNING) << "[AMC] last video encoder '" << pref_codec << "' no supported this time, change to " << codecs.back().pref_name;
            }
            else
            {
                LOG(INFO) << "[AMC] custom media config:\n"
                    "\t video_codec: " << iter->pref_name;
            }
        }
    }

}