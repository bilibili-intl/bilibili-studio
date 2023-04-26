#include "bililive/bililive/livehime/volume/bililive_mic_record_presenter_impl.h"

#include "base/strings/utf_string_conversions.h"
#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/livehime/sources_properties/source_media_property_presenter_impl.h"
#include "obs/obs_proxy/public/proxy/obs_scene_collection.h"
#include "obs/obs_proxy/public/common/pref_constants.h"
#include "obs/obs_proxy/utils/obs_wrapper_impl_cast.h"

#include <Shlobj.h>
namespace
{
    const char kMicRecordFilterName[] = "mic_record_filter_echo_test";
    const char kMicRecordMediaSourceName[] = "echo_test_media_source";

    static bool GetLocalAppDataDirectory(std::wstring& dir_path) {
        const size_t kBufSize = MAX_PATH + 1;
        wchar_t path_buf[kBufSize]{ 0 };
        if (FAILED(SHGetFolderPath(nullptr, CSIDL_LOCAL_APPDATA, nullptr,
            SHGFP_TYPE_CURRENT, path_buf))) {
            return false;
        }
        dir_path = path_buf;
        return true;
    }

    static void CreateDirectoryIfNecessary(const std::wstring& dir_path) {
        if (!(GetFileAttributes((LPCWSTR)dir_path.c_str()) !=
            INVALID_FILE_ATTRIBUTES)) {
            CreateDirectory(dir_path.c_str(), nullptr);
        }
    }
}

std::wstring BililiveMicRecordPresenterImpl::GetBililiveEchoTestDirectory() {
    std::wstring data_dir;
    if (!GetLocalAppDataDirectory(data_dir)) {
        return L"";
    }
    data_dir = data_dir.append(L"\\bililive\\User Data\\EchoTest");
    CreateDirectoryIfNecessary(data_dir);
    return data_dir;
}

BililiveMicRecordPresenterImpl::BililiveMicRecordPresenterImpl()
{

}


BililiveMicRecordPresenterImpl::~BililiveMicRecordPresenterImpl()
{

}

