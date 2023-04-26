#include "bottom_area_view.h"

#include "bililive/bililive/bililive_database.h"
#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/ui/bililive_commands.h"
#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/controls/bililive_theme_common.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_menu.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/bililive/ui/views/menu/sceneitem_menu_cmd.h"

#include "ui/views/layout/grid_layout.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/widget/widget.h"

#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/public/secret/bililive_secret.h"

#include "base/strings/utf_string_conversions.h"
#include "base/strings/stringprintf.h"

#include "ui/base/resource/resource_bundle.h"
#include "grit/theme_resources.h"
#include "grit/generated_resources.h"

namespace
{
    class BottomBarFunctionButton : public BililiveImageButton
    {
    public:
        BottomBarFunctionButton(TabareaBottomFunction function, views::ButtonListener* listener)
            : BililiveImageButton(listener)
            , checked_(false)
        {
            ResourceBundle &rb = ResourceBundle::GetSharedInstance();
            set_id(function);
            switch (function)
            {
            case TabareaBottomFunction_Banned:
                SetTooltipText(GetLocalizedString(IDS_DANMUKU_BANNED_TITLE));
                SetImage(views::Button::STATE_NORMAL, GetImageSkiaNamed(IDR_LIVEHIME_V3_TABAREA_BANNED));
                SetImage(views::Button::STATE_HOVERED, GetImageSkiaNamed(IDR_LIVEHIME_V3_TABAREA_BANNED_HV));
                SetImage(views::Button::STATE_PRESSED, GetImageSkiaNamed(IDR_LIVEHIME_V3_TABAREA_BANNED_HV));
                break;
            case TabareaBottomFunction_NameList:
                SetTooltipText(GetLocalizedString(IDS_DANMUKU_NAMELIST_WINDOW_TITLE));
                SetImage(views::Button::STATE_NORMAL, GetImageSkiaNamed(IDR_LIVEHIME_V3_TABAREA_NAMELIST));
                SetImage(views::Button::STATE_HOVERED, GetImageSkiaNamed(IDR_LIVEHIME_V3_TABAREA_NAMELIST_HV));
                SetImage(views::Button::STATE_PRESSED, GetImageSkiaNamed(IDR_LIVEHIME_V3_TABAREA_NAMELIST_HV));
                break;
            case TabareaBottomFunction_Search:
                SetTooltipText(GetLocalizedString(IDS_DANMUKU_SEARCH_TITLE));
                SetImage(views::Button::STATE_NORMAL, GetImageSkiaNamed(IDR_LIVEHIME_V3_TABAREA_SEARCH));
                SetImage(views::Button::STATE_HOVERED, GetImageSkiaNamed(IDR_LIVEHIME_V3_TABAREA_SEARCH_HV));
                SetImage(views::Button::STATE_PRESSED, GetImageSkiaNamed(IDR_LIVEHIME_V3_TABAREA_SEARCH_HV));
            break;
            case TabareaBottomFunction_Refresh:
                SetTooltipText(GetLocalizedString(IDS_LOGIN_REFRESH));
                SetImage(views::Button::STATE_NORMAL, GetImageSkiaNamed(IDR_LIVEHIME_V3_TABAREA_REFRESH));
                SetImage(views::Button::STATE_HOVERED, GetImageSkiaNamed(IDR_LIVEHIME_V3_TABAREA_REFRESH_HV));
                SetImage(views::Button::STATE_PRESSED, GetImageSkiaNamed(IDR_LIVEHIME_V3_TABAREA_REFRESH_HV));
                break;
            default:
                break;
            }
        }

        void SetCheck(bool checked)
        {
            if (checked != checked_)
            {
                checked_ = checked;

                if (id() != TabareaBottomFunction_Refresh)
                {
                    ResourceBundle &rb = ResourceBundle::GetSharedInstance();
                    switch (id())
                    {
                    case TabareaBottomFunction_Banned:
                        SetImage(views::Button::STATE_NORMAL, rb.GetImageSkiaNamed(
                            checked_ ? IDR_LIVEHIME_V3_TABAREA_BANNED_HV : IDR_LIVEHIME_V3_TABAREA_BANNED));
                        break;
                    case TabareaBottomFunction_NameList:
                        SetImage(views::Button::STATE_NORMAL, rb.GetImageSkiaNamed(
                            checked_ ? IDR_LIVEHIME_V3_TABAREA_NAMELIST_HV : IDR_LIVEHIME_V3_TABAREA_NAMELIST));
                        break;
                    case TabareaBottomFunction_Search:
                        SetImage(views::Button::STATE_NORMAL, rb.GetImageSkiaNamed(
                            checked_ ? IDR_LIVEHIME_V3_TABAREA_SEARCH_HV : IDR_LIVEHIME_V3_TABAREA_SEARCH));
                        break;
                    default:
                        break;
                    }

                    if (!checked_)
                    {
                        base::NotificationService::current()->Notify(
                            kTabareaCloseTable.at(TabareaBottomFunction(id())),
                            base::NotificationService::AllSources(),
                            base::NotificationService::DummyDetails());
                    }
                }

                SchedulePaint();
            }
        };
        bool checked() const { return checked_; }

    private:
        bool checked_;
    };

    const std::map<TabareaBottomFunction, secret::LivehimeBehaviorEvent> kEventTable
    {
        { TabareaBottomFunction_Banned, secret::LivehimeBehaviorEvent::LivehimeBarrageBanned },
        { TabareaBottomFunction_NameList, secret::LivehimeBehaviorEvent::LivehimeBarrageNameList },
        { TabareaBottomFunction_Search, secret::LivehimeBehaviorEvent::LivehimeBarrageSearch },
        { TabareaBottomFunction_Refresh, secret::LivehimeBehaviorEvent::LivehimeBarrageFlush }
    };

