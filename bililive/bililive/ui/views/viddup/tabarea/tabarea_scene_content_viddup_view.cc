#include "bililive/bililive/ui/views/viddup/tabarea/tabarea_scene_content_viddup_view.h"

#include "bililive/common/bililive_logging.h"

#include "bililive/bililive/bililive_database.h"
#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/livehime/obs/srcprop_ui_util.h"
#include "bililive/bililive/ui/bililive_commands.h"
#include "bililive/bililive/ui/views/controls/util/bililive_util_views.h"
#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_menu.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_bubble.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/bililive/ui/views/hotkey_notify/bililive_toast_notify_view.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_media_property_view.h"
#include "bililive/bililive/ui/views/preview/livehime_preview_area_utils.h"
#include "bililive/bililive/ui/views/viddup/tabarea/canvas_materials_viddup_view.h"

#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/public/secret/bililive_secret.h"

#include "base/notification/notification_observer.h"
#include "base/strings/utf_string_conversions.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_util.h"

#include "ui/views/layout/grid_layout.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/controls/button/label_button.h"
#include "ui/views/controls/button/image_button.h"
#include "ui/views/controls/menu/menu_runner.h"

#include "ui/base/resource/resource_bundle.h"
#include "grit/theme_resources.h"
#include "grit/generated_resources.h"

#include "obs/obs_proxy/core_proxy/core_proxy_impl.h"
#include "obs/obs_proxy/public/proxy/obs_proxy.h"
#include "obs/obs_proxy/public/proxy/obs_scene_collection.h"


namespace
{
    enum SceneContentCtrlId
    {
        MoveUpButton,
        MoveDownButton,
        MoveToTopButton,
        MoveToBottomButton,

        ItemVsbButton,
        ItemLookButton,
        ItemEditButton,
        ItemDelButton,
        ItemLockButton
    };

    enum class SceneItemType : size_t {
        Image = 0,
        Slider,
        MediaSource,
        Text,
        Scene,
        ColiveSource,
        VoiceLinkSource,
        DmkhimeSource,
        DisplayCapture,
        WindowCapture,
        GameCapture,
        VideoCaptureDevice,
        AudioInputCapture,
        AudioOutputCapture,
        ColorSource,
        ReceiverSource,
        BrowserSource,
        RtcColiveSource,
        TextureRendererSource,
        VtuberSource,
        VtuberRendererSource,
    };

    const std::string kStormItemName = "storm-effect";

    class SceneItemRenameTextfield : public views::Textfield
    {
    public:
        SceneItemRenameTextfield(SceneItemRenameDelegate* controller)
            : delegate_(controller)
            , should_rename_(false)
            , is_init_(true)
        {
            DCHECK(delegate_);
            SetController(controller);
        }

        virtual ~SceneItemRenameTextfield()
        {
        }

        void SetText(const string16& text)
        {
            old_name_ = text;
            views::Textfield::SetText(text);
            should_rename_ = true;
            SelectAll(false);
            RequestFocus();
        }

    protected:
        // View
        virtual void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override
        {
            __super::ViewHierarchyChanged(details);

            if (details.is_add && native_wrapper_ && GetWidget() && is_init_)
            {
                is_init_ = false;
                SetFont(ftPrimary);
                SetColor(clrTextPrimary);
            }
        }

        bool OnKeyPressed(const ui::KeyEvent& event) override
        {
            if (event.key_code() == ui::KeyboardCode::VKEY_ESCAPE)
            {
                LoseFocus();
                return true;
            }
            else if (event.key_code() == ui::KeyboardCode::VKEY_RETURN)
            {
                should_rename_ = true;
                LoseFocus();
                return true;
            }

            return views::Textfield::OnKeyPressed(event);
        }

        void OnBlur() override
        {
            Finish();
        }

    private:
        void LoseFocus()
        {
            delegate_->SetFocus();
        }

        void Finish()
        {
            bool is_name_conflict = false;
            bool is_name_long = false;

            if (should_rename_)
            {
                // Check that the name meets the requirements
                should_rename_ = false;
                base::string16 new_name = text();
                for (auto x : new_name)
                {
                    if (x != ' ')
                    {
                        should_rename_ = true;
                    }
                }

                // Check whether the old and new names are different
                if (new_name == old_name_)
                {
                    should_rename_ = false;
                }
            }

            if (should_rename_)
            {
                do
                {
                    auto* core_proxy = OBSProxyService::GetInstance().GetOBSCoreProxy();
                    if (!core_proxy)
                        break;
                    auto scene_coll = core_proxy->GetCurrentSceneCollection();
                    if (!scene_coll)
                        break;
                    auto current_scene = scene_coll->GetScene(base::UTF16ToUTF8(delegate_->GetSceneName()));
                    if (!current_scene)
                        break;
                    auto target_item = current_scene->GetItem(base::UTF16ToUTF8(old_name_));
                    if (!target_item)
                        break;

                    is_name_conflict = !target_item->Rename(base::UTF16ToUTF8(text()));
                    is_name_long = text().length() >= 25 ? true : false;

                } while (false);
            }

            delegate_->OnFinishEdit();

            if (is_name_conflict)
            {
                livehime::ShowMessageBox(livehime::UniversalMsgboxType::CannotRenameSceneItem);
            }
            if (is_name_long)
            {
                livehime::ShowBubble(this, views::BubbleBorder::Arrow::BOTTOM_RIGHT, L"The number of words has reached its limit！");
            }

            should_rename_ = false;
        }

    private:
        bool should_rename_;
        base::string16 old_name_;
        bool is_init_;

        SceneItemRenameDelegate* delegate_;
    };

    void ExecuteSceneItemRelatedCommandWithParam(int cmd_id, SceneItemCmd &cmd_param)
    {
        bililive::ExecuteCommandWithParams(
            GetBililiveProcess()->bililive_obs(), cmd_id, CommandParams<SceneItemCmd>(&cmd_param));
    }

    void EventTracking(secret::LivehimeBehaviorEvent event_id) {
        auto secret_core = GetBililiveProcess()->secret_core();
        secret_core->event_tracking_service()->ReportLivehimeBehaviorEvent(
            event_id, secret_core->account_info().mid(), std::string()).Call();
    }
}

// TabAreaSceneContentView
TabAreaSceneContentViddupView::TabAreaSceneContentViddupView(int index, const base::string16 &name)
    : scene_index_(index)
    , scene_name_(name)
    , move_up_button_(nullptr)
    , move_down_button_(nullptr)
    , top_button_(nullptr)
    , bottom_button_(nullptr)
    , list_view_(nullptr)
    , weakptr_factory_(this)
{
}

