#include "preset_material_ui_presenter.h"

#include "base/ext/bind_lambda.h"
#include "base/file_util.h"
#include "base/json/json_reader.h"
#include "base/json/json_file_value_serializer.h"
#include "base/memory/singleton.h"
#include "base/strings/utf_string_conversions.h"
#include "base/path_service.h"
#include "base/prefs/pref_service.h"

#include "bililive/bililive/bililive_thread_impl.h"
#include "bililive/bililive/command_updater_delegate.h"
#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/livehime/obs/source_creator.h"
#include "bililive/bililive/livehime/sources_properties/source_image_property_contract.h"
#include "bililive/bililive/livehime/sources_properties/source_image_property_presenter_impl.h"
#include "bililive/bililive/ui/bililive_command_handler_livehime.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/utils/bililive_image_util.h"
#include "bililive/bililive/utils/file_util.h"
#include "bililive/bililive/utils/image_util.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/common/bililive_paths.h"
#include "ui/base/resource/resource_bundle.h"

#include "obs/obs_proxy/core_proxy/scene_collection/obs_sources_wrapper_impl.h"
#include "obs/obs_proxy/public/proxy/obs_scene_collection.h"

#include <shlwapi.h>


std::string  PresetMaterailUIPresenter::BK_INFO_STRING =
"{\
\"horiz\" : [\"./preset_material/bg_horiz0.png\", \"./preset_material/bg_horiz1.png\", \"./preset_material/bg_horiz2.png\", \"./preset_material/bg_horiz3.png\", \"./preset_material/bg_horiz4.png\", \"./preset_material/bg_horiz5.png\" ],\
\"vert\" : [\"./preset_material/bg_vert0.png\", \"./preset_material/bg_vert1.png\", \"./preset_material/bg_vert2.png\", \"./preset_material/bg_vert3.png\", \"./preset_material/bg_vert4.png\", \"./preset_material/bg_vert5.png\"]\
}\
";

std::unique_ptr<base::Value> PresetMaterailUIPresenter::BK_INFO_ROOT_VALUE(base::JSONReader::Read(BK_INFO_STRING));

const base::string16 PresetMaterailUIPresenter::DEFAULT_BK_INFO_FILE_NAME = L"info.json";

const std::vector<base::string16> PresetMaterailUIPresenter::VEC_DEFAULT_HORIZ_BK_IMG_PATH =
{
        L"bg_horiz0.png", L"bg_horiz1.png", L"bg_horiz2.png",
        L"bg_horiz3.png", L"bg_horiz4.png", L"bg_horiz5.png"
};

const std::vector<base::string16> PresetMaterailUIPresenter::VEC_DEFAULT_VERT_BK_IMG_PATH =
{
        L"bg_vert0.png", L"bg_vert1.png", L"bg_vert2.png",
        L"bg_vert3.png", L"bg_vert4.png", L"bg_vert5.png"
};

class PresetMatrialDownloadCallback    
{
public:
    PresetMatrialDownloadCallback(
        const base::WeakPtr<PresetMaterailUIPresenter>& persenter,
        base::FilePath download_file_path,
        base::FilePath user_material_file_path,
        bool is_horiz,
        int index) :
        persenter_(persenter),
        download_file_path_(download_file_path),
        user_material_file_path_(user_material_file_path),
        index_(index),
        is_horiz_(is_horiz)
    {
    }

    ~PresetMatrialDownloadCallback()
    {
    }

private:
    base::WeakPtr<PresetMaterailUIPresenter> persenter_;
    base::FilePath download_file_path_;
    base::FilePath user_material_file_path_;
    bool is_horiz_ = false;
    int index_ = -1;

    void PostDeleteDownloaderTask()
    {

    }

    void PostUpdateBkCompletedTask()
    {
        PresetMaterailUIPresenter::UpdateBkInfoJsonValue(is_horiz_, index_, user_material_file_path_.AsUTF8Unsafe());

        auto image = bililive::decodeImgFromFile(user_material_file_path_);
        if (image.isNull())
        {
            LOG(INFO) << "PresetMatrialDownloadCallback : decodeImgFromFile failed, file path:" << user_material_file_path_.AsUTF8Unsafe();
            DCHECK(0);
        }

        auto closure = base::Bind(
            &PresetMaterailUIPresenter::UpdateBackgroundImageKVCompleted,
            persenter_,
            is_horiz_,
            index_,
            std::move(image),
            std::move(user_material_file_path_));
        BililiveThread::PostTask(BililiveThread::UI, FROM_HERE, closure);
    }
};

