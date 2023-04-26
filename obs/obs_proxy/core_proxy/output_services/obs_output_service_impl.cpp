/*
 @ 0xCCCCCCCC
*/

#include "obs/obs_proxy/core_proxy/output_services/obs_output_service_impl.h"

#include <regex>
#include <atomic>
#include <algorithm>
#include <deque>
#include <map>

#include "base/ext/bind_lambda.h"
#include "base/file_util.h"
#include "base/strings/string_piece.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_util.h"
#include "base/strings/sys_string_conversions.h"
#include "base/threading/thread_restrictions.h"
#include "base/time/time.h"

#include "bilibase/error_exception_util.h"
#include "bilibase/scope_guard.h"

#include "bililive/bililive/livehime/streaming_report/streaming_report_service.h"

#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/public/bililive/livehime_obs_frame_extra.h"

#include "obs/obs_proxy/app/obs_proxy_access_stub.h"
#include "obs/obs_proxy/common/obs_proxy_constants.h"
#include "obs/obs_proxy/core_proxy/core_proxy_impl.h"
#include "obs/obs_proxy/public/common/pref_constants.h"
#include "obs/obs_proxy/public/common/pref_names.h"
#include "obs/obs_proxy/utils/audio_encoders.h"

#include "ui/base/resource/resource_bundle.h"

#include "grit/generated_resources.h"

#include "obs/obs-studio/libobs/graphics/matrix4.h"
#include "obs/obs_proxy/live_mask/avc/avc-sei-util.h"
#include "obs/obs_proxy/live_mask/live-mask.h"
#include "obs/obs_proxy/live_mask/lib/nal-filter.hpp"

namespace {

// OBS service settings constants.
const char kKeyAccessKey[] = "key";
const char kKeyServerProtocol[] = "protocol";
const char kKeyServerAddr[] = "server";
const char kKeyServerUseAuth[] = "use_auth";
const char kDefaultAccessKey[] = "None";

// Encoder settings constants.
const char kKeyCRF[] = "crf";
const char kKeyBitRate[] = "bitrate";
const char kKeyUseBufSize[] = "use_bufsize";
const char kKeyRateControl[] = "rate_control";
const char kKeyProfile[] = "profile";
const char kKeyPreset[] = "preset";
const char kKeyQPI[] = "qpi";
const char kKeyQPP[] = "qpp";
const char kKeyQPB[] = "qpb";
const char kKeyCQP[] = "cqp";
const char kRateCRF[] = "CRF";
const char kRateCQP[] = "CQP";
const char kRateCBR[] = "CBR";
const char kProfileHigh[] = "high";
const char kProfileHQ[] = "hq";
const char kUltraFast[] = "ultrafast";
const char kVeryFast[] = "veryfast";
const char kKeyKeyFrameInterval[] = "keyint_sec";

const char kRateX264ABR[] = "ABR";
const char kRateNVENCABR[] = "vbr";
const char kRateQSVABR[] = "AVBR";
const char kRateAMDABR[] = "VBR";
const char kRateNVTURINGENCABR[] = "VBR";

const char kDNSCallback[] = "dns_hook_func";
const char kDNSCallbackUserData[] = "dns_hook_userdata";

const char kSRTMaxBandwidth[] = "srt_max_bandwidth";

const int kPresetKeyFrameInterval = 3;  // in seconds.
const uint64_t kFeedbackRefreshInterval = 2;    // in seconds.


std::wstring LocalStr(int message_id)
{
    return ResourceBundle::GetSharedInstance().GetLocalizedString(message_id);
}


#pragma region(Helper class)

class OBSOutputHelper : public std::enable_shared_from_this<OBSOutputHelper>
{
public:
    using Ptr = std::shared_ptr<OBSOutputHelper>;
    using CallbackT = std::function<void(calldata_t * param)>;

private:
    using string_ptr = std::unique_ptr<std::string>;
    std::map<std::string, string_ptr> string_container_;

    const char* asLPCSTR(const std::string& str)
    {
        auto it = string_container_.find(str);
        if (it != string_container_.end())
            return it->second->c_str();
        else
        {
            auto& v = string_container_[str];
            v.reset(new std::string(str));
            return v->c_str();
        }
    }

    struct SignalPair
    {
        OBSSignal obsSignal;
        CallbackT handler;

        using Ptr = std::unique_ptr<SignalPair>;
    };

    OBSOutput output_;
    std::map<std::string, SignalPair::Ptr> signals_;

public:
    ~OBSOutputHelper()
    {
        DisconnectSignals();
    }

    static Ptr Create(const std::string& output_id, const std::string& output_name, obs_data* settings)
    {
        auto output = obs_output_create(output_id.c_str(), output_name.c_str(), settings, nullptr);
        if (!output)
            return {};

        auto result = std::make_shared<OBSOutputHelper>();
        result->output_ = output;

        obs_output_release(output);

        return result;
    }

    template<class T>
    void ConnectSignal(const std::string& signalName, T&& handler)
    {
        auto thiswptr = weak_from_this();
        auto signal_handler = obs_output_get_signal_handler(output_);
        auto& pair = signals_[signalName];
        pair.reset(new SignalPair());
        pair->handler = [handler{std::move(handler)}, thiswptr](calldata_t* param) {
            if (thiswptr.lock())
                handler(param);
        };
        pair->obsSignal.Connect(signal_handler, asLPCSTR(signalName),
            [](void* userdata, calldata_t* param) {
                auto pair = (SignalPair*)userdata;
                pair->handler(param);
            }, pair.get());
    }

    void DisconnectSignal(const std::string& signalName)
    {
        auto it = signals_.find(signalName);
        if (it != signals_.end())
            signals_.erase(it);
    }

    void DisconnectSignals()
    {
        signals_.clear();
    }

