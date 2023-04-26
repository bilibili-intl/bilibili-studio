#ifndef OBS_OBS_PROXY_UI_PROXY_SCENEITEM_OPERATOR_SCENEITEM_OPT_H
#define OBS_OBS_PROXY_UI_PROXY_SCENEITEM_OPERATOR_SCENEITEM_OPT_H

#include <vector>

#include "base/strings/string16.h"

namespace obs_proxy {
class SceneCollection ;
class Scene;
class SceneItem;
}

class CommandParamsDetails;

namespace obs_proxy_ui
{
// 辅助UIProxyImpl完成针对指定scene中的指定item的操作
class SceneItemOpt
{
public:
    SceneItemOpt();

    ~SceneItemOpt() = default;
    
    void ExecuteCommandWithParams(int cmd_id, const CommandParamsDetails &params);

private:
    void ResetOpt(const std::string &scene_name, const std::string &scene_item_name);
    void ScaleSceneItemToFitDisplay(const std::string& item_name) const;

private:
    obs_proxy::SceneCollection* scene_collection_;
    obs_proxy::Scene* scene_;
    obs_proxy::SceneItem* scene_item_;
    std::vector<std::string> wait_visualization_scene_items_;
};

}//namespace obs_proxy_ui


#endif // OBS_OBS_PROXY_UI_PROXY_SCENEITEM_OPERATOR_SCENEITEM_OPT_H