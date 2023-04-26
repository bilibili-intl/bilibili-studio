#include "bililive/bililive/livehime/gift_image/image_fetcher.h"

#include "base/ext/bind_lambda.h"
#include "base/ext/callable_callback.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/thread_restrictions.h"

#include "ui/base/resource/resource_bundle.h"

#include "bililive/bililive/livehime/danmaku_hime/danmaku_hime_presenter_impl.h"
#include "bililive/bililive/livehime/function_control/app_function_controller.h"
#include "bililive/bililive/utils/bililive_image_util.h"
#include "bililive/bililive/utils/convert_util.h"
#include "bililive/common/bililive_logging.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"

#include "grit/theme_resources.h"


//部分资源线上没有 res id,这里自己定
enum CustomMedalResourceID
{
    CROWN_UNLIT_ID = 0x10000001,
    GUARD_CAPTAIN_ID = 0x10000002,
    GUARD_ADMIRAL_ID = 0x10000003,
    GUARD_GUARD_ID = 0x10000003,
};

namespace {
    const char kGiftCoinTypeGold[] = "gold";
}

ImageFetcher* ImageFetcher::GetInstance() {
    return Singleton<ImageFetcher>::get();
}

ImageFetcher::ImageFetcher() :
    wpf_(this),
    presenter_(std::make_unique<DanmakuHimePresenterImpl>()),
    icon_cache_(64),
    gif_cache_(64),
    url_cache_(64),
    title_cache_(64),
    image_cache_(64)
{
    LoadAssets();

    auto dir_path= bililive::GetBililiveUserDataDirectory();
    dir_path = dir_path.Append(L"Cache").Append(L"Gift Icons");
    base::ThreadRestrictions::ScopedAllowIO allow;
    if (!base::PathExists(dir_path)) {
        file_util::CreateDirectoryW(dir_path);
    }

    dir_path = dir_path.Append(L"record.dat");
    if (!dir_path.empty()) {
        gift_icon_recorder_.open(dir_path.value());
    }
   
}

ImageFetcher::~ImageFetcher() {
    download_thread_stop_ = true;
    gift_icon_recorder_.close();
}

std::string ImageFetcher::FetchAsset(Assets res_id) {
    std::lock_guard<std::mutex> guard(assets_sync_);

    auto it = asset_map_.find(res_id);
    if (it != asset_map_.end()) {
        return it->second;
    }

    NOTREACHED();
    return {};
}

bool ImageFetcher::IsGiftConfigLoaded()
{
    return presenter_->IsInitialized();
}

void ImageFetcher::RequestGiftConfigListSync()
{
    //presenter_->RequestGiftConfigListSync();
}

int ImageFetcher::FetchGiftType(int64_t gift_id) {
    DCHECK(!BililiveThread::CurrentlyOn(BililiveThread::UI));

    std::lock_guard<std::mutex> guard(gift_config_sync_);

    return -1;
}

int ImageFetcher::FetchGiftStayTime(int64_t gift_id) {
    DCHECK(!BililiveThread::CurrentlyOn(BililiveThread::UI));

    std::lock_guard<std::mutex> guard(gift_config_sync_);

    secret::DanmakuHimeService::GiftConfigInfo config;

    return -1;
}

ImageFetcher::ComboColorInfo ImageFetcher::FetchComboColorsByResId(int res_id) {
    DCHECK(!BililiveThread::CurrentlyOn(BililiveThread::UI));

    std::lock_guard<std::mutex> guard(gift_config_sync_);

    return {};
}

ImageFetcher::ComboColorInfo ImageFetcher::FetchComboColorsByGiftId(int64_t gift_id) {
    DCHECK(!BililiveThread::CurrentlyOn(BililiveThread::UI));

    std::lock_guard<std::mutex> guard(gift_config_sync_);

    return {};
}

std::string ImageFetcher::FetcherMP4EffectURL(int64_t effect_id)
{
    std::lock_guard<std::mutex> guard(effect_sync_);

    std::string url;

    return url;
}

std::string ImageFetcher::FetchGiftImageSync(int64_t gift_id) {
    DCHECK(!BililiveThread::CurrentlyOn(BililiveThread::UI));

    std::lock_guard<std::mutex> guard(gift_icon_sync_);

    // 先读内存缓存
    auto it = icon_cache_.Get(gift_id);
    if (it != icon_cache_.end()) {
        return it->second;
    }

    std::string data;
    secret::DanmakuHimeService::GiftConfigInfo info;

    if (!data.empty()) {
        icon_cache_.Put(info.id, data);
    }

    return data;
}

