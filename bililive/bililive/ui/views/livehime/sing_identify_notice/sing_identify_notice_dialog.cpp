#include "bililive/bililive/ui/views/livehime/sing_identify_notice/sing_identify_notice_dialog.h"

#include "base/ext/callable_callback.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"

#include "bililive/bililive/bililive_process_impl.h"
#include "bililive/bililive/ui/views/controls/util/bililive_util_views.h"
#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/hotkey_notify/bililive_toast_notify_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_image_view.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/bililive/ui/views/controls/bililive_labelbutton.h"
#include "bililive/bililive/utils/convert_util.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/grid_layout.h"
#include "bililive/public/common/pref_names.h"
#include "base/prefs/pref_service.h"
#include "base/time/time.h"

namespace {
    SingIdentifyNoticeDialog* g_single_instance = nullptr;
    const gfx::Size kSingImageSize(GetLengthByDPIScale(222), GetLengthByDPIScale(126));
}

SingIdentifyNoticeDialog::SingIdentifyNoticeDialog()
    : BililiveWidgetDelegate(gfx::ImageSkia(), L"提示")
    , weakptr_factory_(this)
    , image_url_("")
{
}

SingIdentifyNoticeDialog::~SingIdentifyNoticeDialog()
{
    g_single_instance = nullptr;
}

void SingIdentifyNoticeDialog::ShowWindow(views::Widget * parent, const std::string& image_url)
{
    if (!g_single_instance){
        views::Widget *widget = new views::Widget();

        views::Widget::InitParams params;
        params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
        params.native_widget = new BililiveNativeWidgetWin(widget);
        params.parent = parent->GetNativeView();

        g_single_instance = new SingIdentifyNoticeDialog();
        g_single_instance->SetImageUrl(image_url);
        DoModalWidget(g_single_instance, widget, params);
    }
    else{
        g_single_instance->GetWidget()->Activate();
    }
}

void SingIdentifyNoticeDialog::Close()
{
    if (g_single_instance){
        g_single_instance->GetWidget()->Close();
    }
}

void SingIdentifyNoticeDialog::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails & details)
{
    if (details.child == this){
        if (details.is_add){
            InitViews();
        }
    }
}

void SingIdentifyNoticeDialog::InitViews()
{
    auto& rb = ResourceBundle::GetSharedInstance();
    views::GridLayout* layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    views::ColumnSet* column_set = layout->AddColumnSet(0);
    column_set->AddPaddingColumn(0, GetLengthByDPIScale(110));
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(1.0, GetLengthByDPIScale(110));

    image_view_ = new LivehimeImageView(this);
    image_view_->SetPreferredSize(kSingImageSize);
    if (image_url_.empty()) {
        image_view_->SetImagePlaceholder(*GetImageSkiaNamed(IDR_LIVEHIME_V4_LIVE_POPUP_SING_IDENTIFY));
    }
    else {
        image_view_->SetImageUrl(image_url_);
    }
    image_view_->SetEnabled(false);
    layout->StartRowWithPadding(0, 0, 0, GetLengthByDPIScale(16));
    layout->AddView(image_view_);

    notice_title_lab = new LivehimeTitleLabel(L"唱歌检测");
    notice_title_lab->SetFont(ftFourteenBold);
    layout->StartRowWithPadding(0, 0, 0, GetLengthByDPIScale(11));
    layout->AddView(notice_title_lab);

    notice_lab_ = new LivehimeTitleLabel(L"唱歌检测已开启,唱歌会增加直播间曝光唷`");
    notice_lab_->SetFont(ftTwelve);
    layout->StartRowWithPadding(0, 0, 0, GetLengthByDPIScale(10));
    layout->AddView(notice_lab_);

    column_set = layout->AddColumnSet(1);
    column_set->AddPaddingColumn(0, GetLengthByDPIScale(109));
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, GetLengthByDPIScale(109));

    know_button_ = new LivehimeActionLabelButton(this, L"我知道了", true);
    layout->StartRowWithPadding(0, 1, 0, GetLengthByDPIScale(25));
    layout->AddView(know_button_);

    column_set = layout->AddColumnSet(2);
    column_set->AddPaddingColumn(0, GetLengthByDPIScale(190));
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, GetLengthByDPIScale(170));

    not_notice_checkbox_ = new LivehimeCheckbox(L"不在提示");
    not_notice_checkbox_->set_listener(this);
    layout->StartRowWithPadding(0, 2, 0, GetLengthByDPIScale(10));
    layout->AddView(not_notice_checkbox_);

    livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::SingPanelShow, "");
}

gfx::Size SingIdentifyNoticeDialog::GetPreferredSize()
{
    gfx::Size size = __super::GetPreferredSize();
    size.SetSize(GetLengthByDPIScale(440), GetLengthByDPIScale(308));
    return size;
}

void SingIdentifyNoticeDialog::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    if (sender != nullptr) {
        if (know_button_ && sender == know_button_) {
            livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::SingPanelConfirmClick, "");
            SetResultCode(IDOK);
            GetWidget()->Close();
        }
        else if (not_notice_checkbox_&& sender == not_notice_checkbox_) {
            PrefService* pref = GetBililiveProcess()->global_profile()->GetPrefs();
            if (pref) {
                if (not_notice_checkbox_->checked()) {  // 选择不再提示
                    pref->SetBoolean(prefs::kSingIdentifyNoticeShow, false);
                    livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::SingPanelNoMoreClick, "");
                }
                else {
                    pref->SetBoolean(prefs::kSingIdentifyNoticeShow, true);
                }
            }
        }
    }
}