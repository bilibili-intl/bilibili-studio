/*
 @ 0xCCCCCCCC
*/

#ifndef BILILIVE_BILILIVE_LIVEHIME_OBS_SOURCE_CREATOR_H_
#define BILILIVE_BILILIVE_LIVEHIME_OBS_SOURCE_CREATOR_H_

#include "base/files/file_path.h"
#include "ui/gfx/display.h"

#include "obs/obs_proxy/public/proxy/obs_sources_wrapper.h"

namespace obs_proxy
{
    class SceneCollection;
}

namespace bililive {

std::string NextAvailableName(const std::string& name_prefix, const obs_proxy::SceneCollection* scene_collec);
std::string NextAvailableSceneName(const std::string& name_prefix, const obs_proxy::SceneCollection* scene_collec);
enum class SourceType {
    Camera,
    Game,
    Monitor,
    Window,
    Media,
    Text,
    Image,
    Album,
    Color,
    Receiver,
    Browser,
    AudioInput,
    AudioOutput,
    DmkHime,
    TextureRenderer,
};

struct ExtraCreationData {
    enum Type {
        DropFile,
        DropText,
        Monitor,
        Projection,
        BrowserPlugin,
    };

    Type data_type = {};
    bool silence_mode = false;
    bool selected = false;

    virtual ~ExtraCreationData() = default;

protected:
    explicit ExtraCreationData(Type type);
};

struct DropFileData : ExtraCreationData {
    base::FilePath file_path;

    explicit DropFileData(const base::FilePath& path);
};

struct DropTextData : ExtraCreationData {
    std::wstring text;

    explicit DropTextData(const std::wstring& text);
};

struct MonitorCaptureData : ExtraCreationData
{
    gfx::Display display;
    gfx::Rect capture_bounds;

    MonitorCaptureData(const gfx::Display& dsp, const gfx::Rect& rect);
};

struct ProjectionData : ExtraCreationData{
    int projection_type = 0; //0: 无线投屏  1：IOS有线 2：Android 有线
    ProjectionData(int type);
};


struct BrowserPluginData : ExtraCreationData {
    std::string url;
    int64_t plugin_id;
    std::string plugin_name;
    std::string set_url;
    bool is_free;

    BrowserPluginData(int64_t plugin_id, const std::string& url, const std::string& plugin_name, const std::string& set_url,bool free);
};

// `extra_data` could be null if it carries no data.
struct CreatingSourceParams {
    SourceType type = {};
    std::string name_prefix;
    const ExtraCreationData* extra_data = nullptr;

    explicit CreatingSourceParams(SourceType type, const ExtraCreationData* extra_data = nullptr);
};

struct SceneItemCmdHandlerParam
{
	int cmd = 0;
    int item_type = -1;
	std::string scene_name;
    std::string item_name;
	obs_proxy::SceneItem* scene_item = nullptr;
    bool is_from_preset_material = false;
};

obs_proxy::SceneItem* CreateSource(const CreatingSourceParams& params);
obs_proxy::SceneItem* CreateSourceAt(obs_proxy::Scene* scene, const CreatingSourceParams& params);

}   // namespace bililive

#endif  // BILILIVE_BILILIVE_LIVEHIME_OBS_SOURCE_CREATOR_H_
