#ifndef BILILIVE_BILILIVE_LIVEHIME_GIFT_IMAGE_IMAGE_FETCHER_H_
#define BILILIVE_BILILIVE_LIVEHIME_GIFT_IMAGE_IMAGE_FETCHER_H_

#include <mutex>

#include "SkColor.h"

#include "base/containers/mru_cache.h"
#include "base/memory/singleton.h"
#include "base/strings/string_piece.h"
#include "base/threading/thread.h"
#include "base/file_util.h"

#include "bililive/bililive/livehime/danmaku_hime/danmaku_hime_contract.h"
#include "bililive/bililive/livehime/gift_image/gift_icon_recorder.h"
#include "bililive/bililive/livehime/gift_image/image_recorder.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/wic_bitmaps_read.h"

#include "ui/gfx/image/image_skia.h"


//弹幕荣誉皇冠，icon图标，gif图标，MP4配置信息，相关资源管理
class ImageFetcher {
public:
    enum Assets {
        DEF_AVATAR,
        MONTHLY_LORD,
        YEARLY_LORD,
        GOVERNOR,
        ADMIRAL,
        CAPTAIN,
        GOVERNOR_LARGE,
        ADMIRAL_LARGE,
        CAPTAIN_LARGE,
        GOVERNOR_FRAME,
        ADMIRAL_FRAME,
        CAPTAIN_FRAME,
        MEDAL_GOVERNOR,
        MEDAL_ADMIRAL,
        MEDAL_CAPTAIN,
        SILVER_MELON,
        GOLD_ANCHOR,
        SILVER_ANCHOR,
        NO_FACE,
        // 请勿修改 NUMBER_ 前缀项的顺序！
        // 若必须修改，请查找所有引用，确保不出现问题
        NUMBER_0,
        NUMBER_1,
        NUMBER_2,
        NUMBER_3,
        NUMBER_4,
        NUMBER_5,
        NUMBER_6,
        NUMBER_7,
        NUMBER_8,
        NUMBER_9,
        NUMBER_MULTIPLE,

        NUMBER_WHITE_0,
        NUMBER_WHITE_1,
        NUMBER_WHITE_2,
        NUMBER_WHITE_3,
        NUMBER_WHITE_4,
        NUMBER_WHITE_5,
        NUMBER_WHITE_6,
        NUMBER_WHITE_7,
        NUMBER_WHITE_8,
        NUMBER_WHITE_9,
        NUMBER_WHITE_MULTIPLE,
        STRING_COMBO,
        STRING_SPECIAL_COMBO,
        SUPER_CHAT,

        RANK1,
        RANK2,
        RANK3,

        HOT_RANKING,
        POPULARITY,
        POPULARITY_BLACK,
        MVP,
        VOICE_IMAGE,
        VOICE_IMAGE_WHITE,
        VOICE_PAUSE_IMG,
        VOICE_PLAY_IMG,
        VOICE_GIF,
        CMP,
        STORM,
        COMPLETION,
        ASSISTANT_RIGHT,
        RED_PACKET,
        DAMAKU_CROWN_1_4,
        DAMAKU_CROWN_5_8,
        DAMAKU_CROWN_9_12,
        DAMAKU_CROWN_13_16,
        DAMAKU_CROWN_17_20,
        DAMAKU_CROWN_21_24,
        DAMAKU_CROWN_25_28,
        DAMAKU_CROWN_29_32,
        DAMAKU_CROWN_33_36,
        DAMAKU_CROWN_37_40,
        DAMAKU_CROWN_CAPTAIN,
        DAMAKU_CROWN_GOVERNOR,
        DAMAKU_CROWN_UNLIT,
        DAMAKU_ICON_CAPTAIN,
        DAMAKU_ICON_ADMIRAL,
        DAMAKU_ICON_GOVERNOR,
        DAMAKU_ICON_THOUSND_CAPTAIN,
        DAMAKU_ICON_THOUSND_ADMIRAL,
        DAMAKU_ICON_THOUSND_GOVERNOR,
        DAMAKU_ICON_VOICE_LINK_GO_CHECK,
        DAMAKU_ICON_USER_IDENTITY_TAG_ADMIN,
        DAMAKU_ICON_USER_IDENTITY_TAG_TOP1,
        DAMAKU_ICON_USER_IDENTITY_TAG_TOP2,
        DAMAKU_ICON_USER_IDENTITY_TAG_TOP3,
        DAMAKU_ICON_WARING,
    };

    enum class ImageType { ICON = 0, GIF, PNG, WEBP };