    operator obs_output* () const
    {
        return output_;
    }
};

#pragma endregion


#pragma region(Helper free functions)

std::string GetConfiguredStreamingVideoEncoder(const PrefService* pref_service)
{
    // Use x264 by default, as it is the default config.
    std::string encoder_id = obs_proxy::kVideoEncoderX264ID;
    std::string encoder_name = pref_service->GetString(prefs::kOutputStreamVideoEncoder);

    if (prefs::kVideoStreamQSV == encoder_name) {
        encoder_id = obs_proxy::kVideoEncoderQSVID;
    } else if (prefs::kVideoStreamNVENC == encoder_name) {
        encoder_id = obs_proxy::kVideoEncoderNVENCID;
    }else if (prefs::kVideoStreamAMD == encoder_name){
        encoder_id = obs_proxy::kVideoEncoderAMFID;
    }else if (prefs::kVideoStreamNVTURINGENC == encoder_name){
        encoder_id = obs_proxy::kVideoEncoderNVTURINGENCID;
    }

    return encoder_id;
}

// Loads video encoder for either streaming or recording.
void LoadVideoEncoder(OBSEncoder& encoder, OBSSignal& bound_signal, base::StringPiece encoder_id,
                      base::StringPiece name)
{
    bound_signal.Disconnect();

    encoder = obs_video_encoder_create(encoder_id.data(), name.data(), nullptr, nullptr);
    ENSURE(encoder != nullptr)(encoder_id)(name).Require("Failed to create video encoder");
    obs_encoder_release(encoder);
}

// Loads audio encoder for either streaming or recording.
// current audio encoder id would be updated, once the loading succeeded.
void LoadAudioEncoder(OBSEncoder& encoder, std::string& current_encoder_id, base::StringPiece name,
                      int bitrate)
{
    std::string encoder_id = obs_proxy::GetEncoderIDForBitrate(bitrate);
    ENSURE(encoder_id.empty() != true)(bitrate).Require();

    // Don't update in this case.
    if (current_encoder_id == encoder_id) {
        return;
    }

    encoder = obs_audio_encoder_create(encoder_id.c_str(), name.data(), nullptr, 0, nullptr);
    ENSURE(encoder != nullptr)(encoder_id)(name).Require("Failed to create audio encoder");
    obs_encoder_release(encoder);
    current_encoder_id = encoder_id;
}

std::string GetFormatExtension(const std::string& format)
{
    if (format == prefs::kRecordingFormatFLV)
        return "flv";
    else if (format == prefs::kRecordingFormatMP4)
        return "mp4";
    else if (format == prefs::kRecordingFormatFMP4)
        return "mp4";
    else
        return format;
}

bool ConfigRecOutputForFormat(obs_data* settings, const std::string& format)
{
    if (!settings)
        return false;

    std::string extension = GetFormatExtension(format);
    if (format == prefs::kRecordingFormatFLV)
    {
        obs_data_set_string(settings, "format_name", extension.c_str());
        obs_data_set_string(settings, "muxer_settings", "");
        return true;
    }
    else if (format == prefs::kRecordingFormatMP4)
    {
        obs_data_set_string(settings, "format_name", extension.c_str());
        obs_data_set_string(settings, "muxer_settings", "");
        return true;
    }
    else if (format == prefs::kRecordingFormatFMP4)
    {
        obs_data_set_string(settings, "format_name", extension.c_str());
        obs_data_set_string(settings, "muxer_settings", "movflags=+frag_keyframe+faststart+empty_moov");
        return true;
    }
    else
        return false;
}

// 为了解决排序问题,录制文件名称精确到秒
base::FilePath GenerateOutputFilePath(const PrefService* pref_service)
{
    auto path = pref_service->GetFilePath(prefs::kOutputFolder);
    auto current_time = base::Time::Now();
    base::Time::Exploded dimensions;
    current_time.LocalExplode(&dimensions);
    std::string filename = base::StringPrintf("%d%02d%02d-%02d%02d%02d",
                                              dimensions.year,
                                              dimensions.month,
                                              dimensions.day_of_month,
                                              dimensions.hour,
                                              dimensions.minute,
                                              dimensions.second);
  //                                            dimensions.millisecond);

    std::string format = pref_service->GetString(prefs::kOutputRecordingFormat);
    path = path.AppendASCII(filename).AddExtension(base::SysNativeMBToWide(GetFormatExtension(format)));

    return path;
}

bool EnsureDirectoryExists(const base::FilePath& dir_path)
{
    if (base::DirectoryExists(dir_path)) {
        return true;
    }

    auto parent_dir = dir_path.DirName();
    if (parent_dir == dir_path) {
        return false;
    }

    if (EnsureDirectoryExists(parent_dir) && base::PathIsWritable(parent_dir)) {
        return file_util::CreateDirectory(dir_path);
    }

    return false;
}

int CalculateCRF(int base, bool use_low_cpu_x264)
{
    const double kCrossDistCutOff = 2000.0;

    double cx = obs_proxy::GetPrefs()->GetInteger(prefs::kVideoOutputCX);
    double cy = obs_proxy::GetPrefs()->GetInteger(prefs::kVideoOutputCY);

    if (use_low_cpu_x264)
    {
        base -=2;
    }

    double cross_dist = sqrt(cx * cx + cy * cy);
    double crf_res_reduction = fmin(kCrossDistCutOff, cross_dist) / kCrossDistCutOff;
    crf_res_reduction = (1.0 - crf_res_reduction) * 10.;

    return base - static_cast<int>(crf_res_reduction);
}

void SetupOutputAV(obs_output* output)
{
    if (!output)
        return;

    obs_output_set_media(output, obs_get_video(), obs_get_audio());

    auto video_enc = obs_output_get_video_encoder(output);
    if (video_enc)
    {
        if (!obs_encoder_active(video_enc))
        {
            obs_encoder_set_video(video_enc, obs_get_video());
        }
    }
    int idx = 0;
    for (;;)
    {
        auto audio_enc = obs_output_get_audio_encoder(output, idx);
        if (!audio_enc)
            break;
        if (!obs_encoder_active(audio_enc))
        {
            obs_encoder_set_audio(audio_enc, obs_get_audio());
        }
        ++idx;
    }
}

#pragma endregion


#pragma region 弹幕蒙板相关

constexpr int LIVEMASK_TIMEOUT = 500000000LL; // 0.5s

using bililive::OBSFrameExtra;
using bililive::OBSFrameExtraPtr;

template<class MapContainer>
typename MapContainer::const_iterator search_live_mask(uint64_t frame_time, const MapContainer& container)
{
    auto it1 = container.lower_bound(frame_time);
    if (it1 != container.end() && it1 != container.begin() && it1->first > (uint64_t)frame_time)
        --it1;
    else if (it1 == container.end() && it1 != container.begin())
        --it1;
    return it1;
}

// 管理某个 source 和它关联的 mask 的类
class SourceLiveMaskHelper
{
    std::mutex mutex_;
    std::map<uint64_t, OBSFrameExtraPtr> ts_to_extra_;

public:
    bool Tick(uint64_t frame_time, obs_source_t* source)
    {
        if (!source)
            return false;

        std::unique_lock<std::mutex> lg(mutex_);

        auto frame = obs_source_peek_frame(source);
        if (!frame)
            return false;

        if (!frame->extra_data || !frame->extra_data_op)
            return false;

        auto extra_rawptr = frame->extra_data_op->as_type(OBS_FRAME_EXTRA_BILILIVE, frame->extra_data);
        if (!extra_rawptr)
            return false;

        ts_to_extra_[frame_time] = static_cast<OBSFrameExtra*>(extra_rawptr);

        // fallback: it should be not too large
        if (ts_to_extra_.size() > 500)
        {
            assert(false);
            auto to_remove_count = ts_to_extra_.size() - 200;
            auto itend = ts_to_extra_.begin();
            while (to_remove_count-- > 0)
                ++itend;
            ts_to_extra_.erase(ts_to_extra_.begin(), itend);
        }

        return true;
    }

    OBSFrameExtraPtr GetLiveMask(uint64_t frame_time, uint64_t threshold)
    {
        std::unique_lock<std::mutex> lg(mutex_);

        auto it = search_live_mask(frame_time, ts_to_extra_);

        // 往前找直到在范围内找到完成的
        while (it != ts_to_extra_.cend() && frame_time - it->first < threshold) {
            if (it->second->live_mask().done_flag)
                return it->second;
            if (it == ts_to_extra_.cbegin())
                return {};
            --it;
        }
        return {};
    }

    // return: true=已经清空了
    bool Clean(uint64_t deadline_frame_time)
    {
        std::unique_lock<std::mutex> lg(mutex_);

        auto it = ts_to_extra_.lower_bound(deadline_frame_time);
        ts_to_extra_.erase(ts_to_extra_.begin(), it);

        return ts_to_extra_.empty();
    }
};
using SourceLiveMaskHelperPtr = std::shared_ptr<SourceLiveMaskHelper>;


// 管理多个 source 的蒙板信息
class SourceLiveMaskManager
{
    std::mutex mutex_;
    std::map<intptr_t, SourceLiveMaskHelperPtr> source_extra_set_;

public:
    SourceLiveMaskManager()
    {
    }

    bool Tick(uint64_t frame_time, obs_source_t* src)
    {
        std::unique_lock<std::mutex> lg(mutex_);
        auto srcptr = (intptr_t)src;
        auto& src_extra = source_extra_set_[srcptr];
        if (!src_extra)
            src_extra.reset(new SourceLiveMaskHelper());
        return src_extra->Tick(frame_time, src);
    }

    SourceLiveMaskHelperPtr GetHelper(obs_source_t* src)
    {
        std::unique_lock<std::mutex> lg(mutex_);
        auto srcptr = (intptr_t)src;
        auto it = source_extra_set_.find(srcptr);
        if (it == source_extra_set_.end())
            return nullptr;
        return it->second;
    }

    void Clean(uint64_t deadline_frame_time)
    {
        auto it = source_extra_set_.begin();
        while (it != source_extra_set_.end())
        {
            if (it->second->Clean(deadline_frame_time))
                it = source_extra_set_.erase(it);
            else
                ++it;
        }
    }
};


// 表示一个场景元素的蒙板信息
struct SceneItemMaskData
{
    obs_sceneitem_crop crop_info;
    uint32_t src_width, src_height;
    matrix4 draw_transform_matrix;
    SourceLiveMaskHelperPtr source_livemask_helper;

    SceneItemMaskData()
        : src_width(0), src_height(0)
    {
    }

    ~SceneItemMaskData()
    {
    }
};


// 表示一个场景帧的蒙板信息
struct SceneMaskData
{
    int64_t frame_time;
    uint32_t scene_width, scene_height;
    std::vector<SceneItemMaskData> itemMaskDatas;
};

// 遍历场景元素时收集信息用的上下文
struct EnumItemCtx
{
    SceneMaskData& sceneMask;
    SourceLiveMaskManager& livemaskManager;
    bool hasMask;

    EnumItemCtx(SceneMaskData& sm, SourceLiveMaskManager& lm)
        : sceneMask(sm)
        , livemaskManager(lm)
        , hasMask(false)
    {
    }
};


// 弹幕蒙版处理对象
// 在绘制时回调、收集所有 dshow 源的蒙版数据
// 为 encoder 提供 packet filter（）添加蒙板
class LiveMask : public boost::intrusive_ref_counter<LiveMask>
{
    std::mutex mutex_;

    struct encoder_packet last_pkt_;
    std::vector<uint8_t> last_pkt_data_;