    void EventTracking(secret::LivehimeBehaviorEvent event_id)
    {
        auto secret_core = GetBililiveProcess()->secret_core();
        secret_core->event_tracking_service()->ReportLivehimeBehaviorEvent(
            event_id, secret_core->account_info().mid(), std::string()).Call();
    }
}

BottomAreaView::BottomAreaView(BottomAreaDelegate *delegate)
    : delegate_(delegate)
{
    for (int i = 0; i < TabareaBottomFunction_COUNT; i++)
    {
        function_buttons_[i] = nullptr;
    }
}

void BottomAreaView::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            ResourceBundle &rb = ResourceBundle::GetSharedInstance();

            views::GridLayout *layout = new views::GridLayout(this);
            SetLayoutManager(layout);

            views::ColumnSet *column_set = layout->AddColumnSet(0);
            column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
            column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
            column_set->AddPaddingColumn(1.0f, 0);
            column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
            column_set->AddPaddingColumn(1.0f, 0);
            column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
            column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

            for (int i = 0; i < TabareaBottomFunction_COUNT; i++)
            {
                function_buttons_[i] = new BottomBarFunctionButton((TabareaBottomFunction)i, this);
            }

            layout->AddPaddingRow(1.0f, 0);
            layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForGroupCtrls);
            layout->AddView(function_buttons_[TabareaBottomFunction_NameList]);
            layout->AddView(function_buttons_[TabareaBottomFunction_Search]);
            layout->AddView(function_buttons_[TabareaBottomFunction_Refresh]);
            layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
            layout->AddPaddingRow(1.0f, 0);

            notifation_register_.Add(this, bililive::NOTIFICATION_LIVEHIME_TABAREA_BANNED_SHOW, base::NotificationService::AllSources());
            notifation_register_.Add(this, bililive::NOTIFICATION_LIVEHIME_TABAREA_BANNED_CLOSE, base::NotificationService::AllSources());
            notifation_register_.Add(this, bililive::NOTIFICATION_LIVEHIME_TABAREA_NAMELIST_SHOW, base::NotificationService::AllSources());
            notifation_register_.Add(this, bililive::NOTIFICATION_LIVEHIME_TABAREA_NAMELIST_CLOSE, base::NotificationService::AllSources());
            notifation_register_.Add(this, bililive::NOTIFICATION_LIVEHIME_TABAREA_SEARCH_SHOW, base::NotificationService::AllSources());
            notifation_register_.Add(this, bililive::NOTIFICATION_LIVEHIME_TABAREA_SEARCH_CLOSE, base::NotificationService::AllSources());
        }
        else
        {
            notifation_register_.RemoveAll();
        }
    }
}

void BottomAreaView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    if (sender->id() != TabareaBottomFunction_Refresh)
    {
        for (int i = 0; i < TabareaBottomFunction_Refresh; i++)
        {
            function_buttons_[i]->SetCheck(function_buttons_[i]->id() == sender->id());
        }
    }

    auto event_id = kEventTable.at((TabareaBottomFunction)(sender->id()));

    switch (sender->id())
    {
    case TabareaBottomFunction_NameList:
    case TabareaBottomFunction_Search:
        if (sender->tag() != 1)
        {
            ShowTabAreaBottomBarFunctionView((TabareaBottomFunction)sender->id());
            if (sender->id() != TabareaBottomFunction_NameList)
            {
                EventTracking(event_id);
            }
        }
        break;
    case TabareaBottomFunction_Refresh:
        if (delegate_)
        {
            delegate_->OnRefreshButtonPressed();
            EventTracking(event_id);
        }
        break;
    default:
        break;
    }
}

void BottomAreaView::Observe(int type, const base::NotificationSource& source, const base::NotificationDetails& details)
{
    if (0 != details.map_key())
    {
        return; // 只在乎由弹出框发出的通知
    }

    switch (type)
    {
    case bililive::NOTIFICATION_LIVEHIME_TABAREA_NAMELIST_SHOW:
        function_buttons_[TabareaBottomFunction_NameList]->SetCheck(true);
        function_buttons_[TabareaBottomFunction_NameList]->set_tag(1);
        break;
    case bililive::NOTIFICATION_LIVEHIME_TABAREA_NAMELIST_CLOSE:
        function_buttons_[TabareaBottomFunction_NameList]->SetCheck(false);
        function_buttons_[TabareaBottomFunction_NameList]->set_tag(0);
        break;
    case bililive::NOTIFICATION_LIVEHIME_TABAREA_SEARCH_SHOW:
        function_buttons_[TabareaBottomFunction_Search]->SetCheck(true);
        function_buttons_[TabareaBottomFunction_Search]->set_tag(1);
        break;
    case bililive::NOTIFICATION_LIVEHIME_TABAREA_SEARCH_CLOSE:
        function_buttons_[TabareaBottomFunction_Search]->SetCheck(false);
        function_buttons_[TabareaBottomFunction_Search]->set_tag(0);
        break;
    default:
        break;
    }
}

bool BottomAreaView::IsBottomAreaFunctionViewPopupNow() const
{
    for (int i = 0; i < TabareaBottomFunction_Refresh; i++)
    {
        if (function_buttons_[i]->checked())
        {
            return true;
        }
    }
    return false;
}