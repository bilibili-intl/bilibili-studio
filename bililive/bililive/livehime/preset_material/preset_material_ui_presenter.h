#pragma once

#include "ui/gfx/image/image_skia.h"

#include "base/files/file_path.h"
#include "base/prefs/pref_registry_simple.h"
#include "base/memory/weak_ptr.h"

#include "net/url_request/url_request_context.h"

#include "bililive/public/bililive/bililive_thread.h"

struct BkImageInfo
{
    bool                is_horiz = false;
    gfx::ImageSkia      bk_img;
    base::FilePath      file_path;
};

class PresetMaterialUIObserver
{
public:
    virtual ~PresetMaterialUIObserver() = default;

    virtual void OnPresetMaterialInitialized() = 0;

    virtual void OnPresetMaterialBkUpdate(int index, const BkImageInfo& info) = 0;

protected:
    PresetMaterialUIObserver() {};
};

class PresetMaterailUIPresenter
{
    enum FileStatus
    {
        kFree = 0,
        kLock = 1
    };

    friend class PresetMatrialDownloadCallback;

public:
    PresetMaterailUIPresenter();
    ~PresetMaterailUIPresenter();

    static PresetMaterailUIPresenter* GetInstance();

    void Initialize();
    bool IsInitialized();
    void AddObserver(PresetMaterialUIObserver* observer, bool notify = true);
    void RemoveObserver(PresetMaterialUIObserver* observer);
    void UpdateBackgroundImageKV(const std::string& kv_json);

    BkImageInfo FetchBgInfo(bool is_horiz, int index);
    int GetBgInfoCount(bool is_horiz);

private:
    static void InitializedDefaultBkInfoClosure(base::WeakPtr<PresetMaterailUIPresenter> weak_ptr, const base::string16& json_file);
    static void UpdateBackgroundImageKVClosure(base::WeakPtr<PresetMaterailUIPresenter> weak_ptr, const std::string& url, bool is_horiz, int index);
    static void HandleBackgroundImageExist(base::WeakPtr<PresetMaterailUIPresenter> weak_ptr, bool is_horiz, int index, base::FilePath& file_path);
    static void LoadDefaultBkInfo(std::vector<BkImageInfo>& vec_horiz, std::vector<BkImageInfo>& vec_vert);
    static bool GetDefaultFilePathImage(bool is_horiz, int index, base::FilePath& file_path, gfx::ImageSkia& img);
    static bool LoadJsonValueFilePathImage(const base::Value* value, base::FilePath& file_path, gfx::ImageSkia& img);
    static void LoadJsonInfoBkInfo(const base::Value* json_root, std::vector<BkImageInfo>& horiz, std::vector<BkImageInfo>& vert);
    static base::FilePath GetBkInfoJsonPath();
    static void UpdateBkInfoJsonValue(bool is_horiz, int index, const std::string& path);
    static base::FilePath GetDefaultBkFilePath(bool is_horiz, int index);
    static void SaveBkInfoJson();

    void InitializeDefaultBkInfoCompleted(std::vector<BkImageInfo> horiz_bk_info, std::vector<BkImageInfo> vert_bk_info);
    void UpdateBackgroundImageKVCompleted(bool is_horiz, int index, gfx::ImageSkia image, base::FilePath file_path);

    bool is_initialized_ = false;

    int download_count_ = 0;
    static std::unique_ptr<base::Value> BK_INFO_ROOT_VALUE;
    static std::string BK_INFO_STRING;
    static const base::string16 DEFAULT_BK_INFO_FILE_NAME;

    static const std::vector<base::string16> VEC_DEFAULT_HORIZ_BK_IMG_PATH;
    static const std::vector<base::string16> VEC_DEFAULT_VERT_BK_IMG_PATH;

    std::vector<BkImageInfo> horiz_vec_bk_img_info_;
    std::vector<BkImageInfo> vert_vec_bk_img_info_;

    ObserverList<PresetMaterialUIObserver> observer_list_;
    base::WeakPtrFactory<PresetMaterailUIPresenter> weak_ptr_factory_;
};