std::string ImageFetcher::FetchGiftDynamicImageSyncByUrl(int64_t gift_id, const std::string& url)
{
    DCHECK(!BililiveThread::CurrentlyOn(BililiveThread::UI));

    std::lock_guard<std::mutex> guard(gift_gif_sync_);

    auto it = gif_cache_.Get(gift_id);
    if (it != gif_cache_.end())
    {
        return it->second;
    }

    std::string data;
    secret::DanmakuHimeService::GiftConfigInfo info;
    base::ThreadRestrictions::ScopedAllowWait auto_allow;

    info.id = gift_id;
    info.gif_url = url;

    data = presenter_->RequestImageSync(url);
    if (!data.empty())
    {
        WriteFileToDisk(info, ImageType::GIF, data);
        gif_cache_.Put(gift_id, data);
    }

    return data;
}

std::string ImageFetcher::FetchGiftDynamicImageSync(int64_t gift_id)
{
    DCHECK(!BililiveThread::CurrentlyOn(BililiveThread::UI));

    std::lock_guard<std::mutex> guard(gift_gif_sync_);

    // 先读内存缓存
    auto it = gif_cache_.Get(gift_id);
    if (it != gif_cache_.end()) {
        return it->second;
    }

    std::string data;
    secret::DanmakuHimeService::GiftConfigInfo info;

    if (!data.empty()) {
        gif_cache_.Put(info.id, data);
    }

    return data;
}

std::string ImageFetcher::FetchGiftImageOnlyOnMRU(int64_t gift_id) {
    std::lock_guard<std::mutex> guard(gift_icon_sync_);

    auto it = icon_cache_.Get(gift_id);
    if (it != icon_cache_.end()) {
        return it->second;
    }

    return {};
}

void ImageFetcher::SaveGifWicBitmaps(int64_t gift_id, const WICBitmaps& bitmaps) {
    std::lock_guard<std::mutex> guard(gif_bitmaps_sync_);
    gifs_bitmaps_[gift_id] = bitmaps;
}

WICBitmaps ImageFetcher::FetchGifWicBitmaps(int64_t gift_id) {
    std::lock_guard<std::mutex> guard(gif_bitmaps_sync_);
    auto it = gifs_bitmaps_.find(gift_id);
    if (it != gifs_bitmaps_.end()) {
        return it->second;
    }
    return {};
}


std::string ImageFetcher::FetchImageSync(const std::string& url) {
    DCHECK(!BililiveThread::CurrentlyOn(BililiveThread::UI));

    if (url.empty()) {
        return {};
    }

    std::lock_guard<std::mutex> guard(url_icon_sync_);

    auto it = url_cache_.Get(url);
    if (it != url_cache_.end()) {
        return it->second;
    }

    auto result = presenter_->RequestImageSync(url);
    if (result.empty()) {
        return {};
    }

    url_cache_.Put(url, result);

    return result;
}

int64_t ImageFetcher::GetGiftEffectID(int64_t gift_id, bool is_batch, int num)
{
    return 0;
}

std::string ImageFetcher::GetGiftEffectURL(int64_t effect_id)
{   
    return "";
}

gfx::ImageSkia ImageFetcher::CacheImage(const base::string16& img_path)
{
    std::lock_guard<std::mutex> lock{ cache_image_sync_ };

    gfx::ImageSkia img;

    base::FilePath img_file_path(img_path);
    auto it = image_cache_.Get(img_file_path.BaseName().AsUTF16Unsafe());
    if (it != image_cache_.end())
    {
        return it->second;
    }
    else
    {
        img = bililive::decodeImgFromFile(img_file_path);
        if (!img.isNull())
        {
            image_cache_.Put(img_file_path.BaseName().AsUTF16Unsafe(), img);
        }
    }

    return img;
}

gfx::ImageSkia ImageFetcher::FetchCacheImage(const base::string16& img_name)
{
    std::lock_guard<std::mutex> lock{ cache_image_sync_ };

    auto it = image_cache_.Get(img_name);

    if (it != image_cache_.end())
    {
        return it->second;
    }
    else
    {
        return gfx::ImageSkia();
    }
}

