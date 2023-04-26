#include "obs/obs_proxy/ui_proxy/ui_proxy_impl.h"

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"

#include "bililive/bililive/command_updater_delegate.h"
#include "bililive/bililive/livehime/obs/srcprop_ui_util.h"
#include "bililive/bililive/ui/views/menu/sceneitem_menu_cmd.h"

#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_thread.h"

#include "obs/obs_proxy/app/obs_proxy_access_stub.h"
#include "obs/obs_proxy/common/obs_proxy_constants.h"
#include "obs/obs_proxy/core_proxy/core_proxy_impl.h"
#include "obs/obs_proxy/core_proxy/scene_collection/obs_audio_devices_impl.h"
#include "obs/obs_proxy/core_proxy/scene_collection/obs_sources_wrapper_impl.h"
#include "obs/obs_proxy/public/proxy/obs_proxy_delegate.h"
#include "obs/obs_proxy/ui_proxy/obs_preview/obs_preview.h"
#include "obs/obs_proxy/ui_proxy/obs_preview/obs_preview_controller.h"
#include "obs/obs_proxy/ui_proxy/sceneitem_operator/sceneitem_opt.h"
#include "obs/obs_proxy/utils/obs_wrapper_impl_cast.h"
#include "obs/obs_proxy/public/common/pref_constants.h"


