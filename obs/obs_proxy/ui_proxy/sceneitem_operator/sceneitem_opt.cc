#include "base/logging.h"
#include "base/rand_util.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"

#include "bilibase/lambda_decay.h"
#include "bilibase/scope_guard.h"

#include "bililive/bililive/command_updater_delegate.h"
#include "bililive/bililive/ui/views/menu/sceneitem_menu_cmd.h"
#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_thread.h"

#include "obs/obs_proxy/core_proxy/core_proxy_impl.h"
#include "obs/obs_proxy/core_proxy/scene_collection/obs_scene_collection_impl.h"
#include "obs/obs_proxy/core_proxy/scene_collection/obs_sources_wrapper_impl.h"
#include "obs/obs_proxy/ui_proxy/sceneitem_operator/sceneitem_opt.h"
#include "obs/obs_proxy/ui_proxy/ui_proxy_impl.h"
#include "obs/obs_proxy/ui_proxy/obs_preview/obs_preview.h"
#include "obs/obs_proxy/utils/obs_wrapper_impl_cast.h"

namespace {

bool NeedItemVisualized(obs_proxy::SceneItemType item_type)
{
    return item_type != obs_proxy::SceneItemType::Text &&
           item_type != obs_proxy::SceneItemType::DmkhimeSource &&
           item_type != obs_proxy::SceneItemType::AudioInputCapture &&
           item_type != obs_proxy::SceneItemType::AudioOutputCapture;
}

void TryToWaitVisualizableItemReady(obs_weak_source_t* weak_source)
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::IO));

    obs_source_t* source = obs_weak_source_get_source(weak_source);
    if (!source)
        return;

    const unsigned int kMaxRetry = 5U;
    const int kYieldQuantum = 100; // in milliseconds.
    for (unsigned int try_count = 0U; (obs_source_get_base_width(source) == 0 ||
                                       obs_source_get_base_height(source) == 0) &&
                                      (try_count < kMaxRetry); ++try_count) {
        int max_time_slice = static_cast<int>(1U << try_count);
        auto time_slice = base::RandInt(1, max_time_slice);
        DLOG(INFO) << "Wait source (" << obs_source_get_name(source) << ") for " << time_slice << "slices!";
        auto duration = base::TimeDelta::FromMilliseconds(kYieldQuantum * time_slice);
        base::PlatformThread::Sleep(duration);
    }

    obs_source_release(source);
    obs_weak_source_release(weak_source);
}

}   // namespace

