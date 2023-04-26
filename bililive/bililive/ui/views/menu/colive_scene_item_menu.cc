#include "bililive/bililive/ui/views/menu/colive_scene_item_menu.h"

#include "base/strings/utf_string_conversions.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/base/win/dpi.h"
#include "ui/views/controls/menu/menu_runner.h"
#include "ui/views/widget/widget.h"

#include "bililive/bililive/livehime/event_report/event_report_center.h"
#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/ui/bililive_commands.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_menu.h"
#include "bililive/bililive/ui/views/menu/sceneitem_menu_cmd.h"

#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "bililive/public/secret/bililive_secret.h"

#include "grit/generated_resources.h"

#include "obs/obs_proxy/public/common/pref_names.h"
#include "obs/obs_proxy/public/common/pref_constants.h"
#include "obs/obs_proxy/public/proxy/obs_proxy.h"
#include "obs/obs_proxy/public/proxy/obs_scene_collection.h"
#include "obs/obs_proxy/public/proxy/obs_sources_wrapper.h"
#include <bililive/bililive/livehime/obs/srcprop_ui_util.h>


namespace {

    const std::map<int, secret::LivehimeBehaviorEvent> kEventTable
    {
        { IDC_LIVEHIME_SCENE_ITEM_ACTUAL_SIZE, secret::LivehimeBehaviorEvent::LivehimeRightKeyOriginalSize },
        { IDC_LIVEHIME_SCENE_ITEM_TOTOP, secret::LivehimeBehaviorEvent::LivehimeRightKeyTop },
        { IDC_LIVEHIME_SCENE_ITEM_TOBOTTOM, secret::LivehimeBehaviorEvent::LivehimeRightKeyLow},
        { IDC_LIVEHIME_SCENE_ITEM_SCALE_ZOOM, secret::LivehimeBehaviorEvent::LivehimeRightKeyZoom },
        { IDC_LIVEHIME_SCENE_ITEM_SCALE_STRETCH, secret::LivehimeBehaviorEvent::LivehimeRightKeyStretch },
        { IDC_LIVEHIME_SCENE_ITEM_SCALE_CUT, secret::LivehimeBehaviorEvent::LivehimeRightKeyCut },
        { IDC_LIVEHIME_SCENE_ITEM_FIT_TO_SCREEN, secret::LivehimeBehaviorEvent::LivehimeRightKeyFullScreen },
        { IDC_LIVEHIME_SCENE_ITEM_LOCK, secret::LivehimeBehaviorEvent::LivehimeSceneLock },
        { IDC_LIVEHIME_SCENE_ITEM_DELETE, secret::LivehimeBehaviorEvent::LivehimeRightKeyDelete },
        { IDC_LIVEHIME_SCENE_ITEM_INTERACTION, secret::LivehimeBehaviorEvent::LivehimeRightKeyDelete },
        { IDC_LIVEHIME_SCENE_ITEM_PROPERTY, secret::LivehimeBehaviorEvent::LivehimeRightKeyProperty }
    };

    void EventTracking(int command_id)
    {
        secret::LivehimeBehaviorEvent event_id = kEventTable.at(command_id);
        auto secret_core = GetBililiveProcess()->secret_core();
        secret_core->event_tracking_service()->ReportLivehimeBehaviorEvent(
            event_id, secret_core->account_info().mid(), std::string()).Call();

        switch (command_id)
        {
        case IDC_LIVEHIME_SCENE_ITEM_ACTUAL_SIZE:
            livehime::BehaviorEventReport(secret::LivehimeBehaviorEventV2::SceneItemMenuClick, "button_type:1");
            break;
        case IDC_LIVEHIME_SCENE_ITEM_TOTOP:
            livehime::BehaviorEventReport(secret::LivehimeBehaviorEventV2::SceneItemMenuClick, "button_type:2");
            break;
        case IDC_LIVEHIME_SCENE_ITEM_TOBOTTOM:
            livehime::BehaviorEventReport(secret::LivehimeBehaviorEventV2::SceneItemMenuClick, "button_type:3");
            break;
        case IDC_LIVEHIME_SCENE_ITEM_SCALE_ZOOM:
            livehime::BehaviorEventReport(secret::LivehimeBehaviorEventV2::SceneItemMenuClick, "button_type:4");
            break;
        case IDC_LIVEHIME_SCENE_ITEM_SCALE_STRETCH:
            livehime::BehaviorEventReport(secret::LivehimeBehaviorEventV2::SceneItemMenuClick, "button_type:5");
            break;
        case IDC_LIVEHIME_SCENE_ITEM_SCALE_CUT:
            livehime::BehaviorEventReport(secret::LivehimeBehaviorEventV2::SceneItemMenuClick, "button_type:6");
            break;
        case IDC_LIVEHIME_SCENE_ITEM_FIT_TO_SCREEN:
            livehime::BehaviorEventReport(secret::LivehimeBehaviorEventV2::SceneItemMenuClick, "button_type:7");
            break;
        case IDC_LIVEHIME_SCENE_ITEM_LOCK:
            livehime::BehaviorEventReport(secret::LivehimeBehaviorEventV2::SceneItemMenuClick, "button_type:8");
            break;
        case IDC_LIVEHIME_SCENE_ITEM_DELETE:
            livehime::BehaviorEventReport(secret::LivehimeBehaviorEventV2::SceneItemMenuClick, "button_type:9");
            break;
        case IDC_LIVEHIME_SCENE_ITEM_INTERACTION:
            livehime::BehaviorEventReport(secret::LivehimeBehaviorEventV2::SceneItemMenuClick, "button_type:10");
            break;
        case IDC_LIVEHIME_SCENE_ITEM_PROPERTY:
            livehime::BehaviorEventReport(secret::LivehimeBehaviorEventV2::SceneItemMenuClick, "button_type:11");
            break;
        default:
            break;
        }
    }
}