void ImageFetcher::LoadAssets() {
    ResourceBundle& rb = ResourceBundle::GetSharedInstance();

    asset_map_[DEF_AVATAR] = rb.GetRawDataResource(IDR_LIVEMAIN_TITLEBAR_USER_FACE).as_string();
    asset_map_[YEARLY_LORD] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_YEARLY_LORD).as_string();
    asset_map_[MONTHLY_LORD] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_MONTHLY_LORD).as_string();

    asset_map_[GOVERNOR] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_GOVERNOR).as_string();
    asset_map_[ADMIRAL] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_ADMIRAL).as_string();
    asset_map_[CAPTAIN] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_CAPTAIN).as_string();

    asset_map_[GOVERNOR_LARGE] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_GOVERNOR_LARGE).as_string();
    asset_map_[ADMIRAL_LARGE] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_ADMIRAL_LARGE).as_string();
    asset_map_[CAPTAIN_LARGE] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_CAPTAIN_LARGE).as_string();

    asset_map_[GOVERNOR_FRAME] = rb.GetRawDataResource(IDR_LIVEHIME_AVATAR_FRAME_GOVERNOR).as_string();
    asset_map_[ADMIRAL_FRAME] = rb.GetRawDataResource(IDR_LIVEHIME_AVATAR_FRAME_ADMIRAL).as_string();
    asset_map_[CAPTAIN_FRAME] = rb.GetRawDataResource(IDR_LIVEHIME_AVATAR_FRAME_CAPTAIN).as_string();

    asset_map_[MEDAL_GOVERNOR] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_MEDAL_GOVERNOR).as_string();
    asset_map_[MEDAL_ADMIRAL] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_MEDAL_ADMIRAL).as_string();
    asset_map_[MEDAL_CAPTAIN] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_MEDAL_CAPTAIN).as_string();

    asset_map_[SILVER_MELON] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_SILVER_MELON).as_string();
    asset_map_[GOLD_ANCHOR] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_GOLD_ANCHOR).as_string();
    asset_map_[SILVER_ANCHOR] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_SILVER_ANCHOR).as_string();

    asset_map_[NO_FACE] = rb.GetRawDataResource(IDR_LIVEMAIN_TITLEBAR_USER_FACE).as_string();

    asset_map_[NUMBER_0] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_NUMBER_0).as_string();
    asset_map_[NUMBER_1] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_NUMBER_1).as_string();
    asset_map_[NUMBER_2] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_NUMBER_2).as_string();
    asset_map_[NUMBER_3] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_NUMBER_3).as_string();
    asset_map_[NUMBER_4] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_NUMBER_4).as_string();
    asset_map_[NUMBER_5] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_NUMBER_5).as_string();
    asset_map_[NUMBER_6] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_NUMBER_6).as_string();
    asset_map_[NUMBER_7] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_NUMBER_7).as_string();
    asset_map_[NUMBER_8] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_NUMBER_8).as_string();
    asset_map_[NUMBER_9] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_NUMBER_9).as_string();
    asset_map_[NUMBER_MULTIPLE] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_NUMBER_MULTIPLE).as_string();

    asset_map_[NUMBER_WHITE_0] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_NUMBER_WHITE_0).as_string();
    asset_map_[NUMBER_WHITE_1] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_NUMBER_WHITE_1).as_string();
    asset_map_[NUMBER_WHITE_2] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_NUMBER_WHITE_2).as_string();
    asset_map_[NUMBER_WHITE_3] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_NUMBER_WHITE_3).as_string();
    asset_map_[NUMBER_WHITE_4] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_NUMBER_WHITE_4).as_string();
    asset_map_[NUMBER_WHITE_5] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_NUMBER_WHITE_5).as_string();
    asset_map_[NUMBER_WHITE_6] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_NUMBER_WHITE_6).as_string();
    asset_map_[NUMBER_WHITE_7] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_NUMBER_WHITE_7).as_string();
    asset_map_[NUMBER_WHITE_8] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_NUMBER_WHITE_8).as_string();
    asset_map_[NUMBER_WHITE_9] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_NUMBER_WHITE_9).as_string();
    asset_map_[NUMBER_WHITE_MULTIPLE] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_NUMBER_WHITE_MULTIPLE).as_string();
    asset_map_[STRING_COMBO] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_STRING_COMBO).as_string();
    asset_map_[STRING_SPECIAL_COMBO] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_STRING_SPECIAL_COMBO).as_string();
    asset_map_[SUPER_CHAT] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_SUPER_CHAT).as_string();

    asset_map_[RANK1] = rb.GetRawDataResource(IDR_LIVEHIME_HE_RANK_1).as_string();
    asset_map_[RANK2] = rb.GetRawDataResource(IDR_LIVEHIME_HE_RANK_2).as_string();
    asset_map_[RANK3] = rb.GetRawDataResource(IDR_LIVEHIME_HE_RANK_3).as_string();

    asset_map_[HOT_RANKING] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_HOT_RANKING).as_string();
    asset_map_[MVP] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_MVP).as_string();
    asset_map_[VOICE_IMAGE_WHITE] = rb.GetRawDataResource(IDR_LIVEHIME_VOICE_DANMU_WHITE).as_string();
    asset_map_[VOICE_IMAGE] = rb.GetRawDataResource(IDR_LIVEHIME_VOICE_DANMU_RIPE).as_string();
    asset_map_[VOICE_PAUSE_IMG] = rb.GetRawDataResource(IDR_LIVEHIME_VOICE_PAUSE).as_string();
    asset_map_[VOICE_PLAY_IMG] = rb.GetRawDataResource(IDR_LIVEHIME_VOICE_PLAY).as_string();
    asset_map_[VOICE_GIF] = rb.GetRawDataResource(IDR_LIVEHIME_VOICE_GIF).as_string();
    asset_map_[POPULARITY] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_POPULARITY).as_string();
    asset_map_[POPULARITY_BLACK] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKUHIME_ICON_POPULARITY_BLACK).as_string();

    asset_map_[CMP] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKU_EFFECT_CMP).as_string();
    asset_map_[STORM] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKU_EFFECT_STORM).as_string();
    asset_map_[COMPLETION] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKU_EFFECT_COMPLETION).as_string();
    asset_map_[ASSISTANT_RIGHT] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKU_ASSISTANT_RIGHT).as_string();
    asset_map_[RED_PACKET] = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKU_RED_PACKET).as_string();
    asset_map_[DAMAKU_CROWN_1_4] = rb.GetRawDataResource(IDR_PRESET_DANMAKU_CROWN_1_4).as_string();
    asset_map_[DAMAKU_CROWN_5_8] = rb.GetRawDataResource(IDR_PRESET_DANMAKU_CROWN_5_8).as_string();
    asset_map_[DAMAKU_CROWN_9_12] = rb.GetRawDataResource(IDR_PRESET_DANMAKU_CROWN_9_12).as_string();
    asset_map_[DAMAKU_CROWN_13_16] = rb.GetRawDataResource(IDR_PRESET_DANMAKU_CROWN_13_16).as_string();
    asset_map_[DAMAKU_CROWN_17_20] = rb.GetRawDataResource(IDR_PRESET_DANMAKU_CROWN_17_20).as_string();
    asset_map_[DAMAKU_CROWN_21_24] = rb.GetRawDataResource(IDR_PRESET_DANMAKU_CROWN_21_24).as_string();
    asset_map_[DAMAKU_CROWN_25_28] = rb.GetRawDataResource(IDR_PRESET_DANMAKU_CROWN_25_28).as_string();
    asset_map_[DAMAKU_CROWN_29_32] = rb.GetRawDataResource(IDR_PRESET_DANMAKU_CROWN_29_32).as_string();
    asset_map_[DAMAKU_CROWN_33_36] = rb.GetRawDataResource(IDR_PRESET_DANMAKU_CROWN_33_36).as_string();
    asset_map_[DAMAKU_CROWN_37_40] = rb.GetRawDataResource(IDR_PRESET_DANMAKU_CROWN_37_40).as_string();
    asset_map_[DAMAKU_CROWN_CAPTAIN] = rb.GetRawDataResource(IDR_PRESET_DANMAKU_CROWN_CAPTAIN).as_string();
    asset_map_[DAMAKU_CROWN_GOVERNOR] = rb.GetRawDataResource(IDR_PRESET_DANMAKU_CROWN_GOVERNOR).as_string();
    asset_map_[DAMAKU_CROWN_UNLIT] = rb.GetRawDataResource(IDR_PRESET_DANMAKU_CROWN_UNLIT).as_string();
    asset_map_[DAMAKU_ICON_CAPTAIN] = rb.GetRawDataResource(IDR_PRESET_DANMAKU_ICON_CAPTAIN).as_string();
    asset_map_[DAMAKU_ICON_ADMIRAL] = rb.GetRawDataResource(IDR_PRESET_DANMAKU_ICON_ADMIRAL).as_string();
    asset_map_[DAMAKU_ICON_GOVERNOR] = rb.GetRawDataResource(IDR_PRESET_DANMAKU_ICON_GOVERNOR).as_string();
    asset_map_[DAMAKU_ICON_THOUSND_CAPTAIN] = rb.GetRawDataResource(IDR_PRESET_DANMAKU_ICON_TH_CAPTAIN).as_string();
    asset_map_[DAMAKU_ICON_THOUSND_ADMIRAL] = rb.GetRawDataResource(IDR_PRESET_DANMAKU_ICON_TH_ADMIRAL).as_string();
    asset_map_[DAMAKU_ICON_THOUSND_GOVERNOR] = rb.GetRawDataResource(IDR_PRESET_DANMAKU_ICON_TH_GOVERNOR).as_string();
    asset_map_[DAMAKU_ICON_VOICE_LINK_GO_CHECK] = rb.GetRawDataResource(IDR_LIVEHIME_V3_VOICE_LINK_R_ARROW).as_string();

    asset_map_[DAMAKU_ICON_USER_IDENTITY_TAG_ADMIN] = rb.GetRawDataResource(IDR_VIDDUP_DANMAKU_USER_IDENTITY_TAG_ADMIN).as_string();
    asset_map_[DAMAKU_ICON_USER_IDENTITY_TAG_TOP1] = rb.GetRawDataResource(IDR_VIDDUP_DANMAKU_USER_IDENTITY_TAG_TOP1).as_string();
    asset_map_[DAMAKU_ICON_USER_IDENTITY_TAG_TOP2] = rb.GetRawDataResource(IDR_VIDDUP_DANMAKU_USER_IDENTITY_TAG_TOP2).as_string();
    asset_map_[DAMAKU_ICON_USER_IDENTITY_TAG_TOP3] = rb.GetRawDataResource(IDR_VIDDUP_DANMAKU_USER_IDENTITY_TAG_TOP3).as_string();
    asset_map_[DAMAKU_ICON_WARING] = rb.GetRawDataResource(IDR_VIDDUP_DANMAKU_WARNING).as_string();

    //点赞图标带替换
    like_icon_data_ = rb.GetRawDataResource(IDR_LIVEHIME_DANMAKU_LIKE_ICON).as_string();
}