PresetMaterailUIPresenter::PresetMaterailUIPresenter() :
    weak_ptr_factory_(this)
{
}

PresetMaterailUIPresenter::~PresetMaterailUIPresenter()
{
}

PresetMaterailUIPresenter* PresetMaterailUIPresenter::GetInstance()
{
    return Singleton<PresetMaterailUIPresenter>::get();
}

void PresetMaterailUIPresenter::UpdateBackgroundImageKV(const std::string& kv_json)
{
    std::unique_ptr<base::Value> attribute_value(base::JSONReader::Read(kv_json));

    base::DictionaryValue* obj{};
    if (attribute_value->GetAsDictionary(&obj) && obj)
    {
        base::ListValue* url_list{};
        int index{};
        std::string url;
        if (obj->GetList("horiz", &url_list) && url_list)
        {
            for (auto &value : *url_list)
            {
                if (value->GetAsString(&url))
                {
                    auto closure = base::Bind(&PresetMaterailUIPresenter::UpdateBackgroundImageKVClosure, weak_ptr_factory_.GetWeakPtr(), url, true, index);
                    BililiveThread::PostTask(BililiveThread::FILE, FROM_HERE, closure);
                    download_count_++;
                }

                index++;
            }
        }

        index = 0;
        if (obj->GetList("vert", &url_list) && url_list)
        {
            for (auto &value : *url_list)
            {
                if (value->GetAsString(&url))
                {
                    auto closure = base::Bind(&PresetMaterailUIPresenter::UpdateBackgroundImageKVClosure, weak_ptr_factory_.GetWeakPtr(), url, false, index);
                    BililiveThread::PostTask(BililiveThread::FILE, FROM_HERE, closure);
                    download_count_++;
                }

                index++;
            }
        }
    }
}

//UI线程调用，获取背景信息
BkImageInfo PresetMaterailUIPresenter::FetchBgInfo(bool is_horiz, int index)
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    auto& vec_bk_img_info = is_horiz ? horiz_vec_bk_img_info_ : vert_vec_bk_img_info_;

    if (index < 0 || index >= static_cast<int>(vec_bk_img_info.size()))
    {
        return {};
    }

    return vec_bk_img_info[index];
}

int PresetMaterailUIPresenter::GetBgInfoCount(bool is_horiz)
{
    auto& vec_bg_info = is_horiz ? horiz_vec_bk_img_info_ : vert_vec_bk_img_info_;
    return vec_bg_info.size();
}

void PresetMaterailUIPresenter::InitializedDefaultBkInfoClosure(base::WeakPtr<PresetMaterailUIPresenter> weak_ptr, const base::string16& json_file)
{
    //加载默认背景资源
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::FILE));

    std::vector<BkImageInfo> horiz_vec_bk_img_info;
    std::vector<BkImageInfo> vert_vec_bk_img_info;

    base::ListValue* horiz_file_name_list{};
    base::ListValue* vert_file_name_list{};
    base::Closure closure;
    std::vector<char> json_data;

    base::FilePath default_material_path;
    auto ret = PathService::Get(bililive::DIR_BILI_PRESET_MATERIAL, &default_material_path);
    if (!ret)
    {
        horiz_vec_bk_img_info.clear();
        vert_vec_bk_img_info.clear();
        return;
    }

    json_data = bililive::GetBinaryFromFile(base::FilePath(json_file));
    base::Value* json_value = nullptr;
    if (!json_data.empty())
    {
		//为了安全加上/0
		json_data.push_back(0);
        json_value = base::JSONReader::Read(BK_INFO_STRING);
    }

    if (json_value)
    {
		BK_INFO_STRING = json_data.data();
		BK_INFO_ROOT_VALUE.reset(base::JSONReader::Read(BK_INFO_STRING));
        LoadJsonInfoBkInfo(BK_INFO_ROOT_VALUE.get(), horiz_vec_bk_img_info, vert_vec_bk_img_info);
    }
    else
    {
        LoadDefaultBkInfo(horiz_vec_bk_img_info, vert_vec_bk_img_info);
    }

    SaveBkInfoJson();

    closure = base::Bind(&PresetMaterailUIPresenter::InitializeDefaultBkInfoCompleted, weak_ptr, std::move(horiz_vec_bk_img_info), std::move(vert_vec_bk_img_info));
    BililiveThread::PostTask(BililiveThread::UI, FROM_HERE, closure);
}