TabAreaSceneContentViddupView::~TabAreaSceneContentViddupView()
{
}

bool TabAreaSceneContentViddupView::IsCurrentSceneSelected() const
{
    return visible();
}

void TabAreaSceneContentViddupView::EnsureSelectedItemVisible()
{
    TabAreaSceneItemViddupView* item_view = (TabAreaSceneItemViddupView*)list_view_->
        GetItem(list_view_->GetFirstSelectedItemPosition());
    if (item_view)
    {
        gfx::Rect vsb_bounds = list_view_->GetVisibleBounds();
        gfx::Rect item_bounds = item_view->bounds();
        if (!vsb_bounds.Contains(item_bounds))
        {
            list_view_->ScrollRectToVisible(item_bounds);
        }
    }
}

void TabAreaSceneContentViddupView::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            InitViews();

            notifation_register_.Add(this, bililive::NOTIFICATION_LIVEHIME_SCENE_ITEM_CMD_EXECUTED,
                                     base::NotificationService::AllSources());
            OBSProxyService::GetInstance().obs_ui_proxy()->AddObserver(this);
        }
        else
        {
            notifation_register_.RemoveAll();
            if (OBSProxyService::GetInstance().obs_ui_proxy())
            {
                OBSProxyService::GetInstance().obs_ui_proxy()->RemoveObserver(this);
            }
        }
    }
}

void TabAreaSceneContentViddupView::InitViews()
{
    set_background(views::Background::CreateSolidBackground(GetColor(WindowClient)));
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();

    views::GridLayout *layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    views::ColumnSet *col_set = layout->AddColumnSet(0);
    col_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    col_set->AddColumn(views::GridLayout::CENTER, views::GridLayout::CENTER, 0.2f, views::GridLayout::USE_PREF, 0, 0);
    col_set->AddColumn(views::GridLayout::CENTER, views::GridLayout::CENTER, 0.2f, views::GridLayout::USE_PREF, 0, 0);
    col_set->AddColumn(views::GridLayout::CENTER, views::GridLayout::CENTER, 0.2f, views::GridLayout::USE_PREF, 0, 0);
    col_set->AddColumn(views::GridLayout::CENTER, views::GridLayout::CENTER, 0.2f, views::GridLayout::USE_PREF, 0, 0);
    col_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

    col_set = layout->AddColumnSet(1);
    col_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);

    move_up_button_ = new views::ImageButton(this);
    move_up_button_->SetImage(views::Button::STATE_NORMAL, rb.GetImageSkiaNamed(IDR_LIVEHIME_V3_SCENEITEM_MOVE_UP));
    move_up_button_->SetImage(views::Button::STATE_HOVERED, rb.GetImageSkiaNamed(IDR_LIVEHIME_V3_SCENEITEM_MOVE_UP_HV));
    move_up_button_->SetImage(views::Button::STATE_PRESSED, rb.GetImageSkiaNamed(IDR_LIVEHIME_V3_SCENEITEM_MOVE_UP_HV));
    move_up_button_->SetTooltipText(rb.GetLocalizedString(IDS_TABAREA_SCENE_TIP_UP));
    move_up_button_->set_id(MoveUpButton);

    move_down_button_ = new views::ImageButton(this);
    move_down_button_->SetImage(views::Button::STATE_NORMAL, rb.GetImageSkiaNamed(IDR_LIVEHIME_V3_SCENEITEM_MOVE_DOWN));
    move_down_button_->SetImage(views::Button::STATE_HOVERED, rb.GetImageSkiaNamed(IDR_LIVEHIME_V3_SCENEITEM_MOVE_DOWN_HV));
    move_down_button_->SetImage(views::Button::STATE_PRESSED, rb.GetImageSkiaNamed(IDR_LIVEHIME_V3_SCENEITEM_MOVE_DOWN_HV));
    move_down_button_->SetTooltipText(rb.GetLocalizedString(IDS_TABAREA_SCENE_TIP_DOWN));
    move_down_button_->set_id(MoveDownButton);

    top_button_ = new views::ImageButton(this);
    top_button_->SetImage(views::Button::STATE_NORMAL, rb.GetImageSkiaNamed(IDR_LIVEHIME_V3_SCENEITEM_MOVE_TOP));
    top_button_->SetImage(views::Button::STATE_HOVERED, rb.GetImageSkiaNamed(IDR_LIVEHIME_V3_SCENEITEM_MOVE_TOP_HV));
    top_button_->SetImage(views::Button::STATE_PRESSED, rb.GetImageSkiaNamed(IDR_LIVEHIME_V3_SCENEITEM_MOVE_TOP_HV));
    top_button_->SetTooltipText(rb.GetLocalizedString(IDS_TABAREA_SCENE_TIP_TOP));
    top_button_->set_id(MoveToTopButton);

    bottom_button_ = new views::ImageButton(this);
    bottom_button_->SetImage(views::Button::STATE_NORMAL, rb.GetImageSkiaNamed(IDR_LIVEHIME_V3_SCENEITEM_MOVE_BOTTOM));
    bottom_button_->SetImage(views::Button::STATE_HOVERED, rb.GetImageSkiaNamed(IDR_LIVEHIME_V3_SCENEITEM_MOVE_BOTTOM_HV));
    bottom_button_->SetImage(views::Button::STATE_PRESSED, rb.GetImageSkiaNamed(IDR_LIVEHIME_V3_SCENEITEM_MOVE_BOTTOM_HV));
    bottom_button_->SetTooltipText(rb.GetLocalizedString(IDS_TABAREA_SCENE_TIP_BOTTOM));
    bottom_button_->set_id(MoveToBottomButton);

    list_view_ = new ListView(this);
    list_view_->SetEnabledItemDragMove(true);
    list_view_->SetItemSize(gfx::Size(kMainWndTabAreaWidth, kDataTitleHeight));
    list_view_->SetItemBackgroundColor(views::Button::STATE_NORMAL, GetColor(WindowClient));
    list_view_->SetItemBackgroundColor(views::Button::STATE_HOVERED, SkColorSetA(GetColor(WindowClient), 0.05f * 256));
    list_view_->SetItemBackgroundColor(views::Button::STATE_PRESSED, SkColorSetA(GetColor(WindowClient), 0.05f * 256));

    empty_view_ = new views::View();
    {
        views::GridLayout* layout = new views::GridLayout(empty_view_);
        empty_view_->SetLayoutManager(layout);

        auto col = layout->AddColumnSet(0);
        col->AddPaddingColumn(0, GetLengthByDPIScale(42));
        col->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
        col->AddPaddingColumn(0, GetLengthByDPIScale(42));

        BililiveLabel* tips_lab = new BililiveLabel();
        tips_lab->SetText(GetLocalizedString(IDS_SCENE_EMPTY_PLACEHOLDER));
        tips_lab->SetFont(ftTwelve);
        tips_lab->SetTextColor(SkColorSetA(clrTextALL, kMaskAlphaFour));
        layout->AddPaddingRow(1.0f, 0);
        layout->StartRow(0, 0);
        layout->AddView(tips_lab);
        layout->AddPaddingRow(1.0f, 0);
    }

    empty_view_->SetVisible(false);
    auto acttually_client_view = new BililiveSingleChildShowContainerView;
    acttually_client_view->AddChildView(list_view_->Container());
    acttually_client_view->AddChildView(empty_view_);

    layout->StartRow(1.0f, 1);
    layout->AddView(acttually_client_view);

    ShowEmptyView();
}