void ImageFetcher::WriteFileToDisk(
    const secret::DanmakuHimeService::GiftConfigInfo& info,
    ImageType type,
    const std::string& data)
{
    auto dir_path = bililive::GetBililiveUserDataDirectory();
    if (dir_path.empty()) {
        return;
    }

    dir_path = dir_path.Append(L"Cache").Append(L"Gift Icons");
    if (!base::PathExists(dir_path)) {
        if (!file_util::CreateDirectoryW(dir_path)) {
            return;
        }
    }

    auto file_name = std::to_wstring(info.id);
    if (type == ImageType::ICON)
    {
        file_name.append(L"i");
    }
    else
    {
        file_name.append(L"g");
    }
    file_name.append(L".tmp");


    if (file_util::WriteFile(dir_path.Append(file_name), data.data(), data.size()) != -1) {
        // 图标保存成功，则修改记录
        gift_icon_recorder_.put(info.id, { info.id, info.icon_url,info.gif_url });
    }
}

void ImageFetcher::ReadFileFromDisk(
    const secret::DanmakuHimeService::GiftConfigInfo& info,
    ImageType type,
    std::string* data)
{
    auto dir_path = bililive::GetBililiveUserDataDirectory();
    if (dir_path.empty()) {
        return;
    }

    auto file_name = std::to_wstring(info.id);
    if (type == ImageType::ICON)
    {
        file_name.append(L"i");
    }
    else
    {
        file_name.append(L"g");
    }
    file_name.append(L".tmp");

    auto icon_path = dir_path.Append(L"Cache").Append(L"Gift Icons").Append(file_name);
    if (base::PathExists(icon_path)) {
        // 和记录中的 url 对照，如果不一致，说明图标变了;
        // 如果不存在对应记录，则应重新下载图标。
        std::string rec_url;

        livehime::GiftIconRecorder::GiftIconData icon_data{ info.id, info.gif_url};
        if (!gift_icon_recorder_.get(info.id, &icon_data) ||
            icon_data.icon_url != info.icon_url ||
            icon_data.gif_url != info.gif_url)
        {
            base::DeleteFile(icon_path, false);
            return;
        }

        std::string content;
        if (file_util::ReadFileToString(icon_path, &content) && !content.empty()) {
            *data = content;
        }
    }
}