bool PresetMaterailUIPresenter::GetDefaultFilePathImage(bool is_horiz, int index, base::FilePath& p_file_path, gfx::ImageSkia& p_img)
{
    const auto& vec_bk_info = is_horiz ? VEC_DEFAULT_HORIZ_BK_IMG_PATH : VEC_DEFAULT_VERT_BK_IMG_PATH;

    auto&& file_path = GetDefaultBkFilePath(is_horiz, index);
    auto&& image = bililive::decodeImgFromFile(file_path);

    if (!image.isNull())
    {
        p_file_path = std::move(file_path);
        p_img = std::move(image);
        return true;
    }
    else
    {
        return false;
    }
}

void PresetMaterailUIPresenter::LoadJsonInfoBkInfo(const base::Value* json_root, std::vector<BkImageInfo>& vec_horiz, std::vector<BkImageInfo>& vec_vert)
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::FILE));

    BkImageInfo bk_image;
    gfx::ImageSkia bg_img;
    base::string16 file_path;
    base::FilePath img_file_path;
    const base::DictionaryValue* dic_root{};
    if (json_root && json_root->GetAsDictionary(&dic_root) && dic_root)
    {
        const base::ListValue* horiz_file_name_list{};
        dic_root->GetList("horiz", &horiz_file_name_list);
        if (!horiz_file_name_list)
        {
            return;
        }

        const base::ListValue* vert_file_name_list{};
        dic_root->GetList("vert", &vert_file_name_list);
        if (!vert_file_name_list)
        {
            return;
        }

        int index{};
        for (const auto& v : *horiz_file_name_list)
        {
            bool ret{};
            ret = LoadJsonValueFilePathImage(v, img_file_path, bg_img);
            if (!ret)
            {
                ret = GetDefaultFilePathImage(true, index, img_file_path, bg_img);
            }

            if (ret)
            {
                bk_image.is_horiz = true;
                bk_image.file_path = img_file_path;
                bk_image.bk_img = bg_img;
                vec_horiz.push_back(bk_image);
            }

            index++;
        }

        index = 0;
        for (auto& v : *vert_file_name_list)
        {
            bool ret{};
            ret = LoadJsonValueFilePathImage(v, img_file_path, bg_img);
            if (!ret)
            {
                ret = GetDefaultFilePathImage(false, index, img_file_path, bg_img);
            }

            if (ret)
            {
                bk_image.is_horiz = false;
                bk_image.file_path = img_file_path;
                bk_image.bk_img = bg_img;
                vec_vert.push_back(bk_image);
            }

            index++;
        }
    }
}

bool PresetMaterailUIPresenter::LoadJsonValueFilePathImage(const base::Value* value, base::FilePath& p_file_path, gfx::ImageSkia& p_img)
{
    base::string16 path;
    if (!value->GetAsString(&path))
    {
        return false;
    }

    base::FilePath file_path(path);
    //缩略图
    base::FilePath thumbnail_file_path = file_path.InsertBeforeExtension(L"_thumbnail");
    auto thumbnail_img = bililive::decodeImgFromFile(thumbnail_file_path);
    if (!thumbnail_img.isNull())
    {
		p_file_path = file_path;
		p_img = thumbnail_img;
		return true;
    }
    auto img = bililive::decodeImgFromFile(file_path);
    if (!img.isNull())
    {
        p_file_path = file_path;
        p_img = img;
        return true;
    }
    else
    {
        return false;
    }
}

void PresetMaterailUIPresenter::InitializeDefaultBkInfoCompleted(std::vector<BkImageInfo> horiz_bk_info, std::vector<BkImageInfo> vert_bk_info)
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    horiz_vec_bk_img_info_ = std::move(horiz_bk_info);
    vert_vec_bk_img_info_ = std::move(vert_bk_info);

    is_initialized_ = true;

    FOR_EACH_OBSERVER(
        PresetMaterialUIObserver, observer_list_,
        OnPresetMaterialInitialized());
}