void TabAreaSceneContentViddupView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    int cmd_id = -1;
    switch (sender->id())
    {
    case MoveUpButton:
    {
        cmd_id = IDC_LIVEHIME_SCENE_ITEM_MOVEUP;
        EventTracking(secret::LivehimeBehaviorEvent::LivehimeSceneTop1);
        break;
    }
    case MoveDownButton:
    {
        cmd_id = IDC_LIVEHIME_SCENE_ITEM_MOVEDOWN;
        EventTracking(secret::LivehimeBehaviorEvent::LivehimeSceneLow1);
        break;
    }
    case MoveToTopButton:
    {
        cmd_id = IDC_LIVEHIME_SCENE_ITEM_TOTOP;
        EventTracking(secret::LivehimeBehaviorEvent::LivehimeSceneTop);
        break;
    }
    case MoveToBottomButton:
    {
        cmd_id = IDC_LIVEHIME_SCENE_ITEM_TOBOTTOM;
        EventTracking(secret::LivehimeBehaviorEvent::LivehimeSceneLow);
        break;
    }
    default:
        break;
    }
    if (cmd_id != -1)
    {
        TabAreaSceneItemViddupView* item_view = (TabAreaSceneItemViddupView*)list_view_->
            GetItem(list_view_->GetFirstSelectedItemPosition());
        if (item_view)
        {
            ExecuteSceneItemRelatedCommand(cmd_id, item_view->item_name());
        }
    }
}

void TabAreaSceneContentViddupView::OnItemSelectedChanged(ListItemView::ListItemViewChangeReason reason)
{
}

void TabAreaSceneContentViddupView::OnItemsOrderChanged(ListItemView::ListItemViewChangeReason reason)
{
    if(reason == ListItemView::CHANGED_BY_USER)
    {
        // After changing the order, get the order of the UI again, and then update the order 
        // of the underlying items according to the UI order
        SceneItemCmd cmd_params(new SCENEITEMCMD_(scene_name_, L""));
        auto item_views = list_view_->GetItemViews(true);
        for (auto item : item_views)
        {
            cmd_params->scene_items_.push_back(((TabAreaSceneItemViddupView*)item)->item_name());
        }
        if (!cmd_params->scene_items().empty())
        {
            cmd_params->item_name_ = cmd_params->scene_items_[0];
            ExecuteSceneItemRelatedCommandWithParam(IDC_LIVEHIME_SCENE_ITEM_REORDER, cmd_params);
        }
    }
}

bool TabAreaSceneContentViddupView::FindListItem(const LVFINDITEMINFO* pFindInfo, ListItemView* item_view)
{
    if (pFindInfo->wtext == ((TabAreaSceneItemViddupView*)item_view)->item_name())
    {
        return true;
    }

    return false;
}

void TabAreaSceneContentViddupView::Observe(int type, const base::NotificationSource& source, const base::NotificationDetails& details)
{
    switch (type)
    {
    case bililive::NOTIFICATION_LIVEHIME_SCENE_ITEM_CMD_EXECUTED:
    {
        if (IsCurrentSceneSelected())
        {
            TabAreaSceneItemViddupView* item_view = (TabAreaSceneItemViddupView*)list_view_->
                GetItem(list_view_->GetFirstSelectedItemPosition());
            if (item_view)
            {
                int cmd_id = *base::Details<int>(details).ptr();
                ExecuteSceneItemRelatedCommand(cmd_id, item_view->item_name());
            }
        }
    }
    break;
    default:
        break;
    }
}

void TabAreaSceneContentViddupView::ExecuteSceneItemRelatedCommand(int cmd_id, const string16 &item_name)
{
    if (cmd_id == IDC_LIVEHIME_SCENE_ITEM_DELETE_FROM_PREVIEW)
    {
        cmd_id = IDC_LIVEHIME_SCENE_ITEM_DELETE;
    }

    SceneItemCmd cmd_params(new SCENEITEMCMD_(scene_name_, item_name));
    ExecuteSceneItemRelatedCommandWithParam(cmd_id, cmd_params);
}

void TabAreaSceneContentViddupView::OnSourceRename(const base::string16& prev_item_name, const base::string16& new_item_name)
{
    LVFINDITEMINFO fi;
    fi.wtext = prev_item_name;
    TabAreaSceneItemViddupView* item_view = (TabAreaSceneItemViddupView*)list_view_->GetItem(list_view_->FindItem(&fi));
    if (item_view)
    {
        item_view->SetItemName(new_item_name);
    }
}

void TabAreaSceneContentViddupView::OnSceneItemAdd(const base::string16& scene_name, const obs_proxy_ui::TableItem& table_item, obs_source_t* source, int source_type)
{
    if ((scene_name_ != scene_name) || !list_view_)
    {
        return;
    }

    list_view_->AddItemViewAt(new TabAreaSceneItemViddupView(list_view_, scene_name_, table_item, source_type), 0);

    ShowOrHideCanvas(false);
    ShowEmptyView();
}

void TabAreaSceneContentViddupView::OnSceneItemRemove(const base::string16& scene_name, const base::string16& item_name)
{
    if ((scene_name_ != scene_name) || !list_view_)
    {
        return;
    }

    LVFINDITEMINFO fi;
    fi.wtext = item_name;
    int index = list_view_->FindItem(&fi);
    if (-1 != index)
    {
        bool is_item_checked = false;
        ListItemView* item_view = list_view_->DeleteItem(index);
        if (item_view)
        {
            is_item_checked = item_view->checked();
            delete item_view;
        }

        // After deleting the currently selected source, sets the next item of that source as the selected item
        if(is_item_checked)
        {
            TabAreaSceneItemViddupView* item_view = (TabAreaSceneItemViddupView*)list_view_->GetItem(index);
            if (item_view)
            {
                BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
                                         base::Bind(&TabAreaSceneContentViddupView::ExecuteSceneItemRelatedCommand,
                                                    weakptr_factory_.GetWeakPtr(),
                                                    IDC_LIVEHIME_SCENE_ITEM_SELECT, item_view->item_name()));
            }
        }
        if (base::UTF8ToUTF16(kStormItemName) == item_name)
        {
            if (media_view_)
            {
                media_view_ = nullptr;
            }
        }
    }
    else
    {
        NOTREACHED();
    }

    if (list_view_->GetItemCount() < 1)
    {
        ShowOrHideCanvas(true);
    }

    ShowEmptyView();
}