    std::map<uint64_t, SceneMaskData> frametime_to_scene_maskdata_;
    SourceLiveMaskManager source_livemask_set_;

public:
    LiveMask()
    {
    }

    ~LiveMask()
    {
    }

    LiveMask(const LiveMask&) = delete;
    const LiveMask& operator = (const LiveMask&) = delete;

    // called after obs's tick and before render
    void TickLiveMask()
    {
        // 获取每个异步源当前帧的蒙板数据
        std::unique_lock<std::mutex> lg(mutex_);

        auto out_src = obs_get_output_source(0);
        if (!out_src)
            return;

        auto out_id = obs_source_get_id(out_src);
        auto out_type = obs_source_get_type(out_src);
        if (out_type == OBS_SOURCE_TYPE_TRANSITION)
        {
            auto new_src = obs_transition_get_active_source(out_src);
            obs_source_release(out_src);
            out_src = new_src;
        }
        if (!out_src)
            return;
        auto out_scene = obs_scene_from_source(out_src);
        if (out_scene)
        {
            auto frame_time = obs_get_video_frame_time();
            SceneMaskData scene_mask_data {};
            EnumItemCtx mask_data(scene_mask_data, source_livemask_set_);
            mask_data.sceneMask.frame_time = frame_time;
            mask_data.sceneMask.scene_width = obs_source_get_width(out_src);
            mask_data.sceneMask.scene_height = obs_source_get_height(out_src);

            obs_scene_enum_items(out_scene, [](obs_scene_t* scene, obs_sceneitem_t* item, void* userdata)->bool
            {
                EnumItemCtx& ctx = *static_cast<EnumItemCtx*>(userdata);

                if (!obs_sceneitem_visible(item))
                    return true;

                auto src = obs_sceneitem_get_source(item);
                if (!src)
                    return true;

                if (!ctx.livemaskManager.Tick(ctx.sceneMask.frame_time, src))
                    return true;

                ctx.hasMask = true;
                auto src_width = obs_source_get_width(src);
                auto src_height = obs_source_get_height(src);
                obs_sceneitem_crop crop;
                obs_sceneitem_get_crop(item, &crop);
                if (crop.left + crop.right == (int)src_width || crop.top + crop.bottom == (int)src_height)
                    return true;

                ctx.sceneMask.itemMaskDatas.emplace_back();
                auto& itemMask = ctx.sceneMask.itemMaskDatas.back();
                itemMask.crop_info = crop;
                itemMask.src_width = src_width;
                itemMask.src_height = src_height;
                obs_sceneitem_get_draw_transform(item, &itemMask.draw_transform_matrix);
                itemMask.source_livemask_helper = ctx.livemaskManager.GetHelper(src);
                return true;
            }, &mask_data);

            if (mask_data.hasMask) {
                std::swap(frametime_to_scene_maskdata_[frame_time], scene_mask_data);
            }
        }

        obs_source_release(out_src);
    }

    struct encoder_packet* Filter(obs_encoder_t* encoder, struct encoder_packet* pkt)
    {
        std::unique_lock<std::mutex> lg(mutex_);
        last_pkt_ = *pkt;

        // 清理老数据
        // 对场景帧蒙板来说，因为不会一帧用多次所以过期就不用了
        // 对源蒙板数据来说，因为有帧率不匹配问题所以要考虑一帧用多次
        auto dts = obs_encoder_pkt_ts_to_frame_ts(encoder, pkt->dts);
        frametime_to_scene_maskdata_.erase(
            frametime_to_scene_maskdata_.begin(),
            frametime_to_scene_maskdata_.lower_bound(dts - 1)
        );
        source_livemask_set_.Clean(dts - LIVEMASK_TIMEOUT);

        // === 开始生成当前帧蒙板
        auto frame_time = obs_encoder_pkt_ts_to_frame_ts(encoder, pkt->pts);
        if (frame_time <= 0)
            return pkt;

        // 找不大于 pts 的、最近一个
        auto it = search_live_mask(frame_time, frametime_to_scene_maskdata_);
        if (it == frametime_to_scene_maskdata_.end())
            return pkt;

        auto& scene = it->second;
        if (scene.itemMaskDatas.empty())
            return pkt;

        // 创建蒙板svg
        std::unique_ptr<AlphaToSvgBin> tracer(CreatePotraceAlphaToSvgBin(scene.scene_width, scene.scene_height));
        for (auto& item : scene.itemMaskDatas)
        {
            auto extra = item.source_livemask_helper->GetLiveMask(scene.frame_time, 500000000LL);
            if (!extra)
                continue;

            auto& mask = extra->live_mask();

            matrix4 trans;

            // mask缩放到source大小的变换
            matrix4_identity(&trans);
            matrix4_scale3f(&trans, &trans,
                1.0f * item.src_width / mask.width,
                1.0f * item.src_height / mask.height,
                1.0f
            );

            // mask数据切边
            size_t mask_data_offset = 0;
            auto mask_width = mask.width;
            auto mask_height = mask.height;
            {
                obs_sceneitem_crop crop = item.crop_info;
                size_t left_offset = crop.left * mask.width / item.src_width;
                size_t top_offset = crop.top * mask.height / item.src_height;
                size_t new_width = (item.src_width - crop.left - crop.right) * mask.width / item.src_width;
                size_t new_height = (item.src_height - crop.top - crop.bottom) * mask.height / item.src_height;
                mask_width = std::min<size_t>(mask.width - left_offset, new_width);
                mask_height = std::min<size_t>(mask.height - top_offset, new_height);
                if (new_width <= 0 || new_height <= 0)
                    continue;

                if (left_offset)
                    mask_data_offset += left_offset;
                if (top_offset)
                    mask_data_offset += top_offset * mask.stride;
            }

            // 对mask做sceneitem变换
            matrix4_mul(&trans, &trans, &item.draw_transform_matrix);

            tracer->AppendMask(mask.mask_data.data() + mask_data_offset, 1, mask_width, mask_height, mask.stride,
                (float(*)[4])&trans);
        }
        auto sei = tracer->GetSEI(true);
        // 往 packet 里塞入蒙板 sei
        NalFilter nf(pkt->data, pkt->size);
        nf.DropAUD();
        nf.InsertBeforeVCLNal(sei.data(), sei.size());
        last_pkt_data_ = nf.GetResult();
        last_pkt_.data = last_pkt_data_.data();
        last_pkt_.size = last_pkt_data_.size();
        return &last_pkt_;
    }

    void Clear()
    {
        std::unique_lock<std::mutex> lg(mutex_);
        frametime_to_scene_maskdata_.clear();
    }
};
using LiveMaskPtr = boost::intrusive_ptr<LiveMask>;


static struct LiveMaskInfo : public encoder_packet_filter_info
{
    LiveMaskInfo()
    {
        release = [](void* ctx)
        {
            LiveMaskPtr(static_cast<LiveMask*>(ctx), false).reset();
        };
        filter = [](void* ctx, obs_encoder_t* encoder, struct encoder_packet* pkt)
        {
            return static_cast<LiveMask*>(ctx)->Filter(encoder, pkt);
        };
    }
} s_livemask_info;

#pragma endregion

//通用sei接口

struct SEIInfo
{
    int64_t frame_ts = 0;
    std::vector<uint8_t> sei;
};
class EncoderSEI : public boost::intrusive_ref_counter<EncoderSEI>
{
public:
    EncoderSEI()
    {
    }

    ~EncoderSEI()
    {
    }

    EncoderSEI(const EncoderSEI&) = delete;
    const EncoderSEI& operator = (const EncoderSEI&) = delete;

    struct encoder_packet* Filter(obs_encoder_t* encoder, struct encoder_packet* pkt)
    {
        std::lock_guard<std::mutex> lg(mutex_);
		if (sei_info_map_.empty())
			return pkt;
        last_pkt_ = *pkt;
        // 单位：纳秒
        auto frame_time = obs_encoder_pkt_ts_to_frame_ts(encoder, pkt->pts);
        if (frame_time <= 0)
            return pkt;
        // sei_info_map_是从大到小排列
        // 查找时间戳小于等于 frame_time 里面，最大的那个
        auto it = sei_info_map_.upper_bound(frame_time);
        if (it == sei_info_map_.end())
            return pkt;
        if (std::abs(it->first - frame_time) > 100000000L)
            return pkt;

        // 往 packet 插入 sei
        NalFilter nf(pkt->data, pkt->size);
        nf.DropAUD();
        for (auto& x : it->second)
            nf.InsertBeforeVCLNal(x.sei.data(), x.sei.size());
        last_pkt_data_ = nf.GetResult();
        last_pkt_.data = last_pkt_data_.data();
        last_pkt_.size = last_pkt_data_.size();
        sei_info_map_.erase(it);
        return &last_pkt_;
    }