namespace obs_proxy_ui {

SceneItemOpt::SceneItemOpt()
    : scene_collection_(nullptr)
    , scene_(nullptr)
    , scene_item_(nullptr)
{
    scene_collection_ = obs_proxy::GetCoreProxy()->GetCurrentSceneCollection();
    DCHECK(scene_collection_);
}

void SceneItemOpt::ResetOpt(const std::string &scene_name, const std::string &scene_item_name)
{
    scene_ = scene_collection_->GetScene(scene_name);
    DCHECK(scene_);

    scene_item_ = scene_->GetItem(scene_item_name);
    DCHECK(scene_item_);
}

void SceneItemOpt::ExecuteCommandWithParams(int cmd_id, const CommandParamsDetails &params)
{
    SceneItemCmd &cmd_params = *CommandParams<SceneItemCmd>(params).ptr();
    std::string scene_name = UTF16ToUTF8(cmd_params.get()->scene_name());
    std::string item_name = UTF16ToUTF8(cmd_params.get()->item_name());

    DCHECK(!scene_name.empty() && !item_name.empty());

    ResetOpt(scene_name, item_name);
    if (!scene_ || !scene_item_)
    {
        return;
    }

    obs_proxy::SceneItemImpl *scene_item_impl = reinterpret_cast<obs_proxy::SceneItemImpl *>(scene_item_);
    obs_sceneitem_t *raw_scene_item = scene_item_impl->LeakUnderlyingSource();

    switch (cmd_id)
    {
    case IDC_LIVEHIME_SCENE_ITEM_VISUALIZE_BY_SOURCESHOW:
    {
        auto it = std::find(wait_visualization_scene_items_.cbegin(),
            wait_visualization_scene_items_.cend(),
            item_name);
        if (it != wait_visualization_scene_items_.cend())
        {
            ScaleSceneItemToFitDisplay(*it);
            wait_visualization_scene_items_.erase(it);
        }
    }
        break;
    case IDC_LIVEHIME_SCENE_ITEM_USER_ADDED:
    {
        if (NeedItemVisualized(scene_item_->type()))
        {
            wait_visualization_scene_items_.push_back(item_name);
        }
    }
        break;
    case IDC_LIVEHIME_SCENE_ITEM_ACTUAL_SIZE:
        scene_item_->SizeToActualSize();
        break;
    case IDC_LIVEHIME_SCENE_ITEM_FIT_TO_SCREEN:
        scene_item_->SizeToScreenSize();
        break;
    case IDC_LIVEHIME_SCENE_ITEM_DELETE:
        scene_->RemoveItem(item_name);
        break;
    case IDC_LIVEHIME_SCENE_ITEM_TOTOP:
        scene_item_->MoveOrder(obs_proxy::OrderMovement::MoveTop);
        break;
    case IDC_LIVEHIME_SCENE_ITEM_TOBOTTOM:
        scene_item_->MoveOrder(obs_proxy::OrderMovement::MoveBottom);
        break;
    case IDC_LIVEHIME_SCENE_ITEM_MOVEUP:
        scene_item_->MoveOrder(obs_proxy::OrderMovement::MoveUp);
        break;
    case IDC_LIVEHIME_SCENE_ITEM_MOVEDOWN:
        scene_item_->MoveOrder(obs_proxy::OrderMovement::MoveDown);
        break;
    case IDC_LIVEHIME_SCENE_ITEM_SELECT:
    {
        std::vector<obs_proxy::SceneItem*> items = scene_->GetItems();
        for (auto it : items)
        {
            obs_proxy::SceneItemImpl *item_impl = static_cast<obs_proxy::SceneItemImpl*>(it);
            if (item_impl->LeakUnderlyingSource() == raw_scene_item)
            {
                if (!it->IsSelected())
                {
                    it->Setlect(true);
                }
            }
            else
            {
                it->Setlect(false);
            }
        }
    }
        break;
    case IDC_LIVEHIME_SCENE_ITEM_DESELECT:
    {
        if (scene_item_->IsSelected())
        {
            scene_item_->Setlect(false);
        }
    }
        break;
    case IDC_LIVEHIME_SCENE_ITEM_VISIBLE:
        scene_item_->SetVisible(!scene_item_->IsVisible());
        break;
    case IDC_LIVEHIME_SCENE_ITEM_LOCK:
    {
        scene_item_->SetLocked(!scene_item_->IsLocked());

        // 当前版本的obs对于item的lock是没有signal通知的，仅仅只是置一个标识位而已，
        // 需要我们的自己的业务层自行决定什么时候该触发回调，在此设置一个接口给item自己调用
        // 过后如果obs自己提供了signal的通知绑定则应该把此接口去掉
        scene_->SignaledItemLockedChanged(item_name);
    }
        break;
    case IDC_LIVEHIME_SCENE_ITEM_REORDER:
    {
        const std::vector<string16>& items = cmd_params->scene_items();
        for (int i = 0, count = items.size(); i < count; i++)
        {
            auto item = scene_->GetItem(UTF16ToUTF8(items[i]));
            item->SetOrder(i);
        }
    }
        break;
    default:
        break;
    }
}

void SceneItemOpt::ScaleSceneItemToFitDisplay(const std::string& item_name) const
{
    auto raw_source = impl_cast(scene_->GetItem(item_name))->AsSource();
    auto scene_as_source = impl_cast(scene_)->AsSource();
    auto raw_scene_item = impl_cast(scene_->GetItem(item_name))->LeakUnderlyingSource();

    obs_weak_source_t* weak_item_as_source = obs_source_get_weak_source(raw_source);
    obs_weak_source_addref(weak_item_as_source);

    BililiveThread::PostTaskAndReply(
        BililiveThread::IO,
        FROM_HERE,
        base::Bind(TryToWaitVisualizableItemReady, weak_item_as_source),
        base::Bind(bilibase::lambda_decay([](obs_weak_source_t* weak_item_source,
                                             obs_source_t* scene_source,
                                             obs_sceneitem_t* scene_item) {
            obs_source_t* item_source = obs_weak_source_get_source(weak_item_source);

            ON_SCOPE_EXIT{
                if (item_source)
                {
                    obs_source_release(item_source);
                }
                if (weak_item_source)
                {
                    obs_weak_source_release(weak_item_source);
                }
            };

            if (!item_source)
                return;

            uint32_t item_width = obs_source_get_base_width(item_source);
            uint32_t item_height = obs_source_get_base_height(item_source);

            uint32_t scene_width = obs_source_get_base_width(scene_source);
            uint32_t scene_height = obs_source_get_base_height(scene_source);

            if (item_width <= scene_width && item_height <= scene_height) {
                return;
            }

            auto item_aspect = static_cast<double>(item_width) / item_height;
            auto scene_aspect = static_cast<double>(scene_width) / scene_height;
            uint32_t new_item_width = 0U;
            uint32_t new_item_height = 0U;
            float scale = 0.0F;
            if (item_aspect > scene_aspect) {
                new_item_width = scene_width;
                new_item_height = static_cast<uint32_t>(new_item_width / item_aspect);
                scale = static_cast<float>(static_cast<double>(new_item_width) / item_width);
            } else {
                new_item_height = scene_height;
                new_item_width = static_cast<uint32_t>(new_item_height * item_aspect);
                scale = static_cast<float>(static_cast<double>(new_item_height) / item_height);
            }

            obs_transform_info item_info;
            obs_sceneitem_get_info(scene_item, &item_info);

            float item_pos_x = static_cast<float>(scene_width) / 2.0F - static_cast<float>(new_item_width) / 2.0F;
            float item_pos_y = static_cast<float>(scene_height) / 2.0F - static_cast<float>(new_item_height) / 2.0F;
            vec2_set(&item_info.pos, item_pos_x, item_pos_y);
            vec2_set(&item_info.scale, scale, scale);
            vec2_set(&item_info.bounds, new_item_width, new_item_height);
            item_info.bounds_type = OBS_BOUNDS_NONE;
            item_info.rot = 0.0F;

            obs_sceneitem_set_info(scene_item, &item_info);
        }),
                   weak_item_as_source,
                   scene_as_source,
                   raw_scene_item));
}

} // namespace obs_proxy_ui