void TabAreaSceneContentViddupView::RemoveAllSceneItem()
{
    if (list_view_)
    {
        std::vector<ListItemView*> item_list = list_view_->GetItemViews();
        for (auto it= item_list.begin(); it!= item_list.end(); it++)
        {
            TabAreaSceneItemViddupView* item_view = (TabAreaSceneItemViddupView*)(*it);
            item_view->RemoveSceneItem();
            break;
        }
    }
}

void TabAreaSceneContentViddupView::AddVtuberStromSceneItem(const std::string& mp4_path, int storm_type)
{
    if (media_view_)
    {
        return;
    }

    int vtuber_floor = 0;           // The last level of virtual character subscript
    int count = 0;
    bool find = false;
    auto scene_collection = OBSProxyService::GetInstance().GetOBSCoreProxy()->GetCurrentSceneCollection();
    obs_proxy::Scene* scene = scene_collection->current_scene();
    std::vector<obs_proxy::SceneItem*> scene_list = scene->GetItems();
    std::vector<ListItemView*> item_list = list_view_->GetItemViews();


    if (vtuber_floor>=0)
    {
        auto source = scene->AttachNewItem(obs_proxy::SceneItemType::MediaSource, kStormItemName, true);
        // Full-screen adaptation
        source->SetFitToScreen(true);
        source->SetVisible(false);
        // Hierarchical adjustment
        if (vtuber_floor > 0) {
            SortStormItem(vtuber_floor);
        }

        // Effect addition
        if (!media_view_)
        {
            media_view_ = new MediaPropDetailView(source);
        }

        using namespace std::placeholders;
        auto handle = std::bind(&TabAreaSceneContentViddupView::OnMediaStateChangeCallBack, this, _1);
        if (media_view_) {
            media_view_->SetVtuber(mp4_path, handle);
        }

        BililiveThread::PostDelayedTask(
            BililiveThread::UI, FROM_HERE,
            base::Bind(&TabAreaSceneContentViddupView::StartShowStormMp4, weakptr_factory_.GetWeakPtr(), source), base::TimeDelta::FromMilliseconds(600));
    }
}

void TabAreaSceneContentViddupView::StartShowStormMp4(obs_proxy::SceneItem* scene_item)
{
    if (media_view_ && scene_item)
    {
        scene_item->SetVisible(true);
    }
}

void TabAreaSceneContentViddupView::OnMediaStateChangeCallBack(bool play_end)
{
    std::vector<ListItemView*> item_list = list_view_->GetItemViews();
    for (auto item : item_list)
    {
        TabAreaSceneItemViddupView* item_view = (TabAreaSceneItemViddupView*)item;
        if (item_view->item_name() == base::UTF8ToUTF16(kStormItemName))
        {
            item_view->SceneItemDelete();
            break;
        }
    }

}

void TabAreaSceneContentViddupView::SortStormItem(int vtuber_floor)
{
    TabAreaSceneItemViddupView* item_view = (TabAreaSceneItemViddupView*)list_view_->GetItem(0);
    if (item_view && item_view->item_name() == base::UTF8ToUTF16(kStormItemName))
    {
        for (int i = 0; i < vtuber_floor; i++) {
            ExecuteSceneItemRelatedCommand(IDC_LIVEHIME_SCENE_ITEM_MOVEDOWN, item_view->item_name());
        }

        item_view->LimitItemEdit();
    }
}

void TabAreaSceneContentViddupView::OnSceneItemSelected(const base::string16& scene_name, const base::string16& item_name)
{
    if ((scene_name_ != scene_name) || !list_view_)
    {
        return;
    }

    LVFINDITEMINFO fi;
    fi.wtext = item_name;
    int index = list_view_->FindItem(&fi);
    if (-1 != index)
    {
        list_view_->SetCheck(index, true);

        EnsureSelectedItemVisible();
    }
    else
    {
        NOTREACHED();
    }
}

void TabAreaSceneContentViddupView::OnSceneItemDeselect(const base::string16& scene_name, const base::string16& item_name)
{
    if ((scene_name_ != scene_name) || !list_view_)
    {
        return;
    }

    LVFINDITEMINFO fi;
    fi.wtext = item_name;
    int index = list_view_->FindItem(&fi);
    if (-1 != index)
    {
        list_view_->SetCheck(index, false);
    }
    else
    {
        NOTREACHED();
    }
}

void TabAreaSceneContentViddupView::OnSceneItemReordered(const base::string16& scene_name, const std::vector<string16> &item_names)
{
    if ((scene_name_ != scene_name) || !list_view_)
    {
        return;
    }

    DCHECK((int)item_names.size() == list_view_->GetItemCount());

    if(!list_view_->IsItemDragging())
    {
        LVFINDITEMINFO fi;
        ListItemView* item_view = nullptr;
        std::vector<ListItemView*> item_views;
        for (auto &name : item_names)
        {
            fi.wtext = name;
            item_view = list_view_->GetItem(list_view_->FindItem(&fi));
            if (item_view)
            {
                item_views.push_back(item_view);
            }
            else
            {
                NOTREACHED();
            }
        }

        for (int i = 0, count = item_views.size(); i < count; i++)
        {
            list_view_->SetItemViewIndex(item_views[i], i);
        }

        list_view_->Layout();

        EnsureSelectedItemVisible();
    }
}

void TabAreaSceneContentViddupView::OnSceneItemVisible(const base::string16& scene_name, const base::string16& item_name, bool visible)
{
    if ((scene_name_ != scene_name) || !list_view_)
    {
        return;
    }

    LVFINDITEMINFO fi;
    fi.wtext = item_name;
    TabAreaSceneItemViddupView* item_view = (TabAreaSceneItemViddupView*)list_view_->GetItem(list_view_->FindItem(&fi));
    if (item_view)
    {
        item_view->SetItemVisible(visible);
    }
    else
    {
        NOTREACHED();
    }
}