    void AddSEI(const uint8_t* data, int size)
    {
        std::lock_guard<std::mutex> lg(mutex_);
        if (!encoder_is_running_)
            return;
        sei_msg_t msg;
        msg.sei_type = 5;
        msg.sei_data.insert(msg.sei_data.end(), data, data + size);
        auto rbsp = make_avc_sei(&msg, &msg + 1);
        std::vector<uint8_t> ebsp(4);
        rbsp_to_ebsp(rbsp.begin(), rbsp.end(), std::back_inserter(ebsp));
        ebsp[0] = 0;
        ebsp[1] = 0;
        ebsp[2] = 0;
        ebsp[3] = 1;

        SEIInfo info{ (int64_t)obs_get_video_frame_time(),std::move(ebsp) };
        auto it = sei_info_map_.find(info.frame_ts);
        if (it != sei_info_map_.end())
            it->second.push_back(info);
        else
            sei_info_map_.insert(std::make_pair(info.frame_ts, std::list<SEIInfo>{ info }));

        while (sei_info_map_.size() > 120)
            sei_info_map_.erase(sei_info_map_.rbegin()->first);
    }

    void Clear()
    {
        std::lock_guard<std::mutex> lg(mutex_);
        sei_info_map_.clear();
    }

    void Start()
    {
        std::lock_guard<std::mutex> lg(mutex_);
        sei_info_map_.clear();
        encoder_is_running_ = true;
    }

    void Stop()
    {
        std::lock_guard<std::mutex> lg(mutex_);
        encoder_is_running_ = false;
        sei_info_map_.clear();
    }

private:
    std::mutex mutex_;
    struct encoder_packet last_pkt_;
    std::vector<uint8_t> last_pkt_data_;
    std::map<int64_t, std::list<SEIInfo>, std::greater<int64_t>> sei_info_map_;
    bool encoder_is_running_ = false;
};

using EncoderSEIPtr = boost::intrusive_ptr<EncoderSEI>;

static struct EncoderSEIFilterInfo :public encoder_packet_filter_info
{
    EncoderSEIFilterInfo()
    {
        release = [](void* ctx)
        {
            EncoderSEIPtr(static_cast<EncoderSEI*>(ctx), false).reset();
        };
        filter = [](void* ctx, obs_encoder_t* encoder, struct encoder_packet* pkt)
        {
            return static_cast<EncoderSEI*>(ctx)->Filter(encoder, pkt);
        };
    }
}s_encoder_sei_filter_info;

}   // namespace

namespace obs_proxy {

    std::wstring FormatStreamingError(obs_proxy::StreamingErrorCode code)
    {
        switch (code)
        {
        case obs_proxy::StreamingErrorCode::BadPath:
            return LocalStr(IDS_STREAMING_TIP_BAD_PATH);

        case obs_proxy::StreamingErrorCode::ConnectFailed:
            return LocalStr(IDS_STREAMING_TIP_CONNECT_FAILED);

        case obs_proxy::StreamingErrorCode::InvalidStream:
            return LocalStr(IDS_STREAMING_TIP_INVALID_STREAM);

        case obs_proxy::StreamingErrorCode::Disconnected:
            return LocalStr(IDS_STREAMING_TIP_DISCONNECTED);

        case obs_proxy::StreamingErrorCode::StartFailure:
            return LocalStr(IDS_STREAMING_START_FAILED);

        case obs_proxy::StreamingErrorCode::InvalidMidAndChannel:
            return LocalStr(IDS_STREAMING_INVALID_MID_AND_CHANNEL);

        case obs_proxy::StreamingErrorCode::InvalidMid:
            return LocalStr(IDS_STREAMING_INVALID_MID);

        case obs_proxy::StreamingErrorCode::InvalidChannel:
            return LocalStr(IDS_STREAMING_INVALID_CHANNEL);

        case obs_proxy::StreamingErrorCode::Unknown:
            return LocalStr(IDS_STREAMING_TIP_UNKNOWN);

        default:
            NOTREACHED() << "Unknown error code";
            return LocalStr(IDS_STREAMING_TIP_UNKNOWN);
        }
    }

class OBSOutputServiceImpl : public OBSOutputService {
public:
    OBSOutputServiceImpl();

    ~OBSOutputServiceImpl();

    DISABLE_COPY(OBSOutputServiceImpl);

    void SetupStreamOutput();
    void SetupRecordOutput();

    Events& events() override;

    bool SetupRemoteServer(const std::string& server_address, const std::string& access_key,
        const std::string protocol) override;

    bool StartStreaming() override;

    void StopStreaming() override;

    void ForceStopStreaming() override;

    bool IsStreamingActive() const override;

    bool StartRecording() override;

    void StopRecording() override;

    bool IsRecordingActive() const override;

    bool IsOutputActive() const override;

    void ReloadEncoders() override;

    void RecreateOutputs() override;

    std::string GetRtmpHost() override {
        return last_used_rtmp_host_;
    }

    // return -1: no candidate hosts.
    int GetCurrentLeastUsedHostCount() override;

    int GetOutputTotalFrames() const override;

    int GetOutputDroppedFrames() const override;

    void SetPublishBitrate(int bitrate) override;

    void AddSEIInfo(const uint8_t* data, int size) override;

private:
    void LoadStreamingEncoders(const PrefService* pref_service);

    void LoadRecordingEncoders(const PrefService* pref_service);

    // Updates settings for both streaming video & audio encoders.
    void UpdateStreamingEncoderSettings(bool low_latency);

    void SetLiveStartTime(base::Time start_time) { live_start_time_ = start_time; }

    base::Time LiveStartTime() { return live_start_time_; }

    void SaveBitRateAndFps();

    int BitRateA() { return config_bitrate_a_; }

    int BitRateV() { return config_bitrate_v_; }

    std::string Fps() { return config_fps_; }

    // Functions to redirect signal handler.
    // All these functions are called on obs-managed threads.

    void OnStreamStarting(calldata_t* params);
    void OnStreamStopping(calldata_t* params);
    void OnStartStreaming(calldata_t* params);
    void OnStopStreaming(calldata_t* params);
    void OnStartRecording(calldata_t* params);
    void OnStopRecording(calldata_t* params);
    void OnPostSendFailing(calldata_t* params);


    static void OnEncodedQPFeedback(void* userdata, calldata_t* params);

    //RTMP host DNS resolve callback
    struct DNSResult
    {
        std::string host_name;
        int port;
        std::vector<char> address;
        int used_count;
    };

    DNSResult* GetCurrentLeastUsedDNSResultItem();
    static int DnsResolve(void* userdata, const char* host, int hostLen, int port, void* result, int* resultlen);
    int DnsResolveImpl(const char* host, int hostLen, int port, void* result, int* resultlen);

private:
    OBSOutputHelper::Ptr stream_output_;
    OBSOutputHelper::Ptr record_output_;

    OBSEncoder streaming_video_encoder_;
    OBSEncoder streaming_audio_encoder_;

    std::string streaming_audio_encoder_id_;

    OBSService remote_server_;

    std::unique_ptr<StreamingFeedbacker> streaming_feedbacker_;
    OBSSignal encoded_qp_feedback_signal_;

    Events events_;

    // dns candidate
    std::recursive_mutex dnsDataMutex_;
    std::vector<DNSResult> dnsCandidate;

    std::string last_used_rtmp_hostname_;
    std::string last_used_rtmp_host_;
    base::Time live_start_time_;
    int config_bitrate_v_;
    int config_bitrate_a_;
    std::string config_fps_;

    LiveMaskPtr live_mask_;
    EncoderSEIPtr encoder_sei_ptr_;

    struct RenderCallbackCtx : public boost::intrusive_ref_counter<RenderCallbackCtx>
    {
        LiveMaskPtr live_mask_;
        std::atomic_bool stop_flag_;

        RenderCallbackCtx(LiveMaskPtr livemask)
        {
            live_mask_ = livemask;
            stop_flag_.store(false);
        }
        virtual ~RenderCallbackCtx() {}
    };
    using RenderCallbackCtxPtr = boost::intrusive_ptr<RenderCallbackCtx>;

    RenderCallbackCtx* render_callback_ctx_;