void ImageFetcher::WriteFileToDisk(const ImageData& info, const std::string& data)
{
    auto dir_path = bililive::GetBililiveUserDataDirectory();
    if (dir_path.empty()) {
        return;
    }

    dir_path = dir_path.Append(L"Cache").Append(L"emoji");
    if (!base::PathExists(dir_path)) {
        if (!file_util::CreateDirectoryW(dir_path)) {
            return;
        }
    }
    auto file_name = base::UTF8ToUTF16(info.id);
    if (info.type == ImageType::ICON){
        file_name.append(L"i");
    }
    else if (info.type == ImageType::GIF) {
        file_name.append(L"g");
    }
    else if (info.type == ImageType::PNG){
        file_name.append(L"p");
    }
    file_name.append(L".tmp");

}

void ImageFetcher::ReadFileFromDisk(const ImageData& info, std::string* data)
{
    auto dir_path = bililive::GetBililiveUserDataDirectory();
    if (dir_path.empty()) {
        return;
    }

    auto file_name = base::UTF8ToUTF16(info.id);
    if (info.type == ImageType::ICON) {
        file_name.append(L"i");
    }
    else if (info.type == ImageType::GIF) {
        file_name.append(L"g");
    }
    else if (info.type == ImageType::PNG) {
        file_name.append(L"p");
    }
    file_name.append(L".tmp");

    auto image_path = dir_path.Append(L"Cache").Append(L"emoji").Append(file_name);
    if (base::PathExists(image_path)) {
        // 和记录中的 url 对照，如果不一致，说明图标变了;
        // 如果不存在对应记录，则应重新下载图标。
        livehime::ImageRecorder::ImageData image_data{};
        if (image_data.url != info.url)
        {
            base::DeleteFile(image_path, false);
            return;
        }

        std::string content;
        if (file_util::ReadFileToString(image_path, &content) && !content.empty()) {
            *data = content;
        }
    }
}