void TabAreaSceneContentViddupView::OnSceneItemLocked(const base::string16& scene_name, const base::string16& item_name, bool locked)
{
    if ((scene_name_ != scene_name) || !list_view_)
    {
        return;
    }

    LVFINDITEMINFO fi;
    fi.wtext = item_name;
    TabAreaSceneItemViddupView* item_view = (TabAreaSceneItemViddupView*)list_view_->GetItem(list_view_->FindItem(&fi));
    if (item_view)
    {
        item_view->SetItemLocked(locked);
    }
    else
    {
        NOTREACHED();
    }
}

gfx::Rect TabAreaSceneContentViddupView::GetCameraItemEditBtnBounds(const std::string& name) const
{
    gfx::Rect rect;
    if (list_view_)
    {
        string16 item_name = base::UTF8ToUTF16(name);

        for (int i = 0; i < list_view_->GetItemCount(); i++)
        {
            TabAreaSceneItemViddupView*item_view = (TabAreaSceneItemViddupView*)list_view_->GetItem(i);
            if (item_view && item_view->item_name() == item_name)
            {
                rect = item_view->GetCameraItemEditBtnBounds();
                break;
            }
        }
    }

    return rect;
}

void TabAreaSceneContentViddupView::ShowOperateView(const std::string& name,bool show)
{
    if (list_view_)
    {
        string16 item_name = base::UTF8ToUTF16(name);

        for (int i = 0; i < list_view_->GetItemCount(); i++)
        {
            TabAreaSceneItemViddupView*item_view = (TabAreaSceneItemViddupView*)list_view_->GetItem(i);
            if (item_view && item_view->item_name() == item_name)
            {
                item_view->ShowOperateView(show);
                break;
            }
        }
    }
}

void TabAreaSceneContentViddupView::ShowOrHideCanvas(bool show_canvas)
{
    if (show_canvas)
    {
        if (list_view_->GetItemCount() < 1)
        {
            bool land_model = LiveModelController::GetInstance()->IsLandscapeModel();

            std::unique_ptr<CanvasMaterialsViddupView> cmv(new CanvasMaterialsViddupView(this, land_model));

            add_materials_view_seq_id_ = livehime::ShowAcceptEventPreviewArea(livehime::PAEFWH_First, cmv.get());
            if (-1 != add_materials_view_seq_id_)
            {
                cmv.release();
            }
         }
    }
    else
    {
        if (-1 != add_materials_view_seq_id_)
        {
            livehime::HideAcceptEventPreviewArea(livehime::PAEFWH_First, add_materials_view_seq_id_, true);
            add_materials_view_seq_id_ = -1;
        }
    }
}

void TabAreaSceneContentViddupView::ShowChatRoomView(bool show_canvas)
{
    if (show_canvas)
    {

    }
    else
    {
        if (chat_room_view_seq_id_ != -1)
        {
            livehime::HideAcceptEventPreviewArea(livehime::PFLFWH_ChatRoom, chat_room_view_seq_id_, true);
            chat_room_view_seq_id_ = -1;
            //chat_room_main_view_.reset();
        }
    }
}

void  TabAreaSceneContentViddupView::ShowEmptyView()
{
    if (list_view_->GetItemCount() < 1) {
        empty_view_->SetVisible(true);
        list_view_->SetVisible(false);
    }
    else {
        empty_view_->SetVisible(false);
        list_view_->SetVisible(true);
    }

    Layout();
}


// TabAreaSceneItemView
TabAreaSceneItemViddupView::TabAreaSceneItemViddupView(ListView *list_view, const base::string16& scene_name,
                                           const obs_proxy_ui::TableItem& table_item, int source_type)
    : ListItemView(list_view)
    , SceneItemMenu(this)
    , scene_name_(scene_name)
    , item_info_(table_item)
    , icon_button_(nullptr)
    , name_view_(nullptr)
    , item_name_label_(nullptr)
    , rename_textfield_(nullptr)
    , operate_view_(nullptr)
    , edit_button_(nullptr)
    , del_button_(nullptr)
    , lock_button_(nullptr)
    , look_button_(nullptr)
    , source_type_(source_type)
    , should_start_rename_timer_(false)
    , weakptr_factory_(this)
{
    set_notify_enter_exit_on_child(true);
    set_focusable(true);
    set_focus_border(false);
}

TabAreaSceneItemViddupView::~TabAreaSceneItemViddupView()
{
}

void TabAreaSceneItemViddupView::SetItemName(const base::string16& item_name)
{
    item_info_.set_contents(item_name);
    item_name_label_->SetText(item_info_.contents_);
}

void TabAreaSceneItemViddupView::SetItemVisible(bool visible)
{
    item_info_.set_visible(visible);
    look_button_->SetTooltipText(GetLocalizedString(visible ? IDS_TABAREA_SCENE_TIP_HIDE : IDS_TABAREA_SCENE_TIP_SHOW));
    look_button_->SetImage(views::Button::STATE_NORMAL,
                           GetImageSkiaNamed(item_info_.visible_ ?
                               IDR_LIVEHIME_SEE_NORMAL : IDR_LIVEHIME_HIDE_NORMAL));
    look_button_->SetImage(views::Button::STATE_HOVERED,
                           GetImageSkiaNamed(item_info_.visible_ ?
                               IDR_LIVEHIME_SEE_HOV : IDR_LIVEHIME_HIDE_HOV));
    look_button_->SetImage(views::Button::STATE_PRESSED,
                           GetImageSkiaNamed(item_info_.visible_ ?
                               IDR_LIVEHIME_SEE_HOV : IDR_LIVEHIME_HIDE_HOV));
}

void TabAreaSceneItemViddupView::SetItemLocked(bool lock)
{
    item_info_.set_locked(lock);
    lock_button_->SetTooltipText(GetLocalizedString(lock ? IDS_TABAREA_SCENE_TIP_UNLOCK : IDS_TABAREA_SCENE_TIP_LOCK));
    lock_button_->SetImage(views::Button::STATE_NORMAL,
                           GetImageSkiaNamed(item_info_.locked_ ?
                               IDR_LIVEHIME_UNLOCK_NORMAL : IDR_LIVEHIME_LOCK_NORMAL));
    lock_button_->SetImage(views::Button::STATE_HOVERED,
                           GetImageSkiaNamed(item_info_.locked_ ?
                               IDR_LIVEHIME_UNLOCK_HOV : IDR_LIVEHIME_LOCK_HOV));
    lock_button_->SetImage(views::Button::STATE_PRESSED,
                           GetImageSkiaNamed(item_info_.locked_ ?
                               IDR_LIVEHIME_UNLOCK_HOV : IDR_LIVEHIME_LOCK_HOV));
}

void TabAreaSceneItemViddupView::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details)
{
    if (details.is_add && details.child == this)
    {
        InitViews();
    }
}