    static void RenderCallback(void* param, uint32_t cx, uint32_t cy);
    void ResetRenderCallback()
    {
        if (render_callback_ctx_ != nullptr)
            render_callback_ctx_->stop_flag_ = true;
        render_callback_ctx_ = nullptr;
    }
};

OBSOutputService* CreateOBSOutputService()
{
    return new OBSOutputServiceImpl();
}

OBSOutputServiceImpl::OBSOutputServiceImpl()
    : config_bitrate_v_(0),
      config_bitrate_a_(0),
      live_mask_(new LiveMask),
      encoder_sei_ptr_ (new EncoderSEI),
      render_callback_ctx_(nullptr)
{
    static_assert(sizeof(this) <= sizeof(long long), "long long can't be used as pointer type.");

    ReloadEncoders();
}

OBSOutputServiceImpl::~OBSOutputServiceImpl()
{
    ResetRenderCallback();
}

void OBSOutputServiceImpl::LoadStreamingEncoders(const PrefService* pref_service)
{
    std::string video_encoder_id = GetConfiguredStreamingVideoEncoder(pref_service);
    LoadVideoEncoder(streaming_video_encoder_, encoded_qp_feedback_signal_, video_encoder_id,
                     kStreamingVideoEncoderName);

    int proper_bitrate = GetProperAudioBitrate();
    LoadAudioEncoder(streaming_audio_encoder_, streaming_audio_encoder_id_, kStreamingAudioEncoderName,
                     proper_bitrate);

    if (streaming_video_encoder_)
    {
        obs_encoder_set_packet_filter(streaming_video_encoder_, &s_livemask_info, LiveMaskPtr(live_mask_).detach());
        obs_encoder_set_packet_filter(streaming_video_encoder_, &s_encoder_sei_filter_info, EncoderSEIPtr(encoder_sei_ptr_).detach());
    }
}

void OBSOutputServiceImpl::LoadRecordingEncoders(const PrefService* pref_service)
{
    std::string video_quality = pref_service->GetString(prefs::kOutputRecordVideoQuality);
    if (video_quality != prefs::kVideoQualityStream) {
        DCHECK(false) << "Not implemented";
    }
}

void OBSOutputServiceImpl::SetupStreamOutput()
{
    CHECK(static_cast<obs_service_t*>(remote_server_) != nullptr) << "Service must be set before setup output stream";

    OBSData service_settings = obs_service_get_settings(remote_server_);
    CHECK(static_cast<obs_data_t*>(service_settings) != nullptr) << "Fail to retrieve service settings.";
    obs_data_release(service_settings);

    std::string protocol = "rtmp";

    if (service_settings)
    {
        const char* service_protocol = obs_data_get_string(service_settings, kKeyServerProtocol);
        if (service_protocol)
            protocol = service_protocol;
    }

    OBSData output_settings = obs_data_create();
    obs_data_release(output_settings);
    obs_data_set_int(output_settings, kDNSCallback, reinterpret_cast<long long>(&OBSOutputServiceImpl::DnsResolve));
    obs_data_set_int(output_settings, kDNSCallbackUserData, reinterpret_cast<long long>(this));

    int audio_br = GetPrefs()->GetInteger(prefs::kAudioBitRate);
    int video_br = GetPrefs()->GetInteger(prefs::kVideoBitRate);
    obs_data_set_int(output_settings, kSRTMaxBandwidth, (audio_br + video_br) * 1.03 * 1.5 * 1024 / 8);

    obs_data_set_bool(output_settings, "need_continuous", false);

    std::string output_id;

    if (protocol == "quic")
    {
        output_id = kRTMPOutputID;
        obs_data_set_string(output_settings, kKeyOutputTunnelName, "quic");
    }
    else if (protocol == "srt")
    {
        output_id = kSRTOutputID;
    }
    else
    {
        output_id = kRTMPOutputID;
        obs_data_erase(output_settings, kKeyOutputTunnelName);
    }

    stream_output_ = OBSOutputHelper::Create(output_id.c_str(), kOutputName, output_settings);
    ENSURE(stream_output_ != nullptr).Require();
    streaming_feedbacker_ = std::make_unique<StreamingFeedbacker>(*stream_output_);

    obs_output_set_service(*stream_output_, remote_server_);
    obs_output_set_video_encoder(*stream_output_, streaming_video_encoder_);
    obs_output_set_audio_encoder(*stream_output_, streaming_audio_encoder_, 0);

    stream_output_->ConnectSignal(kSignalStarting, [this](calldata_t* param) {
        OnStreamStarting(param);
    });

    stream_output_->ConnectSignal(kSignalStart, [this](calldata_t* param) {
        OnStartStreaming(param);
    });

    stream_output_->ConnectSignal(kSignalStopping, [this](calldata_t* param) {
        OnStreamStopping(param);
    });

    stream_output_->ConnectSignal(kSignalStop, [this](calldata_t* param) {
        OnStopStreaming(param);
    });

    streaming_feedbacker_->RegisterFeedbackHandler([this](const StreamingFeedbackMetrics& metrics) {
        if (events_.streaming_feedback) {
            events_.streaming_feedback(metrics);
        }
    });

    stream_output_->ConnectSignal(kSignalPostSendFail, [this](calldata_t* param) {
        OnPostSendFailing(param);
	});

}

void OBSOutputServiceImpl::SetupRecordOutput()
{
    record_output_ = OBSOutputHelper::Create(kFFmpegMuxerID, kFileOutputName, nullptr);
    ENSURE(record_output_ != nullptr).Require();

    obs_output_set_video_encoder(*record_output_, streaming_video_encoder_);
    obs_output_set_audio_encoder(*record_output_, streaming_audio_encoder_, 0);

    record_output_->ConnectSignal(kSignalStart, [this](calldata_t* param) {
        OnStartRecording(param);
    });

    record_output_->ConnectSignal(kSignalStop, [this](calldata_t* param) {
        OnStopRecording(param);
    });
}

bool OBSOutputServiceImpl::SetupRemoteServer(const std::string& server_address, const std::string& access_key,
    const std::string protocol)
{
    obs_data_t* service_settings = obs_data_create();
    ON_SCOPE_EXIT { obs_data_release(service_settings); };
    obs_data_set_string(service_settings, kKeyServerAddr, server_address.c_str());
    obs_data_set_string(service_settings, kKeyAccessKey, access_key.empty() ? kDefaultAccessKey :
        access_key.c_str());
    obs_data_set_string(service_settings, kKeyServerProtocol, protocol.c_str());
    obs_data_set_bool(service_settings, kKeyServerUseAuth, false);

    if (!remote_server_) {
        remote_server_ = obs_service_create(kRTMPBilibiliServiceID, kOBSServiceName, service_settings,
                                            nullptr);
        if (!remote_server_) {
            LOG(ERROR) << "Failed to setup remote servcie";
            return false;
        }

        obs_service_release(remote_server_);
    } else {
        obs_service_update(remote_server_, service_settings);
    }

    return true;
}

bool OBSOutputServiceImpl::StartStreaming()
{
    UpdateStreamingEncoderSettings(false);
    SetupStreamOutput();

    signal_handler_t* raw_signal = obs_encoder_get_signal_handler(streaming_video_encoder_);
    encoded_qp_feedback_signal_.Connect(raw_signal,
                                        kSignalEncodedQP,
                                        &OBSOutputServiceImpl::OnEncodedQPFeedback,
                                        this);

    // Set up for output.
    SetupOutputAV(*stream_output_);

    // Set delay options.
    bool use_delay = GetPrefs()->GetBoolean(prefs::kOutputUseDelay);
    bool delay_preserve = GetPrefs()->GetBoolean(prefs::kOutputDelayPreserve);
    uint32_t delay_sec = use_delay ? GetPrefs()->GetInteger(prefs::kOutputDelaySec) : 0;
    uint32_t delay_flag = delay_preserve ? OBS_OUTPUT_DELAY_PRESERVE : 0;
    obs_output_set_delay(*stream_output_, delay_sec, delay_flag);

    // Set reconnect options.
    bool reconnect = GetPrefs()->GetBoolean(prefs::kOutputReconnect);
    int max_retries = reconnect ? GetPrefs()->GetInteger(prefs::kOutputMaxRetries) : 0;
    int retry_delay = GetPrefs()->GetInteger(prefs::kOutputRetryDelay);
    obs_output_set_reconnect_settings(*stream_output_, max_retries, retry_delay);

    bool rv = obs_output_start(*stream_output_);
    return rv;
}

void OBSOutputServiceImpl::StopStreaming()
{
    if (stream_output_) {
        obs_output_stop(*stream_output_);
    }
}

void OBSOutputServiceImpl::ForceStopStreaming()
{
    if (stream_output_)
        obs_output_force_stop(*stream_output_);
}

bool OBSOutputServiceImpl::IsStreamingActive() const
{
    return stream_output_ ? obs_output_active(*stream_output_) : false;
}

bool OBSOutputServiceImpl::StartRecording()
{
    UpdateStreamingEncoderSettings(false);
    SetupRecordOutput();
    SetupOutputAV(*record_output_);

    obs_data_t* output_settings = obs_data_create();
    ON_SCOPE_EXIT { obs_data_release(output_settings); };

    const char* const key_name = "path";
    base::FilePath output_path = GenerateOutputFilePath(GetPrefs());

    // We need the temporary pardon on doing io to check, or even to create directories.
    {
        base::ThreadRestrictions::ScopedAllowIO allow_io;
        if (!EnsureDirectoryExists(output_path.DirName()))
        {
            LOG(ERROR) << "Failed to create directory " << output_path.DirName().AsUTF8Unsafe()
                       << " for saving recording files";
            return false;
        }
    }

    std::string format = GetPrefs()->GetString(prefs::kOutputRecordingFormat);

    obs_data_set_string(output_settings, key_name, output_path.AsUTF8Unsafe().c_str());
    ConfigRecOutputForFormat(output_settings, format);
    obs_output_update(*record_output_, output_settings);

    return obs_output_start(*record_output_);
}

void OBSOutputServiceImpl::StopRecording()
{
    if (record_output_)
        obs_output_stop(*record_output_);
}

bool OBSOutputServiceImpl::IsRecordingActive() const
{
    return record_output_ ? obs_output_active(*record_output_) : false;
}

OBSOutputService::Events& OBSOutputServiceImpl::events()
{
    return events_;
}

void OBSOutputServiceImpl::UpdateStreamingEncoderSettings(bool low_latency)
{
    FORCE_AS_NON_CONST_MEMBER_FUNCTION();

    if (low_latency) {
        LOG(INFO) << " Low latency config.";
    }

    obs_data_t* video_settings = obs_data_create();
    ON_SCOPE_EXIT { obs_data_release(video_settings); };

    int video_bitrate = GetPrefs()->GetInteger(prefs::kVideoBitRate);
    obs_data_set_int(video_settings, kKeyBitRate, video_bitrate);
    obs_data_set_int(video_settings, kKeyKeyFrameInterval, kPresetKeyFrameInterval);

    obs_data_t* audio_settings = obs_data_create();
    ON_SCOPE_EXIT { obs_data_release(audio_settings); };

    // set performance-quality balance and abr
    CHECK(streaming_video_encoder_ != nullptr) << "No video encoder object initialized!";
    if (streaming_video_encoder_) {
        const char* video_encoder_id = obs_encoder_get_id(streaming_video_encoder_);
        DCHECK(video_encoder_id != nullptr) << "Video encoder has no ID!";
        if (video_encoder_id) {
            auto video_stream_quality = GetPrefs()->GetInteger(prefs::kOutputStreamVideoQuality);
            if (video_stream_quality <= 1 || video_stream_quality > prefs::kVideoStreamQualityOptionsCount)
                video_stream_quality = prefs::kVideoStreamQualityDefault;

            const char* x264_balance_opts[] = {
                kVideoStreamX264QualityKey,
                kVideoStreamX264QualityPerformance,
                kVideoStreamX264QualityBalanced,
                kVideoStreamX264QualityQuality
            };

            const char* nvenc_balance_opts[] = {
                kVideoStreamNVENCQualityKey,
                kVideoStreamNVENCQualityPerformance,
                kVideoStreamNVENCQualityBalanced,
                kVideoStreamNVENCQualityQuality
            };

            const char* qsv_balance_opts[] = {
                kVideoStreamQSVQualityKey,
                kVideoStreamQSVQualityPerformance,
                kVideoStreamQSVQualityBalanced,
                kVideoStreamQSVQualityQuality
            };

            const char* amd_balance_opts[] = {
                kVideoStreamAMDQualityKey,
                kVideoStreamAMDQualityPerformance,
                kVideoStreamAMDQualityBalanced,
                kVideoStreamAMDQualityQuality
            };

            const char* nv_turing_enc_balance_opts[] = {
                kVideoStreamNVTURINGENCQualityKey,
                kVideoStreamNVTURINGENCQualityPerformance,
                kVideoStreamNVTURINGENCQualityBalanced,
                kVideoStreamNVTURINGENCQualityQuality
            };

            const char** selected_balance_opts = nullptr;
            const char* rate_control_value = kRateCBR;

            if (strcmp(video_encoder_id, kVideoEncoderX264ID) == 0) {
                selected_balance_opts = x264_balance_opts;
                rate_control_value = kRateX264ABR;

                if (low_latency) {
                    obs_data_set_string(
                        video_settings,
                        "x264opts",
                        "rc-lookahead=0 sync-lookahead=0 sliced-threads=1");
                }
            } else if (strcmp(video_encoder_id, kVideoEncoderNVENCID) == 0) {
                selected_balance_opts = nvenc_balance_opts;
                rate_control_value = kRateNVENCABR;

                if (low_latency) {
                    obs_data_set_int(video_settings, "lookahead", 0);
                }
            } else if (strcmp(video_encoder_id, kVideoEncoderQSVID) == 0) {
                selected_balance_opts = qsv_balance_opts;
                rate_control_value = kRateQSVABR;

                if (low_latency) {
                    obs_data_set_string(video_settings, "latency", "ultra-low");
                }
            }else if (strcmp(video_encoder_id, kVideoEncoderAMFID) == 0) {
                selected_balance_opts = amd_balance_opts;
                rate_control_value = kRateAMDABR;
            }
            else if (strcmp(video_encoder_id, kVideoEncoderNVTURINGENCID) == 0) {
                selected_balance_opts = nv_turing_enc_balance_opts;
                rate_control_value = kRateNVTURINGENCABR;

                if (low_latency) {
                    obs_data_set_int(video_settings, "lookahead", 0);
                }
            }

            DCHECK(selected_balance_opts != nullptr) << "Unrecognized encoder?";

            if (selected_balance_opts != nullptr) {
                obs_data_set_string(video_settings, selected_balance_opts[0],
                                    selected_balance_opts[video_stream_quality]);
            }

            std::string bit_rate_control = GetPrefs()->GetString(prefs::kVideoBitRateControl);
            if (bit_rate_control == kRateCBR) {
                obs_data_set_string(video_settings, kKeyRateControl, kRateCBR);
            } else {
                obs_data_set_string(video_settings, kKeyRateControl, rate_control_value);
            }
        }
    }

    int audio_bitrate = GetProperAudioBitrate();
    obs_data_set_int(audio_settings, kKeyBitRate, audio_bitrate);

    obs_service_apply_encoder_settings(remote_server_, video_settings, audio_settings);

    video_t* video = obs_get_video();
    video_format format = video_output_get_format(video);
    if (format != VIDEO_FORMAT_NV12 && format != VIDEO_FORMAT_I420) {
        obs_encoder_set_preferred_video_format(streaming_video_encoder_, VIDEO_FORMAT_NV12);
    }

    obs_encoder_update(streaming_video_encoder_, video_settings);
    obs_encoder_update(streaming_audio_encoder_, audio_settings);
}

bool OBSOutputServiceImpl::IsOutputActive() const
{
    return IsRecordingActive() || IsStreamingActive();
}

void OBSOutputServiceImpl::ReloadEncoders()
{
    auto prefs = GetPrefs();
    LoadStreamingEncoders(prefs);
    LoadRecordingEncoders(prefs);
}

void OBSOutputServiceImpl::RecreateOutputs() {
}

void OBSOutputServiceImpl::OnStreamStarting(calldata_t* params)
{
    UNUSED_VAR(params);

    BililiveThread::PostTask(
        BililiveThread::UI,
        FROM_HERE,
        base::BindLambda([this] {
            auto& handler = events().stream_starting;
            if (handler) {
                handler();
            }
        }));
}

void OBSOutputServiceImpl::OnStreamStopping(calldata_t* params)
{
    UNUSED_VAR(params);

    BililiveThread::PostTask(
        BililiveThread::UI,
        FROM_HERE,
        base::BindLambda([this] {
            auto& handler = events().stream_stopping;
            if (handler) {
                handler();
            }
        }));
}

void OBSOutputServiceImpl::OnStartStreaming(calldata_t* params)
{
    UNUSED_VAR(params);

    LOG(INFO) << "start streaming after start signal.";

    SetLiveStartTime(base::Time::Now());

    BililiveThread::PostTask(
        BililiveThread::UI,
        FROM_HERE,
        base::BindLambda([this] {
            SaveBitRateAndFps();
        }));


    BililiveThread::PostTask(
        BililiveThread::UI,
        FROM_HERE,
        base::BindLambda([this] {
            auto& handler = events().start_streaming;
            if (handler) {
                handler();
            }

            streaming_feedbacker_->StartWatching(kFeedbackRefreshInterval);
        }));

    //开播采集推流质量
    if (StreamingReportService::GetInstance() && stream_output_)
    {
        StreamingReportService::GetInstance()->SetOutput(*stream_output_);
    }

    live_mask_->Clear();

    ResetRenderCallback();
    render_callback_ctx_ = new RenderCallbackCtx(live_mask_);

    obs_add_main_render_callback(&OBSOutputServiceImpl::RenderCallback, RenderCallbackCtxPtr(render_callback_ctx_).detach());

    if (encoder_sei_ptr_)
    {
        encoder_sei_ptr_->Start();
    }
}

void OBSOutputServiceImpl::OnPostSendFailing(calldata_t* params)
{
    UNUSED_VAR(params);

	BililiveThread::PostTask(
		BililiveThread::UI,
		FROM_HERE,
		base::BindLambda([this] {
			auto& handler = events().post_send_failing;
			if (handler) {
				handler();
			}
			}));
}


void OBSOutputServiceImpl::RenderCallback(void* param, uint32_t cx, uint32_t cy) {
    auto ctx = static_cast<RenderCallbackCtx*>(param);
    if (ctx->stop_flag_)
    {
        obs_remove_main_render_callback(&OBSOutputServiceImpl::RenderCallback, param);
        RenderCallbackCtxPtr(ctx, false).reset();
    }
    else
        ctx->live_mask_->TickLiveMask();
}

void OBSOutputServiceImpl::OnStopStreaming(calldata_t* params)
{
    if (encoder_sei_ptr_)
    {
        encoder_sei_ptr_->Stop();
    }

    ResetRenderCallback();

    int result_code = calldata_int(params, "code");
    const char* last_error_msg = nullptr;
    calldata_get_string(params, "last_error", &last_error_msg);

    LOG(INFO) << "stop streaming after stop signal.";

    //解析额外错误信息
    static std::regex extra_errmsg_regex("(?:=(?:([^=]+)_BEGIN)=(.*?)=\\1_END=)");
    std::string errmsg;
    std::map<std::string, std::string> extra_msgs;

    if (last_error_msg)
    {
        std::smatch extra_msg_match_result;
        std::string errmsgbuf = last_error_msg;
        while (std::regex_search(errmsgbuf, extra_msg_match_result, extra_errmsg_regex))
        {
            if (extra_msg_match_result[1].matched && extra_msg_match_result[2].matched)
                extra_msgs[extra_msg_match_result[1].str()] = extra_msg_match_result[2].str();
            errmsgbuf.replace(extra_msg_match_result.position(0), extra_msg_match_result[0].length(), "");
        }
        errmsg = errmsgbuf;
    }

    StreamingErrorDetails error_details;
    error_details.errmsg = errmsg;

    auto iter = extra_msgs.find("RTMP_ERRFMT");
    if (iter != extra_msgs.end()) {
        error_details.rtmp_errfmt = iter->second;
    }

    iter = extra_msgs.find("RTMP_ERRMSG");
    if (iter != extra_msgs.end()) {
        error_details.rtmp_errmsg = iter->second;
    }

    //处理错误
    StreamingStopDetails stop_details;
    stop_details.config_bitrate_a = BitRateA();
    stop_details.config_bigrate_v = BitRateV();
    stop_details.config_fps = Fps();
    if (stream_output_)
    {
        stop_details.total_bytes = obs_output_get_total_bytes(*stream_output_);
        stop_details.total_frames = obs_output_get_total_frames(*stream_output_);
        stop_details.frames_dropped = obs_output_get_frames_dropped(*stream_output_);

        auto output_settings = obs_output_get_settings(*stream_output_);
        if (output_settings)
        {
            stop_details.encoder_generated_bytes = obs_data_get_int(output_settings, "encoder_generated_bytes");
            stop_details.drop_filler_bytes = obs_data_get_int(output_settings, "drop_filler_bytes");
            obs_data_release(output_settings);
        }
    }
    else
    {
        stop_details.total_bytes = 0;
        stop_details.total_frames = 0;
        stop_details.frames_dropped = 0;
        stop_details.encoder_generated_bytes = 0;
        stop_details.drop_filler_bytes = 0;
    }
    stop_details.duration = base::Time::Now().ToTimeT() - (LiveStartTime()).ToTimeT();

    BililiveThread::PostTask(
        BililiveThread::UI,
        FROM_HERE,
        base::BindLambda([this, result_code, error_details, stop_details] {
            streaming_feedbacker_->StopWatching();
            //关播不再采集推流质量
            if (StreamingReportService::GetInstance())
            {
                StreamingReportService::GetInstance()->ReleaseOutput();
            }

            if (result_code == OBS_OUTPUT_SUCCESS) {
                auto& stop_handler = events().stop_streaming;
                if (stop_handler) {
                    stop_handler(stop_details);
                }
            } else {
                auto& error_handler = events().streaming_error;
                if (error_handler) {
                    StreamingErrorCode error_code = StreamingErrorCode::Unknown;
                    switch (result_code) {
                        case OBS_OUTPUT_BAD_PATH:
                            error_code = StreamingErrorCode::BadPath;
                            break;

                        case OBS_OUTPUT_CONNECT_FAILED:
                            error_code = StreamingErrorCode::ConnectFailed;
                            break;

                        case OBS_OUTPUT_INVALID_STREAM:
                            error_code = StreamingErrorCode::InvalidStream;
                            break;

                        case OBS_OUTPUT_DISCONNECTED:
                            error_code = StreamingErrorCode::Disconnected;
                            break;

                        default:
                            LOG(WARNING) << "Unidentified streaming error: " << result_code;
                            break;
                    }

                    error_handler(error_code, error_details);
                }
            }
        }));
}


void OBSOutputServiceImpl::AddSEIInfo(const uint8_t* data, int size)
{
    if (encoder_sei_ptr_)
    {
        encoder_sei_ptr_->AddSEI(data, size);
    }
}

// static
void OBSOutputServiceImpl::OnStartRecording(calldata_t* params)
{
    UNUSED_VAR(params);

    BililiveThread::PostTask(
        BililiveThread::UI,
        FROM_HERE,
        base::BindLambda([this] {
            auto& handler = events().start_recording;
            if (handler) {
                handler();
            }
        }));
}

// static
void OBSOutputServiceImpl::OnStopRecording(calldata_t* params)
{
    int result_code = static_cast<int>(calldata_int(params, "code"));
    auto raw_output = static_cast<obs_output_t*>(calldata_ptr(params, "output"));
    obs_data_t* settings = obs_output_get_settings(raw_output);
    ON_SCOPE_EXIT { obs_data_release(settings); };
    std::string output_path = obs_data_get_string(settings, "path");

    BililiveThread::PostTask(
        BililiveThread::UI,
        FROM_HERE,
        base::BindLambda([this, result_code, output_path] {
            auto path = base::FilePath::FromUTF8Unsafe(output_path);

            if (result_code == OBS_OUTPUT_SUCCESS) {
                auto& stop_handler = events().stop_recording;
                if (stop_handler) {
                    stop_handler(path);
                }
            } else {
                auto& error_handler = events().recording_error;
                if (error_handler) {
                    RecordingErrorCode error_code = RecordingErrorCode::Unknown;
                    switch (result_code) {
                        case OBS_OUTPUT_UNSUPPORTED:
                            error_code = RecordingErrorCode::Unsupported;
                            break;

                        case OBS_OUTPUT_NO_SPACE:
                            error_code = RecordingErrorCode::NoSpace;
                            break;

                        default:
                            LOG(WARNING) << "Unidentified recording error: " << result_code;
                            break;
                    }

                    error_handler(error_code, path);
                }
            }
        }));
}

// static
void OBSOutputServiceImpl::OnEncodedQPFeedback(void* userdata, calldata_t* params)
{
    auto self = static_cast<OBSOutputServiceImpl*>(userdata);

    base::StringPiece frame_type(calldata_string(params, "frame_type"));
    if (frame_type != "P") {
        return;
    }

    int value = static_cast<int>(calldata_float(params, "qp") + 0.5);

    BililiveThread::PostTask(
        BililiveThread::UI,
        FROM_HERE,
        base::BindLambda([value, self] {
            auto& handler = self->events().encoded_qp_feedback;
            if (handler) {
                handler(value);
            }
        }));
}

void OBSOutputServiceImpl::SaveBitRateAndFps() {
    config_bitrate_a_ = obs_proxy::GetPrefs()->GetInteger(prefs::kAudioBitRate);
    config_bitrate_v_ = obs_proxy::GetPrefs()->GetInteger(prefs::kVideoBitRate);
    config_fps_ = obs_proxy::GetPrefs()->GetString(prefs::kVideoFPSCommon);
}

int OBSOutputServiceImpl::GetOutputTotalFrames() const
{
    return stream_output_ ? obs_output_get_total_frames(*stream_output_) : 0;
}

int OBSOutputServiceImpl::GetOutputDroppedFrames() const
{
    return stream_output_ ? obs_output_get_frames_dropped(*stream_output_) : 0;
}

void OBSOutputServiceImpl::SetPublishBitrate(int bitrate)
{
    FORCE_AS_NON_CONST_MEMBER_FUNCTION();

    obs_data_t* video_settings = obs_data_create();
    ON_SCOPE_EXIT{ obs_data_release(video_settings); };

    obs_data_set_int(video_settings, kKeyBitRate, bitrate);
    obs_data_set_int(video_settings, kKeyKeyFrameInterval, kPresetKeyFrameInterval);

    // set performance-quality balance and abr
    CHECK(streaming_video_encoder_ != nullptr) << "No video encoder object initialized!";
    if (streaming_video_encoder_) {
        const char* video_encoder_id = obs_encoder_get_id(streaming_video_encoder_);
        DCHECK(video_encoder_id != nullptr) << "Video encoder has no ID!";
        if (video_encoder_id) {
            auto video_stream_quality = GetPrefs()->GetInteger(prefs::kOutputStreamVideoQuality);
            if (video_stream_quality <= 1 || video_stream_quality > prefs::kVideoStreamQualityOptionsCount)
                video_stream_quality = prefs::kVideoStreamQualityDefault;

            const char* x264_balance_opts[] = {
                kVideoStreamX264QualityKey,
                kVideoStreamX264QualityPerformance,
                kVideoStreamX264QualityBalanced,
                kVideoStreamX264QualityQuality
            };

            const char* nvenc_balance_opts[] = {
                kVideoStreamNVENCQualityKey,
                kVideoStreamNVENCQualityPerformance,
                kVideoStreamNVENCQualityBalanced,
                kVideoStreamNVENCQualityQuality
            };

            const char* qsv_balance_opts[] = {
                kVideoStreamQSVQualityKey,
                kVideoStreamQSVQualityPerformance,
                kVideoStreamQSVQualityBalanced,
                kVideoStreamQSVQualityQuality
            };

            const char* amd_balance_opts[] = {
                kVideoStreamAMDQualityKey,
                kVideoStreamAMDQualityPerformance,
                kVideoStreamAMDQualityBalanced,
                kVideoStreamAMDQualityQuality
            };

            const char* nv_turing_enc_balance_opts[] = {
                kVideoStreamNVTURINGENCQualityKey,
                kVideoStreamNVTURINGENCQualityPerformance,
                kVideoStreamNVTURINGENCQualityBalanced,
                kVideoStreamNVTURINGENCQualityQuality
            };

            const char** selected_balance_opts = nullptr;
            const char* rate_control_value = kRateCBR;

            if (strcmp(video_encoder_id, kVideoEncoderX264ID) == 0) {
                selected_balance_opts = x264_balance_opts;
                rate_control_value = kRateX264ABR;
            }
            else if (strcmp(video_encoder_id, kVideoEncoderNVENCID) == 0) {
                selected_balance_opts = nvenc_balance_opts;
                rate_control_value = kRateNVENCABR;
            }
            else if (strcmp(video_encoder_id, kVideoEncoderQSVID) == 0) {
                selected_balance_opts = qsv_balance_opts;
                rate_control_value = kRateQSVABR;
            }
            else if (strcmp(video_encoder_id, kVideoEncoderAMFID) == 0) {
                selected_balance_opts = amd_balance_opts;
                rate_control_value = kRateAMDABR;
            }
            else if (strcmp(video_encoder_id, kVideoEncoderNVTURINGENCID) == 0) {
                selected_balance_opts = nv_turing_enc_balance_opts;
                rate_control_value = kRateNVTURINGENCABR;
            }

            DCHECK(selected_balance_opts != nullptr) << "Unrecognized encoder?";

            if (selected_balance_opts != nullptr) {
                obs_data_set_string(video_settings, selected_balance_opts[0],
                    selected_balance_opts[video_stream_quality]);
            }

            std::string bit_rate_control = GetPrefs()->GetString(prefs::kVideoBitRateControl);
            if (bit_rate_control == kRateCBR) {
                obs_data_set_string(video_settings, kKeyRateControl, kRateCBR);
            }
            else {
                obs_data_set_string(video_settings, kKeyRateControl, rate_control_value);
            }
        }
        obs_encoder_update(streaming_video_encoder_, video_settings);
    }

}

#pragma region(DNS)

#include <winsock2.h>
#include <WS2tcpip.h>

int OBSOutputServiceImpl::DnsResolve(void* userdata, const char* host, int hostLen, int port, void* result, int* resultlen)
{
    return static_cast<OBSOutputServiceImpl*>(userdata)->DnsResolveImpl(host, hostLen, port, result, resultlen);
}

int OBSOutputServiceImpl::GetCurrentLeastUsedHostCount()
{
    std::lock_guard<std::recursive_mutex> lg(dnsDataMutex_);

    auto result = GetCurrentLeastUsedDNSResultItem();
    if (result == nullptr)
        return -1;
    else
        return result->used_count;
}

OBSOutputServiceImpl::DNSResult* OBSOutputServiceImpl::GetCurrentLeastUsedDNSResultItem()
{
    std::lock_guard<std::recursive_mutex> lg(dnsDataMutex_);

    if (dnsCandidate.empty())
        return nullptr;

    size_t nextItem = 0;
    for (size_t i = 0; i < dnsCandidate.size(); ++i)
    {
        if (dnsCandidate[nextItem].used_count > dnsCandidate[i].used_count)
            nextItem = i;
    }

    return &dnsCandidate[nextItem];
}

int OBSOutputServiceImpl::DnsResolveImpl(const char* host, int hostLen, int port, void* result, int* resultlen)
{
    std::lock_guard<std::recursive_mutex> lg(dnsDataMutex_);

    std::string strHost(host, host + hostLen);

    if (!dnsCandidate.empty())
    {
        if (strHost != dnsCandidate.front().host_name || dnsCandidate.front().port != port)
            dnsCandidate.clear();
    }

    if (dnsCandidate.empty())
    {
        std::string strPort = std::to_string(port);

        struct addrinfo hints = { 0 };
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        struct addrinfo* addrInfo = nullptr;
        if (getaddrinfo(strHost.c_str(), strPort.c_str(), &hints, &addrInfo) != 0)
        {
            int wsaLastError = WSAGetLastError();
            LOG(ERROR) << "[RTMP DNS Resolver] Could not resolve "
                << strHost << ':' << port
                << '(' << wsaLastError << ':' << gai_strerrorA(wsaLastError) << ')';
            return wsaLastError;
        }

        ON_SCOPE_EXIT{ if (addrInfo) freeaddrinfo(addrInfo); };

        for (struct addrinfo* iter = addrInfo; iter != nullptr; iter = iter->ai_next)
        {
            if (iter->ai_family == AF_INET)
            {
                DNSResult resultItem;
                resultItem.host_name = strHost;
                resultItem.port = port;
                resultItem.used_count = 0;
                resultItem.address.resize(iter->ai_addrlen);
                memcpy(resultItem.address.data(), iter->ai_addr, iter->ai_addrlen);

                dnsCandidate.push_back(std::move(resultItem));
            }
        }

        LOG(INFO) << "[RTMP DNS Resolver] " << strHost << ": Get " << dnsCandidate.size() << " IP address(es).";
    }

    auto* nextItem = GetCurrentLeastUsedDNSResultItem();
    if (nextItem == nullptr)
    {
        return WSAHOST_NOT_FOUND;
    }

    ++nextItem->used_count;
    *resultlen = nextItem->address.size();
    memcpy(result, nextItem->address.data(), nextItem->address.size());

    auto sin_result = static_cast<struct sockaddr_in*>(result);
    char rtmp_host_storage[48]{ 0 };
    const char* rtmp_host = inet_ntop(sin_result->sin_family, &sin_result->sin_addr, rtmp_host_storage, _countof(rtmp_host_storage));
    if (rtmp_host) last_used_rtmp_host_ = rtmp_host;
    last_used_rtmp_hostname_ = strHost;

    LOG(INFO) << "[RTMP DNS Resolver] " << strHost << " -> " << (rtmp_host ? rtmp_host : "(null)");

    return 0;
}

#pragma endregion


}   // namespace obs_proxy