bool ImageFetcher::FileIsExistsFromDisk(const ImageData& info)
{
    auto dir_path = bililive::GetBililiveUserDataDirectory();
    if (dir_path.empty()) {
        return false;
    }

    auto file_name = base::UTF8ToUTF16(info.id);
    if (info.type == ImageType::ICON) {
        file_name.append(L"i");
    }
    else if (info.type == ImageType::GIF) {
        file_name.append(L"g");
    }
    else if (info.type == ImageType::PNG) {
        file_name.append(L"p");
    }
    file_name.append(L".tmp");

    auto image_path = dir_path.Append(L"Cache").Append(L"emoji").Append(file_name);
    if (base::PathExists(image_path)) {
        // 和记录中的 url 对照，如果不一致，说明图标变了;
        // 如果不存在对应记录，则应重新下载图标。
        livehime::ImageRecorder::ImageData image_data{};
        if (image_data.url != info.url)
        {
            base::DeleteFile(image_path, false);
            return false;
        }
        else {
            return true;
        }
    }
    return false;
}

bool ImageFetcher::FileIsExistsFromDisk(const ImageData& info,
    const base::FilePath& dir_path, livehime::ImageRecorder& recorder)
{
    std::string icon_name = GetUrlImageFileName(info.url);
    if (!dir_path.empty() && !icon_name.empty()) {
        base::FilePath file_path;

        if (info.type == ImageType::ICON) {
            icon_name.append("i");
        }
        else if (info.type == ImageType::GIF) {
            icon_name.append("g");
        }
        else if (info.type == ImageType::PNG) {
            icon_name.append("p");
        }
        file_path = dir_path.Append(base::UTF8ToUTF16(icon_name).append(L".tmp"));
        if (base::PathExists(file_path)) {
            livehime::ImageRecorder::ImageData image_data{};
            if (!recorder.get(info.id, &image_data) ||
                image_data.url != info.url)
            {
                base::DeleteFile(file_path, false);
                return false;
            }
            else {
                return true;
            }
        }
    }
    return false;
}