void TabAreaSceneItemViddupView::InitViews()
{
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();
    views::GridLayout *layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    views::ColumnSet *column_set = layout->AddColumnSet(0);
    column_set->AddPaddingColumn(0, GetLengthByDPIScale(12));
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForCtrlTips);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 1.0f, views::GridLayout::FIXED, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForCtrlTips);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, GetLengthByDPIScale(12));

    icon_button_ = new BililiveImageButton(this);
    icon_button_->set_id(ItemVsbButton);
    //icon_button_->SetTooltipText(GetLocalizedString(IDS_TABAREA_SCENE_TIP_HIDE));
    icon_button_->SetImage(views::Button::STATE_NORMAL, GetImageSkia(source_type_));

    name_view_ = new BililiveSingleChildShowContainerView();
    {
        item_name_label_ = new LivehimeContentLabel(item_info_.contents_);
        item_name_label_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
        item_name_label_->SetFont(ftTwelve);
        rename_textfield_ = new SceneItemRenameTextfield(this);
        rename_textfield_->SetVisible(false);
        rename_textfield_->SetFont(ftTwelve);

        name_view_->AddChildView(item_name_label_);
        name_view_->AddChildView(rename_textfield_);
    }

    operate_view_ = new BililiveHideAwareView();
    operate_view_->SetVisible(false);
    {
        operate_view_->SetLayoutManager(new views::BoxLayout(views::BoxLayout::kHorizontal, 0, 0, kPaddingColWidthForCtrlTips));

        look_button_ = new BililiveImageButton(this);
        look_button_->set_id(ItemLookButton);
        look_button_->SetTooltipText(GetLocalizedString(IDS_TABAREA_SCENE_TIP_HIDE));
        look_button_->SetImage(views::Button::STATE_NORMAL, GetImageSkiaNamed(IDR_LIVEHIME_SEE_NORMAL));
        look_button_->SetImage(views::Button::STATE_HOVERED, GetImageSkiaNamed(IDR_LIVEHIME_SEE_HOV));
        look_button_->SetImage(views::Button::STATE_PRESSED, GetImageSkiaNamed(IDR_LIVEHIME_SEE_HOV));

        edit_button_ = new BililiveImageButton(this);
        edit_button_->set_id(ItemEditButton);
        edit_button_->SetImage(views::Button::STATE_NORMAL, GetImageSkiaNamed(IDR_LIVEHIME_EDIT_NORMAL));
        edit_button_->SetImage(views::Button::STATE_HOVERED, GetImageSkiaNamed(IDR_LIVEHIME_EDIT_HOV));
        edit_button_->SetImage(views::Button::STATE_PRESSED, GetImageSkiaNamed(IDR_LIVEHIME_EDIT_HOV));
        edit_button_->SetTooltipText(GetLocalizedString(IDS_TABAREA_SCENE_TIP_EDIT));

        del_button_ = new BililiveImageButton(this);
        del_button_->set_id(ItemDelButton);
        del_button_->SetImage(views::Button::STATE_NORMAL, GetImageSkiaNamed(IDR_LIVEHIME_DELETE_NORMAL));
        del_button_->SetImage(views::Button::STATE_HOVERED, GetImageSkiaNamed(IDR_LIVEHIME_DELETE_HOV));
        del_button_->SetImage(views::Button::STATE_PRESSED, GetImageSkiaNamed(IDR_LIVEHIME_DELETE_HOV));
        del_button_->SetTooltipText(rb.GetLocalizedString(IDS_TABAREA_SCENE_TIP_DEL));

        lock_button_ = new BililiveImageButton(this);
        lock_button_->set_id(ItemLockButton);
        lock_button_->SetImage(views::Button::STATE_NORMAL, GetImageSkiaNamed(IDR_LIVEHIME_LOCK_NORMAL));
        lock_button_->SetImage(views::Button::STATE_HOVERED, GetImageSkiaNamed(IDR_LIVEHIME_LOCK_HOV));
        lock_button_->SetImage(views::Button::STATE_PRESSED, GetImageSkiaNamed(IDR_LIVEHIME_LOCK_HOV));
        lock_button_->SetTooltipText(GetLocalizedString(IDS_TABAREA_SCENE_TIP_LOCK));

        operate_view_->AddChildView(look_button_);
        operate_view_->AddChildView(lock_button_);
        operate_view_->AddChildView(del_button_);
        operate_view_->AddChildView(edit_button_);
    }

    layout->StartRow(1.0f, 0);
    layout->AddView(icon_button_);
    layout->AddView(name_view_);
    layout->AddView(operate_view_);

    SetItemVisible(item_info_.visible_);
    SetItemLocked(item_info_.locked_);
    SetChecked(item_info_.selected_);

    if (item_info_.is_browser_plugin()) {
        
    }
}

void TabAreaSceneItemViddupView::OnMouseEntered(const ui::MouseEvent& event)
{
    __super::OnMouseEntered(event);

    if (IsMouseEntered())
    {
        ShowOperateView(true);
    }
}

void TabAreaSceneItemViddupView::OnMouseExited(const ui::MouseEvent& event)
{
    __super::OnMouseExited(event);

    ShowOperateView(false);
}

bool TabAreaSceneItemViddupView::OnMousePressed(const ui::MouseEvent& event)
{
    should_start_rename_timer_ = false;
    rename_click_timer_.Stop();

    if (event.IsOnlyRightMouseButton())
    {
        __super::OnMousePressed(event);

        gfx::Point location(event.location());
        if (HitTestPoint(location))
        {
            ConvertPointToScreen(this, &location);
            views::View::ShowContextMenu(location, ui::MENU_SOURCE_MOUSE);
        }

        return false;
    }

    if (event.IsOnlyLeftMouseButton())
    {
        bool before_mouse_pressed_checked_status = checked();

        __super::OnMousePressed(event);

        int count = event.GetClickCount();
        if (count >= 2)
        {
            if (before_mouse_pressed_checked_status)
            {
                should_start_rename_timer_ = true;
            }
        }

        return true;
    }

    return __super::OnMousePressed(event);
}

bool TabAreaSceneItemViddupView::OnMouseDragged(const ui::MouseEvent& event)
{
    should_start_rename_timer_ = false;

    if (__super::OnMouseDragged(event))
    {
        return true;
    }

    rename_click_timer_.Stop();
    return false;
}

