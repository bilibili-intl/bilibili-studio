/*
 @ 0xCCCCCCCC
*/

#include "bililive/bililive/livehime/obs/source_creator.h"

#include <string>

#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"

#include "ui/base/resource/resource_bundle.h"

#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/livehime/obs/srcprop_ui_util.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"

#include "obs/obs_proxy/public/proxy/obs_scene_collection.h"

#include "grit/generated_resources.h"

namespace {

using bililive::ExtraCreationData;
using bililive::DropFileData;
using bililive::DropTextData;
using bililive::SourceType;
using obs_proxy::SceneItemType;

std::wstring LocalStr(int res_id)
{
    return ResourceBundle::GetSharedInstance().GetLocalizedString(res_id);
}

struct SourceItemMap {
    SourceType source_type;
    SceneItemType scene_item_type;
    std::string name_prefix;

    SourceItemMap(SourceType source_type, SceneItemType item_type, const std::wstring& prefix)
        : source_type(source_type), scene_item_type(item_type), name_prefix(base::WideToUTF8(prefix))
    {}
};

const SourceItemMap& MapSourceItem(SourceType type)
{
    const static SourceItemMap kSourceItemTable[] {
        {SourceType::Camera, SceneItemType::VideoCaptureDevice, LocalStr(IDS_TOOLBAR_CAMERA)},
        {SourceType::Game, SceneItemType::GameCapture, LocalStr(IDS_TOOLBAR_GAME)},
        {SourceType::Monitor, SceneItemType::DisplayCapture, LocalStr(IDS_TOOLBAR_SCREEN_MONITOR)},
        {SourceType::Window, SceneItemType::WindowCapture, LocalStr(IDS_TOOLBAR_SCREEN_WINDOW)},
        {SourceType::Media, SceneItemType::MediaSource, LocalStr(IDS_TOOLBAR_VIDEO)},
        {SourceType::Text, SceneItemType::Text, LocalStr(IDS_TOOLBAR_TEXT)},
        {SourceType::Image, SceneItemType::Image, LocalStr(IDS_TOOLBAR_PICTURE)},
        {SourceType::Album, SceneItemType::Slider, LocalStr(IDS_TOOLBAR_ALBUM)},
        {SourceType::Color, SceneItemType::ColorSource, LocalStr(IDS_TOOLBAR_COLORSOURCE)},
        {SourceType::Receiver, SceneItemType::ReceiverSource, LocalStr(IDS_TOOLBAR_SCREEN_PROJECTION)},
        {SourceType::Browser, SceneItemType::BrowserSource, LocalStr(IDS_TOOLBAR_BROWSER)},
        {SourceType::AudioInput, SceneItemType::AudioInputCapture, LocalStr(IDS_TOOLBAR_AUDIO_DEVICE_INPUT)},
        {SourceType::AudioOutput, SceneItemType::AudioOutputCapture, LocalStr(IDS_TOOLBAR_AUDIO_DEVICE_OUTPUT)},
        {SourceType::DmkHime, SceneItemType::DmkhimeSource, LocalStr(IDS_TOOLBAR_DANMAKU_HIME)},
        {SourceType::TextureRenderer, SceneItemType::TextureRendererSource, LocalStr(IDS_TOOLBAR_TEXTURE)},
    };

    auto it = std::find_if(std::begin(kSourceItemTable), std::end(kSourceItemTable),
                           [type](const SourceItemMap& item) {
                               return item.source_type == type;
                           });
    CHECK(it != std::end(kSourceItemTable));
    return *it;
}

void ApplyData(obs_proxy::SceneItem* scene_item, const DropFileData* data)
{
    switch (scene_item->type()) {
        case SceneItemType::Text: {
            TextSceneItemHelper helper(scene_item);
            helper.IsFromFile(true);
            helper.TextFile(data->file_path.value());
            helper.Update();
        }
            break;

        case SceneItemType::Image: {
            ImageSceneItemHelper helper(scene_item);
            helper.FilePath(data->file_path.value());
            helper.Update();
        }
            break;

        case SceneItemType::MediaSource: {
            MediaSceneItemHelper helper(scene_item);
            helper.FilePath(data->file_path.value());
            helper.Update();
        }
            break;

        default:
            NOTREACHED();
            break;
    }
}

void ApplyData(obs_proxy::SceneItem* scene_item, const bililive::ProjectionData* data)
{
	DCHECK(scene_item->type() == SceneItemType::ReceiverSource);

    ReceiverItemHelper helper(scene_item);
   
    helper.SetRecvType((ReceiverItemHelper::ReceiverType)data->projection_type);
	helper.Update();
}



void ApplyData(obs_proxy::SceneItem* scene_item, const bililive::MonitorCaptureData* data)
{
    DCHECK(scene_item->type() == SceneItemType::DisplayCapture);

    MonitorSceneItemHelper helper(scene_item);
    // 对显示器的枚举采用OBS底层的方式，以便索引值的对应
    bool matched = false;
    int monitor_idx = 0;
    struct gs_monitor_info info;
    obs_enter_graphics();
    for (monitor_idx = 0; true; ++monitor_idx)
    {
        if (gs_get_duplicator_monitor_info(monitor_idx, &info))
        {
            gfx::Rect monitor_bounds(info.x, info.y, info.cx, info.cy);
            if (data->display.bounds() == monitor_bounds)
            {
                matched = true;
                break;
            }
        }
        else
        {
            break;
        }
    }
    obs_leave_graphics();

    DCHECK(matched);

    helper.SelectedMonitor(monitor_idx);
    helper.Update();

    obs_proxy::CropInfo crop_info;
    crop_info.x = data->capture_bounds.x() - data->display.bounds().x();
    crop_info.y = data->capture_bounds.y() - data->display.bounds().y();
    crop_info.w = data->capture_bounds.width();
    crop_info.h = data->capture_bounds.height();
    obs_proxy::BoundsInfo bounds_info;
    bounds_info.zero();
    bounds_info.w = data->display.bounds().width();
    bounds_info.h = data->display.bounds().height();
    scene_item->Crop(crop_info, &bounds_info);
}

void ApplyData(obs_proxy::SceneItem* scene_item, const bililive::BrowserPluginData* data)
{
    DCHECK(scene_item->type() == SceneItemType::BrowserSource);

    BrowserItemHelper helper(scene_item);
    helper.SetUrl(data->url);
    helper.SetPluginSetUrl(data->set_url);
    helper.SetPluginId(data->plugin_id);
    if (data->plugin_id > 0) {
        helper.SetIsPlugin(true);
    }
    helper.Update();
}

void ApplyExtraCreationData(obs_proxy::SceneItem* scene_item, const ExtraCreationData* data)
{
    DCHECK(scene_item);
    DCHECK(data);

    switch (data->data_type) {
        case ExtraCreationData::DropFile:
            ApplyData(scene_item, static_cast<const DropFileData*>(data));
            break;

        case ExtraCreationData::DropText:
            
            break;

        case ExtraCreationData::Monitor:
            // 显示器捕获参数的应用不放在这里做，放在源已然添加到场景后在其对应的sceneitem上做
            ApplyData(scene_item, static_cast<const bililive::MonitorCaptureData*>(data));
            break;
        case ExtraCreationData::Projection:
			ApplyData(scene_item, static_cast<const bililive::ProjectionData*>(data));
			break;
        case ExtraCreationData::BrowserPlugin:
            ApplyData(scene_item, static_cast<const bililive::BrowserPluginData*>(data));
            break;
        default:
            NOTREACHED();
            break;
    }
}

}   // namespace