ColiveSceneItemMenu::ColiveSceneItemMenu(views::View *target)
    : target_(target)
{
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();
    LivehimeMenuItemView *menu = new LivehimeMenuItemView(this);

    menu->AppendMenuItem(IDC_LIVEHIME_SCENE_ITEM_ACTUAL_SIZE, rb.GetLocalizedString(IDS_CONTENTMENU_ONETOONE));
    menu->AppendMenuItem(IDC_LIVEHIME_SCENE_ITEM_TOTOP, rb.GetLocalizedString(IDS_CONTENTMENU_TOTOP));
    menu->AppendMenuItem(IDC_LIVEHIME_SCENE_ITEM_TOBOTTOM, rb.GetLocalizedString(IDS_CONTENTMENU_TOBOTTOM));

    menu->AppendSeparator();

    menu->AppendCheckMenu(IDC_LIVEHIME_SCENE_ITEM_SCALE_ZOOM, rb.GetLocalizedString(IDS_SOURCE_DRAGMODE_ZOOM));
    menu->AppendCheckMenu(IDC_LIVEHIME_SCENE_ITEM_SCALE_STRETCH, rb.GetLocalizedString(IDS_SOURCE_DRAGMODE_STRETCH));
    menu->AppendCheckMenu(IDC_LIVEHIME_SCENE_ITEM_SCALE_CUT, rb.GetLocalizedString(IDS_SOURCE_DRAGMODE_CUT));

    menu->AppendSeparator();

    menu->AppendCheckMenu(IDC_LIVEHIME_SCENE_ITEM_FIT_TO_SCREEN, rb.GetLocalizedString(IDS_CONTENTMENU_FULLSCREEN));
    menu->AppendCheckMenu(IDC_LIVEHIME_SCENE_ITEM_LOCK, GetLocalizedString(IDS_TABAREA_SCENE_TIP_LOCK));
    menu->AppendMenuItem(IDC_LIVEHIME_SCENE_ITEM_DELETE, rb.GetLocalizedString(IDS_CONTENTMENU_DELCUR));

    menu->AppendSeparator();

    menu->AppendMenuItem(IDC_LIVEHIME_SCENE_ITEM_INTERACTION, L"交互");
    menu->AppendMenuItem(IDC_LIVEHIME_SCENE_ITEM_PROPERTY, rb.GetLocalizedString(IDS_CONTENTMENU_ATTR));

    menu_runner_.reset(new views::MenuRunner(menu));
    target_->set_context_menu_controller(this);
}

ColiveSceneItemMenu::~ColiveSceneItemMenu() {
}

void ColiveSceneItemMenu::CancelMenu()
{
    menu_runner_->Cancel();
}

