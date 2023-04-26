#include "bililive/bililive/livehime/help_center/help_center.h"

#include <map>
#include <shellapi.h>
#include <string>

#include "base/strings/stringprintf.h"

#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/grid_layout.h"

#include "bililive/bililive/utils/net_util.h"
#include "bililive/bililive/ui/views/controls/bililive_bubble.h"
#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_bubble.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_hover_tip_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/theme/livehime_theme_constants.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/secret/public/account_info.h"
#include "bililive/secret/public/event_tracking_service.h"
#include "bililive/bililive/utils/fast_forward_url_convert.h"

namespace
{
    const int kAfterTimespanMs = 500;
    const int kFadeDurationMs = 500;

    const std::map<livehime::HelpType, std::pair<int, std::string>> kHelpCenterAnchorIdMap{
        { livehime::HelpType::Home,  { -1, "home" } },
        { livehime::HelpType::D3DCompiler, {1, "D3DCompiler"} },

        { livehime::HelpType::SourceMonitor, {5, "SourceMonitor"} },
        { livehime::HelpType::SourceGame, {16, "SourceGame"} },
        { livehime::HelpType::SourceGameLOL, {7, "SourceGameLOL"} },
        { livehime::HelpType::SourceWindow, {10, "SourceWindow"} },
        { livehime::HelpType::SourceCamera, {19, "SourceCamera"} },
        { livehime::HelpType::SourceProjection, {12, "SourceProjection"} },
        { livehime::HelpType::SourceMedia, {27, "SourceMedia"} },
        { livehime::HelpType::SourceMediaCaptureOption, {33, "SourceMediaCaptureOption"} },

        { livehime::HelpType::ThirdPartyStreaming, {37, "tps"} },
        { livehime::HelpType::OptimizeForPerformance, {31, "OptimizeForPerformance"} },
        { livehime::HelpType::ColivePkVerTooLow, {24, "ColivePkVerTooLow"} },

        { livehime::HelpType::VoiceMicVSysI, {9, "VoiceMicVSysI"} },
        { livehime::HelpType::VoiceMicI, {23, "VoiceMicI"} },

        { livehime::HelpType::HardwareEncoderError, {22, "HardwareEncoderError"} },
        { livehime::HelpType::SoftwareEncoderError, {32, "SoftwareEncoderError"} },

        { livehime::HelpType::SoundEffectFreeze, {34, "SoundEffectFreeze"} },

        { livehime::HelpType::VtuberFigma, {38, "VtuberFigma"} },
        { livehime::HelpType::VtuberFace, {39, "VtuberFace"} },
    };

    BililiveBubbleView* g_single_bubble_instance = nullptr;

    class HelpBubbleContentView :
        public views::View,
        views::ButtonListener
    {
    public:
        HelpBubbleContentView(livehime::HelpType type)
            : type_(type)
        {
            set_notify_enter_exit_on_child(true);

            SetLayoutManager(new views::FillLayout());
            auto button = new LivehimeLinkButton(this, GetLocalizedString(IDS_HELP_VIEW_HELP));
            button->SetAllStateTextColor(clrLinkButtonPressed);
            AddChildView(button);
        }

        ~HelpBubbleContentView()
        {
            if (g_single_bubble_instance && g_single_bubble_instance->content_view() == this)
            {
                g_single_bubble_instance = nullptr;
            }
        }

    protected:
        // ButtonListener
        void ButtonPressed(views::Button* sender, const ui::Event& event) override
        {
            livehime::ShowHelp(type_);
        }

        // View
        void OnMouseEntered(const ui::MouseEvent& event) override
        {
            // 进来就重置bubble的渐隐
            DCHECK(g_single_bubble_instance);
            if (g_single_bubble_instance && g_single_bubble_instance->content_view() == this)
            {
                g_single_bubble_instance->ResetFade();
            }
            else
            {
                NOTREACHED();
            }
        }

        void OnMouseExited(const ui::MouseEvent& event) override
        {
            DCHECK(g_single_bubble_instance);
            if (g_single_bubble_instance && g_single_bubble_instance->content_view() == this)
            {
                g_single_bubble_instance->StartFade(false, kAfterTimespanMs, kFadeDurationMs);
            }
            else
            {
                NOTREACHED();
            }
        }

    private:
        livehime::HelpType type_;
    };

    void ReportHelpEvent(livehime::HelpType type)
    {
        if (kHelpCenterAnchorIdMap.find(type) != kHelpCenterAnchorIdMap.end())
        {
            auto secret_core = GetBililiveProcess()->secret_core();
            secret_core->event_tracking_service()->ReportLivehimeBehaviorEvent(
                secret::LivehimeBehaviorEvent::LivehimeMoreHelp,
                secret_core->account_info().mid(),
                "type:" + kHelpCenterAnchorIdMap.at(type).second
            ).Call();
        }
    }

}

namespace livehime
{
    bool ShowHelp(HelpType type)
    {
        return true;
    }

    bool ShowHelp(HelpType type, views::View* anchor_view, bool show)
    {
        return true;
    }

    // HelpSupport
    void HelpSupport::EnableShowHelp(bool enable, HelpType help_type)
    {
        enable_help_ = enable;
        help_type_ = help_type;
    }

    bool HelpSupport::ShowHelp(views::View* anchor_view /*= nullptr*/, bool show /*= true*/)
    {
        if (!enable_help())
        {
            return false;
        }

        bool ret = false;
        if (anchor_view)
        {
            ret = livehime::ShowHelp(help_type_, anchor_view, show);
        }
        else
        {
            ret = livehime::ShowHelp(help_type_);
        }
        return ret;
    }


    // HelpSupportView
    HelpSupportView::HelpSupportView(views::View* view, HelpType type,
        int h_padding/* = kPaddingColWidthForCtrlTips*/)
        : help_type_(type)
    {
        views::GridLayout* layout = new views::GridLayout(this);
        SetLayoutManager(layout);

        views::ColumnSet* column_set = layout->AddColumnSet(0);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 1.0f, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddPaddingColumn(0, h_padding);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);

        gfx::ImageSkia *skia = GetImageSkiaNamed(IDR_LIVEHIME_TIP);
        gfx::ImageSkia *skia_hv = GetImageSkiaNamed(IDR_LIVEHIME_TIP_HV);
        BililiveImageButton* help_button = new BililiveImageButton(this);
        help_button->SetImage(views::Button::STATE_NORMAL, skia);
        help_button->SetImage(views::Button::STATE_HOVERED, skia_hv);
        help_button->SetImage(views::Button::STATE_PRESSED, skia_hv);
        help_button->SetTooltipText(GetLocalizedString(IDS_HELP_VIEW_HELP));

        layout->StartRow(0, 0);
        layout->AddView(view);
        
    }

    void HelpSupportView::ButtonPressed(views::Button* sender, const ui::Event& event)
    {
        if (help_type_ != HelpType::None)
        {
            ShowHelp(help_type_);
        }
    }

}