void PresetMaterailUIPresenter::UpdateBackgroundImageKVClosure(base::WeakPtr<PresetMaterailUIPresenter> weak_ptr, const std::string& url, bool is_horiz, int index)
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::FILE));

    base::FilePath user_material_path;
    if (!PathService::Get(bililive::DIR_BILI_USER_MATERIAL, &user_material_path))
    {
        LOG(INFO) << "PresetMaterailUIPresenter : Get bililive::DIR_BILI_USER_MATERIAL path failed";
        return;
    }

    auto file_name = UTF8ToUTF16(bililive::GetFileName(url));
    auto user_material_file = user_material_path.Append(file_name);
    if (base::PathExists(user_material_file))
    {
        HandleBackgroundImageExist(weak_ptr, is_horiz, index, user_material_file);

        //存在同名文件不下载
        LOG(INFO) << "PresetMaterailUIPresenter : " << user_material_file.AsUTF8Unsafe() << "already exists;";
        return;
    }

    base::FilePath download_path;
    if (!PathService::Get(bililive::DIR_BILI_DOWNLOAD_CACHE, &download_path))
    {
        gfx::ImageSkia image;
        auto closure = base::Bind(
            &PresetMaterailUIPresenter::UpdateBackgroundImageKVCompleted,
            weak_ptr,
            is_horiz,
            index,
            image,
            std::move(download_path));
        BililiveThread::PostTask(BililiveThread::UI, FROM_HERE, closure);

        LOG(INFO) << "PresetMaterailUIPresenter : Get bililive::DIR_BILI_DOWNLOAD_CACHE path failed";
        return;
    }
}

void PresetMaterailUIPresenter::HandleBackgroundImageExist(base::WeakPtr<PresetMaterailUIPresenter> weak_ptr, bool is_horiz, int index, base::FilePath& file_path)
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::FILE));

    gfx::ImageSkia img = bililive::decodeImgFromFile(file_path);
    if (!img.isNull())
    {
        UpdateBkInfoJsonValue(is_horiz, index, file_path.AsUTF8Unsafe());
    }
    auto closure = base::Bind(
        &PresetMaterailUIPresenter::UpdateBackgroundImageKVCompleted,
        weak_ptr,
        is_horiz,
        index,
        img,
        std::move(file_path));
    BililiveThread::PostTask(BililiveThread::UI, FROM_HERE, closure);
}

void PresetMaterailUIPresenter::UpdateBackgroundImageKVCompleted(bool is_horiz, int index, gfx::ImageSkia image, base::FilePath file_path)
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    download_count_--;

    if (!image.isNull())
    {
        auto& vec_bk_img_info = is_horiz ? horiz_vec_bk_img_info_ : vert_vec_bk_img_info_;

        if (index < 0)
        {
            LOG(INFO) << "PresetMaterailUIPresenter : index out of range!!";
            DCHECK(0);
            return;
        }

        if (index >= static_cast<int>(vec_bk_img_info.size()))
        {
            vec_bk_img_info.resize(index + 1);
        }

       auto& bk_info = vec_bk_img_info[index];

       bk_info.is_horiz = is_horiz;
       bk_info.bk_img = image;
       bk_info.file_path = file_path;

       FOR_EACH_OBSERVER(
           PresetMaterialUIObserver, observer_list_,
           OnPresetMaterialBkUpdate(index, bk_info));
    }

    if (download_count_ == 0)
    {
        //保存json配置文件
        auto closure = base::Bind(&PresetMaterailUIPresenter::SaveBkInfoJson);
        BililiveThread::PostTask(BililiveThread::FILE, FROM_HERE, closure);
    }
}

base::FilePath PresetMaterailUIPresenter::GetBkInfoJsonPath()
{
    base::FilePath file_path;
    auto ret = PathService::Get(bililive::DIR_BILI_PRESET_MATERIAL, &file_path);
    if (!ret)
    {
        DCHECK(0);
        return base::FilePath();
    }

    file_path = file_path.Append(DEFAULT_BK_INFO_FILE_NAME);

    return file_path;
}