namespace bililive {

std::string NextAvailableName(const std::string& name_prefix, const obs_proxy::SceneCollection* scene_collec)
{
    auto scenes = scene_collec->GetScenes();

    for (int i = 1;; ++i)
    {
        auto probe_name = base::StringPrintf("%s %d", name_prefix.c_str(), i);
        bool fresh = std::all_of(scenes.cbegin(), scenes.cend(),
            [&probe_name](const obs_proxy::Scene* scene)
            {
                return scene->GetItem(probe_name) == nullptr;
            });
        if (fresh)
        {
            return probe_name;
        }
    }
}

std::string NextAvailableSceneName(const std::string& name_prefix, const obs_proxy::SceneCollection* scene_collec)
{
    auto scenes = scene_collec->GetScenes();

    for (int i = 1;; ++i)
    {
        auto probe_name = base::StringPrintf("%s %d", name_prefix.c_str(), i);
        bool found = false;
        for (auto scene : scenes)
        {
            if (probe_name == scene->name())
            {
                found = true;
            }
        }
        if (!found)
        {
            return probe_name;
        }
    }
}

ExtraCreationData::ExtraCreationData(Type type)
    : data_type(type), silence_mode(false)
{}

DropFileData::DropFileData(const base::FilePath& path)
    : ExtraCreationData(Type::DropFile), file_path(path)
{}

DropTextData::DropTextData(const std::wstring& text)
    : ExtraCreationData(Type::DropText), text(text)
{}

MonitorCaptureData::MonitorCaptureData(const gfx::Display& dsp, const gfx::Rect& rect)
    : ExtraCreationData(Type::Monitor)
    , display(dsp)
    , capture_bounds(rect)
{}

ProjectionData::ProjectionData(int type)
    :ExtraCreationData(Type::Projection),
    projection_type(type)
{


}

BrowserPluginData::BrowserPluginData(int64_t id, const std::string& url, const std::string& name,
    const std::string& set_url,bool free)
    : ExtraCreationData(Type::BrowserPlugin),
    plugin_id(id),
    url(url),
    plugin_name(name),
    set_url(set_url),
    is_free(free)
{
}

CreatingSourceParams::CreatingSourceParams(SourceType type, const ExtraCreationData* extra_data)
    : type(type), extra_data(extra_data)
{}

obs_proxy::SceneItem* CreateSource(const CreatingSourceParams& params)
{
    auto scene_collection = OBSProxyService::GetInstance().GetOBSCoreProxy()->GetCurrentSceneCollection();
    return CreateSourceAt(scene_collection->current_scene(), params);
}

obs_proxy::SceneItem* CreateSourceAt(
    obs_proxy::Scene* scene, const CreatingSourceParams& params)
{
    auto scene_collection = OBSProxyService::GetInstance().GetOBSCoreProxy()->GetCurrentSceneCollection();
    auto main_preview = OBSProxyService::GetInstance().GetBililiveOBSView();

    auto item_map = MapSourceItem(params.type);

    std::string name_prefix;
    if (params.name_prefix.empty()) {
        name_prefix = item_map.name_prefix;
    } else {
        name_prefix = params.name_prefix;
    }

    auto source_name = NextAvailableName(name_prefix, scene_collection);

    // Preview area must be aware of the source adding behavior.
    // Maybe we should just refactor preview logics later to get rid of this weird workaround.
    auto scene_item = scene->AttachNewItem(item_map.scene_item_type,
        source_name,
        true);
    DCHECK(scene_item);
    scene_item->SetVisible(false);

    if (params.extra_data) {
        ApplyExtraCreationData(scene_item, params.extra_data);
    }

    return scene_item;
}

}   // namespace bililive