bool BililiveMicRecordPresenterImpl::StartRecord()
{
    bool ret = false;

    obs_proxy::SceneCollection* scene_collection
        = OBSProxyService::GetInstance().GetOBSCoreProxy()->GetCurrentSceneCollection();
    if (scene_collection)
    {
        std::vector<obs_proxy::AudioDevice*> vct = scene_collection->GetAudioDevices();

        auto mic = std::find_if(vct.begin(), vct.end(), [](obs_proxy::AudioDevice* dev)-> bool {
            return dev->name() == obs_proxy::kDefaultInputAudio; });
        if (mic != vct.end())
        {
            obs_proxy::Filter* filter = (*mic)->GetFilter(kMicRecordFilterName);
            if (!filter)
            {
                filter = (*mic)->AddNewFilter(obs_proxy::FilterType::MicRecordFilter, kMicRecordFilterName);
            }
            if (filter)
            {
                obs_proxy::PropertyValue& f_propv = filter->GetPropertyValues();
                std::wstring path = GetBililiveEchoTestDirectory();
                if (!path.empty())
                {
                    auto scene_collection = OBSProxyService::GetInstance().GetOBSCoreProxy()->GetCurrentSceneCollection();
                    if (scene_collection)
                    {
                        obs_proxy::Scene* scene = scene_collection->current_scene();
                        if (scene)
                        {
                            auto source = scene->AttachNewItem(obs_proxy::SceneItemType::MediaSource, kMicRecordMediaSourceName, true);
                            if (source)
                            {
                                if (!media_presenter_)
                                {
                                    media_presenter_ = std::make_shared<SourceMediaPropertyPresenterImpl>(source, this);
                                }
                                if (media_presenter_)
                                {
                                    media_presenter_->Initialize();
                                    wchar_t file_path[MAX_PATH] = { 0 };
                                    HMODULE dll_module = GetModuleHandle(NULL);
                                    GetModuleFileName(dll_module, file_path, MAX_PATH);
                                    if (dll_module)
                                    {
                                        wcsrchr(file_path, '\\')[0] = '\0';
                                        wcscat_s(file_path, L"\\echo_test.mp3");
                                        media_presenter_->SetFilePath(file_path);
                                        obs_source_set_monitoring_type(impl_cast(source)->AsSource(), OBS_MONITORING_TYPE_MONITOR_ONLY);
                                        media_presenter_->Update();
                                        path += L"\\echotest.wav";
                                        f_propv.Set("record_file_path", base::UTF16ToUTF8(path));
                                        f_propv.Set("is_start", true);
                                        f_propv.Set("is_end", false);
                                        filter->UpdatePropertyValues();
                                        ret = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return ret;
}

bool BililiveMicRecordPresenterImpl::StopRecord()
{
    bool ret = false;

    obs_proxy::SceneCollection* scene_collection
        = OBSProxyService::GetInstance().GetOBSCoreProxy()->GetCurrentSceneCollection();
    if (scene_collection)
    {
        std::vector<obs_proxy::AudioDevice*> vct = scene_collection->GetAudioDevices();

        auto mic = std::find_if(vct.begin(), vct.end(), [](obs_proxy::AudioDevice* dev)-> bool {
            return dev->name() == obs_proxy::kDefaultInputAudio; });
        if (mic != vct.end())
        {
            obs_proxy::Filter* filter = (*mic)->GetFilter(kMicRecordFilterName);
            if (!filter)
            {
                filter = (*mic)->AddNewFilter(obs_proxy::FilterType::MicRecordFilter, kMicRecordFilterName);
            }
            if (filter)
            {
                obs_proxy::PropertyValue& f_propv = filter->GetPropertyValues();
                f_propv.Set("record_file_path", "");
                f_propv.Set("is_start", false);
                f_propv.Set("is_end", true);
                filter->UpdatePropertyValues();

                auto scene_collection = OBSProxyService::GetInstance().GetOBSCoreProxy()->GetCurrentSceneCollection();
                if (scene_collection)
                {
                    obs_proxy::Scene* scene = scene_collection->current_scene();
                    if (scene)
                    {
                        scene->RemoveItem(kMicRecordMediaSourceName);
                        media_presenter_.reset();
                    }
                }
                (*mic)->RemoveFilter(kMicRecordFilterName);
                ret = true;
            }
        }
    }

    return ret;
}

bool BililiveMicRecordPresenterImpl::Replay()
{
    bool ret = false;
    std::wstring path = GetBililiveEchoTestDirectory();
    if (!path.empty())
    {
        auto echo_test_source = obs_get_source_by_name(kMicRecordMediaSourceName);
        if (!echo_test_source)
        {
            auto scene_collection = OBSProxyService::GetInstance().GetOBSCoreProxy()->GetCurrentSceneCollection();
            if (scene_collection)
            {
                obs_proxy::Scene* scene = scene_collection->current_scene();
                if (scene)
                {
                    auto source = scene->AttachNewItem(obs_proxy::SceneItemType::MediaSource, kMicRecordMediaSourceName, true);
                    if (source)
                    {
                        if (!media_presenter_)
                        {
                            media_presenter_ = std::make_shared<SourceMediaPropertyPresenterImpl>(source, this);
                        }
                        if (media_presenter_)
                        {
                            path += L"\\echotest.wav";
                            media_presenter_->Initialize();
                            media_presenter_->SetFilePath(path);
                            media_presenter_->SetIsLoop(false);
                            obs_source_set_monitoring_type(impl_cast(source)->AsSource(), OBS_MONITORING_TYPE_MONITOR_ONLY);
                            media_presenter_->Update();
                            ret = true;
                        }
                    }
                }
            }
        }
        else
        {
            obs_source_release(echo_test_source);
        }
    }
    return ret;
}

void BililiveMicRecordPresenterImpl::OnMediaStateChanged(contracts::MediaState state)
{
    if (state == contracts::MediaState::Ended ||
        state == contracts::MediaState::Error)
    {
        auto scene_collection = OBSProxyService::GetInstance().GetOBSCoreProxy()->GetCurrentSceneCollection();
        if (scene_collection)
        {
            obs_proxy::Scene* scene = scene_collection->current_scene();
            if (scene)
            {
                scene->RemoveItem(kMicRecordMediaSourceName);
                media_presenter_.reset();
            }
        }
    }

}