void PresetMaterailUIPresenter::UpdateBkInfoJsonValue(bool is_horiz, int index, const std::string& path)
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::FILE));

    if (index < 0)
    {
        DCHECK(0);
    }

    base::DictionaryValue* dic_v{};
    if (BK_INFO_ROOT_VALUE && BK_INFO_ROOT_VALUE->GetAsDictionary(&dic_v) && dic_v)
    {
        std::string key = is_horiz ? "horiz" : "vert";
        base::ListValue *list_v{};
        if (dic_v->GetList(key, &list_v) && list_v)
        {
            int list_size = list_v->GetSize();
            if (index >= list_size)
            {
                int append_count = index - list_size + 1;

                //填充空的数据
                while (append_count--)
                {
                    list_v->Append(new base::StringValue(""));
                }
            }

            list_v->Set(index, new base::StringValue(path));
        }
    }
}

void PresetMaterailUIPresenter::SaveBkInfoJson()
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::FILE));

    auto json_file_path = GetBkInfoJsonPath();

    JSONFileValueSerializer json_file(json_file_path);
    if (BK_INFO_ROOT_VALUE && !json_file.Serialize(*BK_INFO_ROOT_VALUE.get()))
    {
        DCHECK(0);
        LOG(INFO) << "PresetMaterailUIPresenter : save bk info json failed";
    }
}

base::FilePath PresetMaterailUIPresenter::GetDefaultBkFilePath(bool is_horiz, int index)
{
    if (index < 0)
    {
        return {};
    }

    base::FilePath file_path;
    if (PathService::Get(bililive::DIR_BILI_PRESET_MATERIAL, &file_path))
    {
        DCHECK(0);
        return {};
    }

    if (is_horiz)
    {
        if (index >= static_cast<int>(VEC_DEFAULT_HORIZ_BK_IMG_PATH.size()))
        {
            return{};
        }

        return file_path.Append(VEC_DEFAULT_HORIZ_BK_IMG_PATH[index]);
    }
    else
    {
        if (index >= static_cast<int>(VEC_DEFAULT_VERT_BK_IMG_PATH.size()))
        {
            return{};
        }

        return file_path.Append(VEC_DEFAULT_VERT_BK_IMG_PATH[index]);
    }
}

void PresetMaterailUIPresenter::LoadDefaultBkInfo(std::vector<BkImageInfo>& vec_horiz, std::vector<BkImageInfo>& vec_vert)
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::FILE));

    vec_horiz.resize(VEC_DEFAULT_HORIZ_BK_IMG_PATH.size());
    for (int i = 0; i < static_cast<int>(VEC_DEFAULT_HORIZ_BK_IMG_PATH.size()); i++)
    {
        auto& bg_info = vec_horiz[i];
        auto file_path = GetDefaultBkFilePath(true, i);
        auto img = bililive::decodeImgFromFile(file_path);
        if (!img.isNull())
        {
            bg_info.is_horiz = true;
            bg_info.file_path = file_path;
            bg_info.bk_img = img;
        }
    }

    vec_vert.resize(VEC_DEFAULT_VERT_BK_IMG_PATH.size());
    for (int i = 0; i < static_cast<int>(VEC_DEFAULT_VERT_BK_IMG_PATH.size()); i++)
    {
        auto& bg_info = vec_vert[i];
        auto file_path = GetDefaultBkFilePath(true, i);
        auto img = bililive::decodeImgFromFile(file_path);
        if (!img.isNull())
        {
            bg_info.is_horiz = false;
            bg_info.file_path = file_path;
            bg_info.bk_img = img;
        }
    }
}

void PresetMaterailUIPresenter::Initialize()
{
    //加载默认背景资源
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    if (is_initialized_)
    {
        return;
    }

    DCHECK(horiz_vec_bk_img_info_.empty());
    DCHECK(vert_vec_bk_img_info_.empty());

    auto json_file_path = GetBkInfoJsonPath();

    auto initialize_closure = base::Bind(&PresetMaterailUIPresenter::InitializedDefaultBkInfoClosure, weak_ptr_factory_.GetWeakPtr(), json_file_path.AsUTF16Unsafe());
    BililiveThread::PostTask(BililiveThread::FILE, FROM_HERE, initialize_closure);
}

bool PresetMaterailUIPresenter::IsInitialized()
{
    return is_initialized_;
}

void PresetMaterailUIPresenter::AddObserver(PresetMaterialUIObserver* observer, bool notify)
{
    observer_list_.AddObserver(observer);
}

void PresetMaterailUIPresenter::RemoveObserver(PresetMaterialUIObserver* observer)
{
    observer_list_.RemoveObserver(observer);
}