void TabAreaSceneItemViddupView::OnMouseReleased(const ui::MouseEvent& event)
{
    __super::OnMouseReleased(event);

    if (event.IsOnlyLeftMouseButton())
    {
        if (should_start_rename_timer_ && name_view_->bounds().Contains(event.location()))
        {
            // The timing interval should be greater than the frame's time interval for double - click determination, 
            // see ui::MouseEvent::IsRepeatedClickEvent().kDoubleClickTimeMS，These values match the Windows defaults.
            static const int kDoubleClickTimeMS = 50;
            rename_click_timer_.Start(FROM_HERE,
                                        base::TimeDelta::FromMilliseconds(kDoubleClickTimeMS),
                                        base::Bind(&TabAreaSceneItemViddupView::ShowRenameTextfield,
                                                    weakptr_factory_.GetWeakPtr(), true));
        }
    }
}

void TabAreaSceneItemViddupView::OnKeyEvent(ui::KeyEvent* event)
{
    ui::KeyboardCode key_c = event->key_code();
    if (event->type() == ui::EventType::ET_KEY_PRESSED || event->type() == ui::EventType::ET_KEY_RELEASED)
    {
        if (key_c == VK_DELETE)
        {
            ExecuteSceneItemRelatedCommand(IDC_LIVEHIME_SCENE_ITEM_DELETE);
        }
    }
}

void TabAreaSceneItemViddupView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    int cmd_id = -1;
    int button_id = sender->id();
    int switch_type = 0;

    switch (button_id)
    {
    case ItemLookButton:
        cmd_id = IDC_LIVEHIME_SCENE_ITEM_VISIBLE;
        switch_type = item_info_.visible_ ? 1 : 2;
        break;
    case ItemEditButton:
        if (IsBrowserPlugin())
        {
            cmd_id = IDC_LIVEHIME_BROWSER_SCENE_SET;
        }
        else {
            cmd_id = IDC_LIVEHIME_SCENE_ITEM_PROPERTY;
        }
        break;
    case ItemDelButton:
        cmd_id = IDC_LIVEHIME_SCENE_ITEM_DELETE;
        break;
    case ItemLockButton:
        cmd_id = IDC_LIVEHIME_SCENE_ITEM_LOCK;
        switch_type = item_info_.locked_ ? 2 : 1;
        break;
    default:
        break;
    }

    if(-1 != cmd_id)
    {
        ExecuteSceneItemRelatedCommand(cmd_id);
        EventPolaris(button_id, switch_type);
    }
}

bool TabAreaSceneItemViddupView::IsBrowserPlugin()
{
    if (source_type_ != (int)obs_proxy::SceneItemType::BrowserSource)
    {
        return false;
    }

    if (plugin_id_!=0)
    {
        return true;
    }

    auto collection = obs_proxy::GetCoreProxy()->GetCurrentSceneCollection();
    obs_proxy::Scene* scene = collection->GetScene(base::UTF16ToUTF8(scene_name_));
    DCHECK(scene);
    if (scene)
    {
        obs_proxy::SceneItem* scene_item = scene->GetItem(base::UTF16ToUTF8(item_name()));
        DCHECK(scene_item);
        if (scene_item)
        {
            if (scene_item->type() == obs_proxy::SceneItemType::BrowserSource)
            {
                auto scene_item_helper_ = std::make_unique<BrowserItemHelper>(scene_item);
                if (scene_item_helper_->GetIsPlugin())
                {
                    plugin_id_ = scene_item_helper_->GetPluginId();
                    return true;
                }
            }
        }
    }

    return false;
}

void TabAreaSceneItemViddupView::RemoveSceneItem()
{
    ExecuteSceneItemRelatedCommand(IDC_LIVEHIME_ALL_SCENE_ITEM_DELETE);
}

void TabAreaSceneItemViddupView::OnSelectedChanged(ListItemViewChangeReason reason)
{
    if (reason == ListItemView::CHANGED_BY_USER)
    {
        ExecuteSceneItemRelatedCommand(checked() ?
                                       IDC_LIVEHIME_SCENE_ITEM_SELECT :
                                       IDC_LIVEHIME_SCENE_ITEM_DESELECT);
    }
}

base::string16 TabAreaSceneItemViddupView::GetSceneName() const
{
    return scene_name_;
}

void TabAreaSceneItemViddupView::SetFocus()
{
    RequestFocus();
}

void TabAreaSceneItemViddupView::OnFinishEdit()
{
    base::MessageLoop::current()->PostTask(FROM_HERE,
                                            base::Bind(&TabAreaSceneItemViddupView::ShowRenameTextfield,
                                                       weakptr_factory_.GetWeakPtr(), false));
}

void TabAreaSceneItemViddupView::ContentsChanged(views::Textfield* sender, const string16& new_contents)
{
    gfx::Rect vsb_bounds = list_view()->GetVisibleBounds();
    if (!vsb_bounds.Contains(bounds()))
    {
        list_view()->ScrollRectToVisible(bounds());
    }
}

void TabAreaSceneItemViddupView::ShowOperateView(bool show)
{
    bool old_status = operate_view_->visible();
    if (rename_textfield_->visible())
    {
        operate_view_->SetVisible(false);
    }
    else
    {
        operate_view_->SetVisible(show);
    }
    if(old_status != operate_view_->visible())
    {
        InvalidateLayout();
        Layout();
    }
}

void TabAreaSceneItemViddupView::ShowRenameTextfield(bool show)
{
    item_name_label_->SetVisible(!show);
    rename_textfield_->SetVisible(show);
    if (show)
    {
        operate_view_->SetVisible(false);
        rename_textfield_->SetText(item_name_label_->text());
        rename_textfield_->SelectAll(false);
        rename_textfield_->RequestFocus();

        auto secret_core = GetBililiveProcess()->secret_core();
        secret_core->event_tracking_service()->ReportLivehimeBehaviorEvent(
            secret::LivehimeBehaviorEvent::LivehimeSceneRename,
            secret_core->account_info().mid(), std::string()).Call();
    }
    else
    {
        operate_view_->SetVisible(IsMouseEntered());
    }

    InvalidateLayout();
    Layout();
}

void TabAreaSceneItemViddupView::SceneItemDelete()
{
    SceneItemCmd cmd_params;
    cmd_params.reset(new SCENEITEMCMD_(scene_name_, item_name()));
    cmd_params->no_popup = true;
    ExecuteSceneItemRelatedCommandWithParam(IDC_LIVEHIME_SCENE_ITEM_DELETE, cmd_params);
}

void TabAreaSceneItemViddupView::LimitItemEdit()
{
    if (edit_button_)
    {
        edit_button_->SetEnabled(false);
    }
}

void TabAreaSceneItemViddupView::ExecuteSceneItemRelatedCommand(int cmd_id)
{
    SceneItemCmd cmd_params;
    cmd_params.reset(new SCENEITEMCMD_(scene_name_, item_name()));
    ExecuteSceneItemRelatedCommandWithParam(cmd_id, cmd_params);
}

