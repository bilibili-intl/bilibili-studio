#include "obs_scene_util.h"

#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"

#include "bililive/bililive/command_updater_delegate.h"
#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/ui/views/menu/sceneitem_menu_cmd.h"
#include "bililive/public/bililive/bililive_command_ids.h"

#include "obs/obs_proxy/public/proxy/obs_scene_collection.h"
#include "obs/obs_proxy/ui_proxy/sceneitem_operator/sceneitem_opt.h"

namespace
{
    const int64 kFitToScreenTimeSpanInMs = 500;
}

LivehimeSceneItemFitToScreenPresenter::LivehimeSceneItemFitToScreenPresenter()
    : weakptr_factory_(this)
{
}

LivehimeSceneItemFitToScreenPresenter::~LivehimeSceneItemFitToScreenPresenter()
{
    Shutdown();
}

bool LivehimeSceneItemFitToScreenPresenter::Start()
{
    if (!timer_.IsRunning())
    {
        timer_.Start(FROM_HERE, base::TimeDelta::FromMilliseconds(kFitToScreenTimeSpanInMs),
                     base::Bind(&LivehimeSceneItemFitToScreenPresenter::OnTimer, weakptr_factory_.GetWeakPtr()));
        return true;
    }
    return false;
}

void LivehimeSceneItemFitToScreenPresenter::Shutdown()
{
    if (timer_.IsRunning())
    {
        timer_.Stop();
    }
}

void LivehimeSceneItemFitToScreenPresenter::OnTimer()
{
    auto scene_collection = OBSProxyService::GetInstance().GetOBSCoreProxy()->GetCurrentSceneCollection();
    if (scene_collection)
    {
        auto scene = scene_collection->current_scene();
        if(scene)
        {
            auto scene_name = scene->name();
            auto items = scene->GetItems();
            for (auto& item : items)
            {
                if (item->IsFitToScreen())
                {
                    SceneItemCmd cmd_param(new SCENEITEMCMD_(base::UTF8ToUTF16(scene_name), base::UTF8ToUTF16(item->name())));
                    std::unique_ptr<obs_proxy_ui::SceneItemOpt> item_opt(new obs_proxy_ui::SceneItemOpt());
                    item_opt->ExecuteCommandWithParams(IDC_LIVEHIME_SCENE_ITEM_FIT_TO_SCREEN, CommandParams<SceneItemCmd>(&cmd_param));
                }
            }
        }
    }
}