namespace obs_proxy_ui{

obs_proxy_ui::UIProxyImpl *g_proxy_ui_impl = nullptr;

// static
obs_scene_t *UIProxyImpl::GetRawCurrentScene() {

    obs_proxy::CoreProxyImpl* proxy_impl = obs_proxy::GetCoreProxy() ;
    if (proxy_impl)
    {
        obs_proxy::SceneCollection* scene_collection = proxy_impl->GetCurrentSceneCollection();
        if (scene_collection)
        {
            obs_proxy::SceneImpl* scene_impl = impl_cast(scene_collection->current_scene());
            if(scene_impl)
            {
                return scene_impl->LeakUnderlyingSource();
            }
        }
    }
    NOTREACHED() << "must be called after CoreProxy init scene collection by LoadSceneCollection";
    return nullptr;
}

// static
obs_scene_t* UIProxyImpl::GetRawCurrentColiveScene() {

    obs_proxy::CoreProxyImpl* proxy_impl = obs_proxy::GetCoreProxy();
    if (proxy_impl)
    {
        obs_proxy::SceneCollection* scene_collection = proxy_impl->GetCurrentSceneCollection();
        if (scene_collection)
        {
            obs_proxy::SceneImpl* scene_impl = impl_cast(scene_collection->GetScene(prefs::kJoinMicSceneName));
            if (scene_impl)
            {
                return scene_impl->LeakUnderlyingSource();
            }
        }
    }
    return nullptr;
}

// static
obs_scene_t* GetRawScene(const std::string name) {

    obs_proxy::CoreProxyImpl* proxy_impl = obs_proxy::GetCoreProxy();
    if (proxy_impl)
    {
        obs_proxy::SceneCollection* scene_collection = proxy_impl->GetCurrentSceneCollection();
        if (scene_collection)
        {
            obs_proxy::SceneImpl* scene_impl = impl_cast(scene_collection->GetScene(name));
            if (scene_impl)
            {
                return scene_impl->LeakUnderlyingSource();
            }
        }
    }
    NOTREACHED() << "must be called after CoreProxy init scene collection by LoadSceneCollection";
    return nullptr;
}


void UIProxyImpl::BindSignals(const std::vector<obs_proxy::Scene*>& scenes)
{
    //在ui_proxy 的 PostBililiveStart 里面进行绑定, 此时 scene_collection等已经创建并初始化完毕
    //此时绑定信号后, 需要手动触发一些原来应该在scene等初始化之前调用的功能
    obs_proxy::CoreProxyImpl *core_proxy_impl = obs_proxy::GetCoreProxy();
    obs_proxy_ui::UIProxyImpl *ui_proxy_impl = obs_proxy_ui::GetUIProxy();

    auto scene_collection = static_cast<obs_proxy::SceneCollectionImpl*>(core_proxy_impl->GetCurrentSceneCollection());
    DCHECK(scene_collection);
    scene_collection->RegisterCurrentSceneChangedHandler(UIProxyImpl::OnCurrentSceneChangedHandlerRedirect);
    scene_collection->RegisterSourceShowHandler(UIProxyImpl::OnSourceShowHandlerRedirect);
    scene_collection->RegisterAudioSourceActivateHandler(UIProxyImpl::OnAudioSourceActivateHandlerRedirect);
    scene_collection->RegisterAudioSourceDeactivateHandler(UIProxyImpl::OnAudioSourceDeactivateHandlerRedirect);
    scene_collection->RegisterSourceRenameHandler(UIProxyImpl::OnSourceRenameHandlerRedirect);

    // 从core拿场景集合，为每一个场景在主界面右后侧面板添加一个table，
    // 再拿到场景内的scene item集合，逐一添加到table中以初始化UI数据显示
    // 应该在此过滤掉0~2之外的scene，否则在此注册回调之后，运行时将发生其他scene的item被添加到当前场景的情况
    int cur_scene_index = 0;
    for (auto scene : scenes)
    {
        if (scene == scene_collection->current_scene())
        {
            auto it = std::find(scenes.cbegin(), scenes.cend(), scene);
            cur_scene_index = it - scenes.cbegin();
        }

        base::string16 scene_name = UTF8ToUTF16(scene->name());

        std::vector<obs_proxy::SceneItem*> items = scene->GetItems();
        std::vector<string16> names;
        for (auto item : items)
        {
            if (item)
            {
                base::string16 item_name = UTF8ToUTF16(item->name());
                names.push_back(item_name);

                TableItem table_item;
                table_item.set_contents(item_name);
                table_item.set_visible(item->IsVisible());
                table_item.set_locked(item->IsLocked());

                if (item->type() == obs_proxy::SceneItemType::BrowserSource)
                {
                    auto scene_item_helper_ = std::make_unique<BrowserItemHelper>(item);
                    if (scene_item_helper_->GetIsPlugin())
                    {
                        table_item.set_browser_plugin();
                    }
                }

                FOR_EACH_OBSERVER(obs_proxy_ui::OBSUIProxyObserver, ui_proxy_impl->observer_list_,
                    OnSceneItemAdd(scene_name, table_item, static_cast<obs_proxy::SceneItemImpl*>(item)->AsSource(), (int)item->type()));
            }
        }
        if (names.size())
        {
            std::reverse(names.begin(), names.end());

            FOR_EACH_OBSERVER(obs_proxy_ui::OBSUIProxyObserver, ui_proxy_impl->observer_list_,
                OnSceneItemReordered(scene_name, names));
        }

        InstallSignal(scene);
    }
    // 使用上一次退出前使用的场景
    ui_proxy_impl->ChangeCurrentScene(cur_scene_index);
}

// static
void UIProxyImpl::InstallSignal(obs_proxy::Scene* scene) {
    scene->RegisterItemAddHandler(UIProxyImpl::OnSceneItemAddHandlerRedirect);
    scene->RegisterItemRemoveHandler(UIProxyImpl::OnSceneItemRemoveHandlerRedirect);
    scene->RegisterItemSelectedHandler(UIProxyImpl::OnSceneItemSelectedHandlerRedirect);
    scene->RegisterItemDeselectHandler(UIProxyImpl::OnSceneItemDeselectHandlerRedirect);
    scene->RegisterItemReorderedHandler(UIProxyImpl::OnSceneItemReorderedHandlerRedirect);
    scene->RegisterItemVisibleHandler(UIProxyImpl::OnSceneItemVisibleHandlerRedirect);
    scene->RegisterItemLockedHandler(UIProxyImpl::OnSceneItemLockedHandlerRedirect);
}

// scene-collection callback
void UIProxyImpl::OnCurrentSceneChangedHandlerRedirect(int scene_index)
{
    obs_proxy_ui::UIProxyImpl *ui_proxy_impl = obs_proxy_ui::GetUIProxy();
    ui_proxy_impl->ChangeCurrentScene(scene_index);
}

void UIProxyImpl::OnSourceShowHandlerRedirect(const std::string& item_name)
{
    obs_proxy_ui::UIProxyImpl *ui_proxy_impl = obs_proxy_ui::GetUIProxy();

    base::string16 scene_name = UTF8ToUTF16(
        obs_proxy::GetCoreProxy()->GetCurrentSceneCollection()->current_scene()->name());

    SceneItemCmd cmd_params(new SCENEITEMCMD_(scene_name, UTF8ToUTF16(item_name)));
    ui_proxy_impl->ExecuteCommandWithParams(IDC_LIVEHIME_SCENE_ITEM_VISUALIZE_BY_SOURCESHOW,
        CommandParams<SceneItemCmd>(&cmd_params));
}

void UIProxyImpl::OnAudioSourceActivateHandlerRedirect(obs_proxy::VolumeController* audio_source)
{
    obs_proxy_ui::UIProxyImpl *ui_proxy_impl = obs_proxy_ui::GetUIProxy();

    FOR_EACH_OBSERVER(obs_proxy_ui::OBSUIProxyObserver, ui_proxy_impl->observer_list_,
                      OnAudioSourceActivate(audio_source));
}

void UIProxyImpl::OnAudioSourceDeactivateHandlerRedirect(const std::string& name)
{
    obs_proxy_ui::UIProxyImpl *ui_proxy_impl = obs_proxy_ui::GetUIProxy();

    FOR_EACH_OBSERVER(obs_proxy_ui::OBSUIProxyObserver, ui_proxy_impl->observer_list_,
                      OnAudioSourceDeactivate(name));
}

void UIProxyImpl::OnSourceRenameHandlerRedirect(const std::string& prev_name, const std::string& new_name)
{
    obs_proxy_ui::UIProxyImpl *ui_proxy_impl = obs_proxy_ui::GetUIProxy();

    base::string16 prev_item_name = UTF8ToUTF16(prev_name);
    base::string16 new_item_name = UTF8ToUTF16(new_name);
    FOR_EACH_OBSERVER(obs_proxy_ui::OBSUIProxyObserver, ui_proxy_impl->observer_list_,
                      OnSourceRename(prev_item_name, new_item_name));
}

// scene callback
void UIProxyImpl::OnSceneItemAddHandlerRedirect(const std::string& scene_name, const std::string& item_name, obs_source_t* source, int item_type)
{
    obs_proxy_ui::UIProxyImpl *ui_proxy_impl = obs_proxy_ui::GetUIProxy();
    TableItem table_item;
    table_item.set_contents(UTF8ToUTF16(item_name));

    base::string16 scene_name_w = UTF8ToUTF16(scene_name);

    LOG(INFO) << "scene '" << scene_name << "' add item : '" << item_name << "'";

    FOR_EACH_OBSERVER(obs_proxy_ui::OBSUIProxyObserver, ui_proxy_impl->observer_list_,
        OnSceneItemAdd(scene_name_w, table_item, source, item_type));
}

void UIProxyImpl::OnSceneItemRemoveHandlerRedirect(const std::string& scene_name, const std::string& item_name,
                                                   obs_source_t* underlying_source)
{
    obs_proxy_ui::UIProxyImpl *ui_proxy_impl = obs_proxy_ui::GetUIProxy();

    base::string16 scene_name_w = UTF8ToUTF16(scene_name);
    base::string16 item_name_w = UTF8ToUTF16(item_name);

    LOG(INFO) << "scene '" << scene_name << "' remove item : '" << item_name << "'";

    // 通知scene-colllection item被删了，colllection可以把对应的volumecontrolller删掉了
    obs_proxy::CoreProxyImpl *core_proxy_impl = obs_proxy::GetCoreProxy();
    if (core_proxy_impl)
    {
		auto scene_collection = static_cast<obs_proxy::SceneCollectionImpl*>(core_proxy_impl->GetCurrentSceneCollection());
		if (scene_collection)
		{
            scene_collection->SceneItemRemove(underlying_source);
		}
    }
    // 将预览区持有引用的scene-item释放掉，以便OBS彻底释放source
    if (ui_proxy_impl->main_obs_preview_)
    {
        ui_proxy_impl->main_obs_preview_->ResetDetails();
    }

    FOR_EACH_OBSERVER(obs_proxy_ui::OBSUIProxyObserver, ui_proxy_impl->observer_list_,
        OnSceneItemRemove(scene_name_w, item_name_w));
}

void UIProxyImpl::OnSceneItemSelectedHandlerRedirect(const std::string& scene_name, const std::string& item_name)
{
    obs_proxy_ui::UIProxyImpl *ui_proxy_impl = obs_proxy_ui::GetUIProxy();

    base::string16 scene_name_w = UTF8ToUTF16(scene_name);
    base::string16 item_name_w = UTF8ToUTF16(item_name);
    FOR_EACH_OBSERVER(obs_proxy_ui::OBSUIProxyObserver, ui_proxy_impl->observer_list_,
        OnSceneItemSelected(scene_name_w, item_name_w));
}

void UIProxyImpl::OnSceneItemDeselectHandlerRedirect(const std::string& scene_name, const std::string& item_name)
{
    obs_proxy_ui::UIProxyImpl *ui_proxy_impl = obs_proxy_ui::GetUIProxy();

    base::string16 scene_name_w = UTF8ToUTF16(scene_name);
    base::string16 item_name_w = UTF8ToUTF16(item_name);
    FOR_EACH_OBSERVER(obs_proxy_ui::OBSUIProxyObserver, ui_proxy_impl->observer_list_,
        OnSceneItemDeselect(scene_name_w, item_name_w));
}

void UIProxyImpl::OnSceneItemReorderedHandlerRedirect(const std::string& scene_name)
{
    obs_proxy_ui::UIProxyImpl *ui_proxy_impl = obs_proxy_ui::GetUIProxy();

    std::vector<string16> item_names;
    obs_scene_t *scene = GetRawScene(scene_name);
    if(scene)
    {
        obs_scene_enum_items(
            scene,
            [](obs_scene_t *, obs_sceneitem_t *item, void *param) {
                std::vector<string16> &names =
                    *reinterpret_cast<std::vector<string16>*>(param);
                const char *name = obs_source_get_name(obs_sceneitem_get_source(item));
                string16 name16;

                bool is_ok = UTF8ToUTF16(name, strlen(name), &name16);
                DCHECK(is_ok);

                names.push_back(name16);
                return true;
            },
            &item_names
            );

        std::reverse(item_names.begin(), item_names.end());

        base::string16 scene_name_w = UTF8ToUTF16(scene_name);
        FOR_EACH_OBSERVER(obs_proxy_ui::OBSUIProxyObserver, ui_proxy_impl->observer_list_,
            OnSceneItemReordered(scene_name_w, item_names));
    }
}

void UIProxyImpl::OnSceneItemVisibleHandlerRedirect(const std::string& scene_name,
    const std::string& item_name, bool visible)
{
    obs_proxy_ui::UIProxyImpl *ui_proxy_impl = obs_proxy_ui::GetUIProxy();

    base::string16 scene_name_w = UTF8ToUTF16(scene_name);
    base::string16 item_name_w = UTF8ToUTF16(item_name);
    FOR_EACH_OBSERVER(obs_proxy_ui::OBSUIProxyObserver, ui_proxy_impl->observer_list_,
        OnSceneItemVisible(scene_name_w, item_name_w, visible));
}

void UIProxyImpl::OnSceneItemLockedHandlerRedirect(const std::string& scene_name, const std::string& item_name, bool locked)
{
    obs_proxy_ui::UIProxyImpl *ui_proxy_impl = obs_proxy_ui::GetUIProxy();

    base::string16 scene_name_w = UTF8ToUTF16(scene_name);
    base::string16 item_name_w = UTF8ToUTF16(item_name);
    FOR_EACH_OBSERVER(obs_proxy_ui::OBSUIProxyObserver, ui_proxy_impl->observer_list_,
                      OnSceneItemLocked(scene_name_w, item_name_w, locked));
}


// -*- OBSProxyForUIImpl -*-
UIProxyImpl::UIProxyImpl()
{
    DCHECK(g_proxy_ui_impl == nullptr);
    g_proxy_ui_impl = this;
}

UIProxyImpl::~UIProxyImpl()
{
    g_proxy_ui_impl = nullptr;

    DCHECK(!main_obs_preview_.get());
    DCHECK(obs_previews_.size() == 0);
}

bool UIProxyImpl::is_initialized() const
{
    return is_initialized_;
}

void UIProxyImpl::AddObserver(OBSUIProxyObserver *observer)
{
    observer_list_.AddObserver(observer);
}

void UIProxyImpl::RemoveObserver(OBSUIProxyObserver *observer)
{
    observer_list_.RemoveObserver(observer);
}

void UIProxyImpl::ResetSourcesPos(obs_video_info& old_ovi)
{
    if (main_obs_preview())
    {
        obs_proxy::SceneCollectionImpl *scene_collection_impl
            = static_cast<obs_proxy::SceneCollectionImpl *>(obs_proxy::GetCoreProxy()->GetCurrentSceneCollection());

        std::vector<obs_proxy::Scene*> scenes = scene_collection_impl->GetScenes();
        for (obs_proxy::Scene* s : scenes)
        {
            std::vector<obs_proxy::SceneItem*> sceneitems = s->GetItems();
            for (obs_proxy::SceneItem* si : sceneitems)
            {
                if ((si->type() != obs_proxy::SceneItemType::AudioInputCapture) &&
                    (si->type() != obs_proxy::SceneItemType::AudioOutputCapture))
                {
                    obs_proxy::SceneItemImpl* si_impl = impl_cast(si);
                    obs_sceneitem_t* sceneitem = si_impl->LeakUnderlyingSource();

                    main_obs_preview()->ResetSourcesPos(sceneitem, old_ovi);
                }
            }
        }
    }
}

void UIProxyImpl::ChangeCurrentScene(size_t scene_index)
{
    FOR_EACH_OBSERVER(obs_proxy_ui::OBSUIProxyObserver, observer_list_,
        OnCurrentSceneChanged(scene_index));
}

// OBSUIProxy
void UIProxyImpl::Initialize()
{
    // 调obs_core拿scene集合，并最终调用自己的BindSignals以遍历场景初始化数据到UI上
    obs_proxy::CoreProxyImpl::BindSignals();
    // scene_collection的创建和初始化是在UI创建之后的，所以在scene_collection初始化之后
    // 再为主界面的OBSPreview实际创建obs_display，否则在OBSPreview::Render中会引用到空的scene_collection
    DCHECK(main_obs_preview_);
    main_obs_preview_->CreateDisplay();

    // 初始化完毕，通知上层
    is_initialized_ = true;
    FOR_EACH_OBSERVER(obs_proxy_ui::OBSUIProxyObserver, observer_list_, OnOBSUIProxyInitialized());
}

OBSPreview* UIProxyImpl::ConnectOBSPReview(OBSPreviewController *preview_controller, PreviewType type)
{
    if (type == PreviewType::PREVIEW_MAIN)
    {
        DCHECK(!main_obs_preview_.get());
    }

    std::unique_ptr<OBSPreview> obs_preview(new OBSPreview(preview_controller, type));
    OBSPreview *raw_obs_preview = obs_preview.get();

    if (type == PreviewType::PREVIEW_MAIN)
    {
        main_obs_preview_ = std::move(obs_preview);
        // 主界面的obs_preview不急CreateDisplay，不然Render拿不到场景信息
    }
    else
    {
        obs_previews_.push_back(std::move(obs_preview));
        raw_obs_preview->CreateDisplay();
    }

    return raw_obs_preview;
}

void UIProxyImpl::DisonnectOBSPReview(OBSPreviewController *preview_controller)
{
    if (preview_controller->obs_preview() == main_obs_preview_.get())
    {
        main_obs_preview_.reset();
    }
    else
    {
        for (auto iter = obs_previews_.begin(); iter != obs_previews_.end(); ++iter)
        {
            if (preview_controller->obs_preview() == iter->get())
            {
                obs_previews_.erase(iter);
                return;
            }
        }
    }
}

void UIProxyImpl::ResizeMainOBSPreview(uint32_t base_cx, uint32_t base_cy)
{
    if (main_obs_preview())
    {
        main_obs_preview()->RecalculateDisplayViewport();
    }
}

void UIProxyImpl::ExecuteCommandWithParams(int cmd_id, const CommandParamsDetails &params)
{
    if (!sceneitem_opt_)
    {
        sceneitem_opt_.reset(new SceneItemOpt());
    }
    sceneitem_opt_->ExecuteCommandWithParams(cmd_id, params);
}


UIProxyImpl* GetUIProxy()
{
    DCHECK(g_proxy_ui_impl != nullptr);
    return g_proxy_ui_impl;
}

}//namespace obs_proxy