void ColiveSceneItemMenu::ShowContextMenuForView(
    views::View* source, const gfx::Point& point, ui::MenuSourceType source_type)
{
    obs_proxy::SceneItem* selected_item = GetSelectedSceneItem();
    if (!!selected_item)
    {
        // 判断交互选项
        views::MenuItemView* menu_item = menu_runner_->GetMenu()->GetMenuItemByID(IDC_LIVEHIME_SCENE_ITEM_INTERACTION);
        menu_item->SetEnabled(selected_item->type() == obs_proxy::SceneItemType::BrowserSource);

        // 任务活动进度页源不展示属性页和交互
        if (selected_item->type() == obs_proxy::SceneItemType::BrowserSource)
        {
            auto scene_item_helper_ = std::make_unique<BrowserItemHelper>(selected_item);
            if (scene_item_helper_->activity_type() == BrowserItemHelper::ActivityType::MissionActivity)
            {
                menu_item->SetEnabled(false);

                // 属性
                menu_item = menu_runner_->GetMenu()->GetMenuItemByID(IDC_LIVEHIME_SCENE_ITEM_PROPERTY);
                menu_item->SetEnabled(false);
            }
        }

        BeforeRunMenu(menu_runner_->GetMenu());

        gfx::Point dip_pt = point;
        menu_runner_->RunMenuAt(
            source->GetWidget()->GetTopLevelWidget(),
            NULL,
            gfx::Rect(dip_pt, gfx::Size(0, 0)),
            views::MenuItemView::TOPLEFT,
            source_type,
            views::MenuRunner::CONTEXT_MENU);
    }
}

bool ColiveSceneItemMenu::IsItemChecked(int command_id) const
{
    auto checkSourceDragMode = [](const std::string& mode)->bool
    {
        PrefService *pref = GetBililiveProcess()->profile()->GetPrefs();
        std::string sourceDragMode = pref->GetString(prefs::kSourceDragMode);
        return sourceDragMode == mode;
    };

    bool checked = false;
    switch (command_id)
    {
    case IDC_LIVEHIME_SCENE_ITEM_SCALE_ZOOM:
        checked = checkSourceDragMode(prefs::kSourceDragModeZoom);
        break;;
    case IDC_LIVEHIME_SCENE_ITEM_SCALE_STRETCH:
        checked = checkSourceDragMode(prefs::kSourceDragModeStretch);
        break;;
    case IDC_LIVEHIME_SCENE_ITEM_SCALE_CUT:
        checked = checkSourceDragMode(prefs::kSourceDragModeCut);
        break;
    case IDC_LIVEHIME_SCENE_ITEM_FIT_TO_SCREEN:
    {
        auto selected_item = GetSelectedSceneItem();
        if (selected_item) {
            checked = selected_item->IsFitToScreen();
        }
    }
    break;
    case IDC_LIVEHIME_SCENE_ITEM_LOCK:
    {
        auto selected_item = GetSelectedSceneItem();
        if (selected_item)
        {
            checked = selected_item->IsLocked();
        }
    }
    break;
    default:
        break;
    }

    return checked;
}

void ColiveSceneItemMenu::ExecuteCommand(int command_id, int event_flags)
{
    string16 scene_name;
    obs_proxy::SceneItem* selected_item = nullptr;
    auto scene_collection = OBSProxyService::GetInstance().GetOBSCoreProxy()->GetCurrentSceneCollection();
    if (scene_collection) {
        auto scene = scene_collection->GetScene(prefs::kJoinMicSceneName);
        if (scene) {
            scene_name = UTF8ToUTF16(scene->name());
            selected_item = scene->GetSelectedItem();
        }
    }
    if (!selected_item) {
        NOTREACHED() << "unbelievable";
        return;
    }

    if (IDC_LIVEHIME_SCENE_ITEM_FIT_TO_SCREEN == command_id) {
        selected_item->SetFitToScreen(!selected_item->IsFitToScreen());
    }

    if (command_id == IDC_LIVEHIME_SCENE_ITEM_DELETE_FROM_PREVIEW) {
        command_id = IDC_LIVEHIME_SCENE_ITEM_DELETE;
    }

    auto item_name = UTF8ToUTF16(selected_item->name());

    // 发通知，菜单选择了一条与sceneitem操作相关的命令
    SceneItemCmd cmd_params(new SCENEITEMCMD_(scene_name, item_name));
    bililive::ExecuteCommandWithParams(
        GetBililiveProcess()->bililive_obs(),
        command_id,
        CommandParams<SceneItemCmd>(&cmd_params));

    EventTracking(command_id);
}

obs_proxy::SceneItem* ColiveSceneItemMenu::GetSelectedSceneItem() const {
    auto scene_collection = OBSProxyService::GetInstance().GetOBSCoreProxy()->GetCurrentSceneCollection();
    if (scene_collection) {
        auto scene = scene_collection->GetScene(prefs::kJoinMicSceneName);
        if (scene) {
            return scene->GetSelectedItem();
        }
    }
    return nullptr;
}