gfx::Rect TabAreaSceneItemViddupView::GetCameraItemEditBtnBounds() const
{
    if (!edit_button_)
        return gfx::Rect();

    return edit_button_->GetBoundsInScreen();
}

void TabAreaSceneItemViddupView::EventPolaris(int type, int switch_type)
{
    std::vector<std::string> vec_outcome;
    if (source_type_ == (int)obs_proxy::SceneItemType::BrowserSource) {
        if (IsBrowserPlugin()) {
            vec_outcome.push_back(base::StringPrintf("material_type:%d", (size_t)secret::BehaviorEventMaterialType::Plugin));
            vec_outcome.push_back(base::StringPrintf("plugin_id:%lld", plugin_id_));
        }
        else {
            vec_outcome.push_back(base::StringPrintf("material_type:%d", (size_t)secret::BehaviorEventMaterialType::Browser));
        }
    }
    else if (source_type_ == (int)obs_proxy::SceneItemType::DisplayCapture) {
        vec_outcome.push_back(base::StringPrintf("material_type:%d", (size_t)secret::BehaviorEventMaterialType::Monitor));
    }
    else if (source_type_ == (int)obs_proxy::SceneItemType::WindowCapture) {
        vec_outcome.push_back(base::StringPrintf("material_type:%d", (size_t)secret::BehaviorEventMaterialType::Window));
    }
    else if (source_type_ == (int)obs_proxy::SceneItemType::GameCapture) {
        vec_outcome.push_back(base::StringPrintf("material_type:%d", (size_t)secret::BehaviorEventMaterialType::Game));
    }
    else if (source_type_ == (int)obs_proxy::SceneItemType::Text) {
        vec_outcome.push_back(base::StringPrintf("material_type:%d", (size_t)secret::BehaviorEventMaterialType::Text));
    }
    else if (source_type_ == (int)obs_proxy::SceneItemType::Image) {
        vec_outcome.push_back(base::StringPrintf("material_type:%d", (size_t)secret::BehaviorEventMaterialType::Image));
    }
    else if (source_type_ == (int)obs_proxy::SceneItemType::VideoCaptureDevice) {
        vec_outcome.push_back(base::StringPrintf("material_type:%d", (size_t)secret::BehaviorEventMaterialType::Camera));
    }
    else if (source_type_ == (int)obs_proxy::SceneItemType::MediaSource) {
        vec_outcome.push_back(base::StringPrintf("material_type:%d", (size_t)secret::BehaviorEventMaterialType::Media));
    }
    else if (source_type_ == (int)obs_proxy::SceneItemType::ReceiverSource) {
        vec_outcome.push_back(base::StringPrintf("material_type:%d", (size_t)secret::BehaviorEventMaterialType::Projection));
    }
    else if (source_type_ == (int)obs_proxy::SceneItemType::DmkhimeSource) {
        vec_outcome.push_back(base::StringPrintf("material_type:%d", (size_t)secret::BehaviorEventMaterialType::DanmakuHime));
    }
    else if (source_type_ == (int)obs_proxy::SceneItemType::Scene) {
        vec_outcome.push_back(base::StringPrintf("material_type:%d", (size_t)secret::BehaviorEventMaterialType::Vtuber));
    }
    else if (source_type_ == (int)obs_proxy::SceneItemType::AudioInputCapture) {
        vec_outcome.push_back(base::StringPrintf("material_type:%d", (size_t)secret::BehaviorEventMaterialType::Microphone));
    }
    else if (source_type_ == (int)obs_proxy::SceneItemType::AudioOutputCapture) {
        vec_outcome.push_back(base::StringPrintf("material_type:%d", (size_t)secret::BehaviorEventMaterialType::Loudspeaker));
    }

    if (switch_type != 0)
    {
        vec_outcome.push_back(base::StringPrintf("switch_type:%d", switch_type));
    }

    std::string outcome = JoinString(vec_outcome, ";");

    switch (type)
    {
    case ItemLookButton:
        livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::SceneHideClick, outcome);
        break;
    case ItemEditButton:
        livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::SceneEditClick, outcome);
        break;
    case ItemDelButton:
        livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::SceneDeleteClick, outcome);
        break;
    case ItemLockButton:
        livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::SceneLockClick, outcome);
        break;
    }
}

gfx::ImageSkia* TabAreaSceneItemViddupView::GetImageSkia(int type) {

    gfx::ImageSkia* image_skia = nullptr;
    switch (static_cast<SceneItemType>(type))
    {
    case SceneItemType::VideoCaptureDevice:
        image_skia = GetImageSkiaNamed(IDR_LIVEHIME_SOURCE_CAM);
        break;
    case SceneItemType::Text:
        image_skia = GetImageSkiaNamed(IDR_LIVEHIME_SOURCE_TEXT);
        break;
    case SceneItemType::DisplayCapture:
        image_skia = GetImageSkiaNamed(IDR_LIVEHIME_SOURCE_MONITOR);
        break;
    case SceneItemType::AudioOutputCapture:
        image_skia = GetImageSkiaNamed(IDR_LIVEHIME_SOURCE_SPEAKER);
        break;
    case SceneItemType::AudioInputCapture:
        image_skia = GetImageSkiaNamed(IDR_LIVEHIME_SOURCE_MICRO);
        break;
    case SceneItemType::GameCapture:
        image_skia = GetImageSkiaNamed(IDR_LIVEHIME_SOURCE_GAME);
        break;
    case SceneItemType::MediaSource:
        image_skia = GetImageSkiaNamed(IDR_LIVEHIME_SOURCE_MEDIA);
        break;
    case SceneItemType::Image:
    case SceneItemType::Slider:
    case SceneItemType::ColorSource:
        image_skia = GetImageSkiaNamed(IDR_LIVEHIME_SOURCE_IMAGE);
        break;
    case SceneItemType::WindowCapture:
        image_skia = GetImageSkiaNamed(IDR_LIVEHIME_SOURCE_WINDOW);
        break;
    case SceneItemType::BrowserSource:
        image_skia = GetImageSkiaNamed(IDR_LIVEHIME_SOURCE_BROWSER);
        break;
    case SceneItemType::DmkhimeSource:
        image_skia = GetImageSkiaNamed(IDR_LIVEHIME_SOURCE_BULLET);
        break;
    case SceneItemType::ReceiverSource:
        image_skia = GetImageSkiaNamed(IDR_LIVEHIME_SOURCE_SCREEN);
        break;
    case SceneItemType::Scene:
        image_skia = GetImageSkiaNamed(IDR_LIVEHIME_SOURCE_VIRTUAL);
        break;

    default:
        break;
    }

    return image_skia;
}