    struct ImageInfo
    {
        ImageType type;
        std::string img_data;    //普通png的文件数据
        WICBitmaps wic_bitmaps;  //webp解码之后的缓存数据
    };
    struct ImageData {
        std::string url;
        std::string id;
        ImageType type;
    };
	struct MoreBgImageInfo
	{
		std::shared_ptr<std::string> white_bg_img_data;//白色背景标签图片数据
		std::shared_ptr<std::string> dark_bg_img_data;//深色背景标签图片数据
	};

    enum class MedalType
    {
        CROWN_NORMAL = 0,
        CROWN_GUARD,
        CROWN_UNLIT,
        GUARD,
        GUARD_THOUSAND,
    };

    struct ComboColorInfo {
        bool valid = false;
        SkColor color_start = 0;
        SkColor color_end = 0;
    };

    static ImageFetcher* GetInstance();

    bool IsGiftConfigLoaded();
    void RequestGiftConfigListSync();

    std::string FetchAsset(Assets res_id);

    int FetchGiftType(int64_t gift_id);
    int FetchGiftStayTime(int64_t gift_id);
    ComboColorInfo FetchComboColorsByResId(int res_id);
    ComboColorInfo FetchComboColorsByGiftId(int64_t gift_id);

    std::string FetcherMP4EffectURL(int64_t effect_id);

    std::string FetchGiftImageSync(int64_t gift_id);
    std::string FetchGiftDynamicImageSyncByUrl(int64_t gift_id, const std::string& url);

    std::string FetchGiftImageOnlyOnMRU(int64_t gift_id);
    std::string ImageFetcher::FetchGiftDynamicImageSync(int64_t gift_id);
    void SaveGifWicBitmaps(int64_t gift_id, const WICBitmaps& bitmaps);
    WICBitmaps FetchGifWicBitmaps(int64_t gift_id);

    std::string FetchImageSync(const std::string& url);

    void LoadLikeIcon(const std::string& url);
    std::string FetchLikeIconSync(const std::string& url);
    std::string GetUrlImageFileName(const std::string& url);

    void ThousandGuradUidChange(const std::vector<int64_t>& add_list, const std::vector<int64_t>& del_list);
    bool JuardgeThousandGuard(int64_t uid);
    int64_t GetGiftEffectID(int64_t gift_id, bool is_batch, int num);
    std::string GetGiftEffectURL(int64_t effect_id);

    gfx::ImageSkia CacheImage(const base::string16& img_path);
    gfx::ImageSkia FetchCacheImage(const base::string16& img_name);private:
    ImageFetcher();
    ~ImageFetcher();

    //bool FindGiftConfigInfoById(
    //    int64_t gift_id,
    //    secret::DanmakuHimeService::GiftConfigInfo* info);
    void WriteFileToDisk(
        const secret::DanmakuHimeService::GiftConfigInfo& info,
        ImageType type,
        const std::string& data);

    void ReadFileFromDisk(
        const secret::DanmakuHimeService::GiftConfigInfo& info,
        ImageType type,
        std::string* data);
    void LoadAssets();  

    void WriteFileToDisk(
        const ImageData& info,
        const std::string& data);

    void ReadFileFromDisk(
        const ImageData& info,
        std::string* data);

    bool FileIsExistsFromDisk(const ImageData& info);
    bool FileIsExistsFromDisk(const ImageData& info, 
        const base::FilePath& dir_path, livehime::ImageRecorder& recorder);   

    void DownLoadFinish();
  
    std::mutex assets_sync_;
    std::mutex gift_icon_sync_;
    std::mutex gif_bitmaps_sync_;
    std::mutex gift_gif_sync_;
    std::mutex gift_config_sync_;
    std::mutex url_icon_sync_;
    std::mutex title_icon_sync_;
    std::mutex effect_sync_;
    std::mutex cache_image_sync_;

    std::unique_ptr<contracts::DanmakuHimePresenter> presenter_;

    std::map<Assets, std::string> asset_map_;
    livehime::GiftIconRecorder gift_icon_recorder_;
    base::MRUCache<int64_t, std::string> icon_cache_;
    base::MRUCache<int64_t, std::string> gif_cache_;
    base::MRUCache<std::string, std::string> url_cache_;
    base::MRUCache<std::string, ImageInfo> title_cache_;
    base::MRUCache< base::string16, gfx::ImageSkia> image_cache_;  //临时使用，缓存的图片

    std::string lick_icon_md5_;
    std::string like_icon_data_;

    bool using_honor_crown_ = false;

    base::WeakPtrFactory<ImageFetcher> wpf_;
    //缓存gif wicmaps
    std::map<int64_t, WICBitmaps> gifs_bitmaps_;

    bool download_thread_stop_ = false;

    friend struct DefaultSingletonTraits<ImageFetcher>;
    DISALLOW_COPY_AND_ASSIGN(ImageFetcher);
};

#endif  // BILILIVE_BILILIVE_LIVEHIME_GIFT_IMAGE_IMAGE_FETCHER_H_