void ImageFetcher::DownLoadFinish()
{
    LOG(INFO) << "DownLoadFinish";
    download_thread_stop_ = true;
}

std::string ImageFetcher::GetUrlImageFileName(const std::string& url)
{
    auto pos = url.rfind('/');
    if (pos == std::string::npos)
    {
        return "";
    }

    int sub_pos = pos + 1;
    int sub_len = url.size() - pos - 1;

    if (static_cast<size_t>(sub_pos + sub_len) > url.size())
    {
        return "";
    }

    std::string icon_name = url.substr(sub_pos, sub_len);

    pos = icon_name.rfind(".");
    if (pos != std::string::npos)
    {
        icon_name = icon_name.substr(0, pos);
    }

    return icon_name;
}

//核心付费用户弹幕消息进场标签相关
base::FilePath GetCoreUserLocalTagPath()
{
	base::FilePath dir_path = bililive::GetBililiveUserDataDirectory();
	dir_path = dir_path.Append(L"Cache").Append(L"CoreUserTag");
    return dir_path;
}

void GetCoreUserLocalTagInfoList(std::map<int,CoreUserTagInfo>& core_user_tag_info)
{
    base::FilePath dir_path = GetCoreUserLocalTagPath();
    base::FilePath config_path = dir_path.Append(L"info.config");
    base::ThreadRestrictions::ScopedAllowIO allow;
	if (base::PathExists(config_path))
	{
        std::string config_data;
        file_util::ReadFileToString(config_path, &config_data);
        if (!config_data.empty())
        {
			std::unique_ptr<base::Value> core_user_tag_info_value(base::JSONReader::Read(config_data));
			const base::ListValue* core_user_tag_info_list = nullptr;
			if (core_user_tag_info_value && core_user_tag_info_value->GetAsList(&core_user_tag_info_list) && core_user_tag_info_list)
			{
				for (size_t i = 0; i < core_user_tag_info_list->GetSize(); i++)
				{
					const base::Value* tag_item_value = nullptr;
					const base::DictionaryValue* tag_item_dic = nullptr;
					if (core_user_tag_info_list->Get(i, &tag_item_value) && tag_item_value
						&& tag_item_value->GetAsDictionary(&tag_item_dic) && tag_item_dic)
					{
						CoreUserTagInfo info;
						tag_item_dic->GetInteger("tag_id", &info.tag_id);
						tag_item_dic->GetString("tag_white_bg_url", &info.tag_white_bg_url);
						tag_item_dic->GetString("tag_dark_bg_url", &info.tag_dark_bg_url);
                        tag_item_dic->GetString("tag_white_bg_local_path", &info.tag_white_bg_local_path);
                        tag_item_dic->GetString("tag_dark_bg_local_path", &info.tag_dark_bg_local_path);
                        core_user_tag_info[info.tag_id] = info;
					}
				}
			}
        }	
	}
}

void WriteCoreUserTagInfoToLocal(const std::map<int, CoreUserTagInfo>& core_user_tag_info)
{
    std::unique_ptr<base::ListValue> tag_list(new base::ListValue());
    auto it = core_user_tag_info.begin();
    for (it;it != core_user_tag_info.end();it++)
    {
        std::unique_ptr<base::DictionaryValue> item(new base::DictionaryValue());
        item->SetInteger("tag_id",it->first);
        item->SetString("tag_white_bg_url", it->second.tag_white_bg_url);
        item->SetString("tag_dark_bg_url", it->second.tag_dark_bg_url);
        item->SetString("tag_white_bg_local_path", it->second.tag_white_bg_local_path);
        item->SetString("tag_dark_bg_local_path", it->second.tag_dark_bg_local_path);
        tag_list->Append(item.release());
    }
	std::string json_string;
	base::JSONWriter::Write(tag_list.get(), &json_string);
	base::ThreadRestrictions::ScopedAllowIO allow;
	int data_size = static_cast<int>(json_string.size());

	base::FilePath dir_path = GetCoreUserLocalTagPath();
	base::FilePath config_path = dir_path.Append(L"info.config");

    int write_size = file_util::WriteFile(config_path, json_string.data(), data_size);
}