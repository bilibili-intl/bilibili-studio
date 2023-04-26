#include "bililive_frame_titlebar_view.h"

#include "ui/native_theme/native_theme.h"
#include "ui/views/controls/button/image_button.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/widget/widget.h"
#include "ui/base/resource/resource_bundle.h"

#include "grit/theme_resources.h"
#include "grit/generated_resources.h"

#include "bililive/app/bililive_dll_resource.h"
#include "bililive/bililive/ui/views/controls/bililive_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/utils/bililive_image_util.h"
#include "bililive/bililive/utils/image_util.h"



BililiveFrameTitleBarView::BililiveFrameTitleBarView(gfx::ImageSkia *icon, base::string16 caption,
    int titlebar_button_combination, bool title_center)
    :title_center_(title_center)
{
    set_focus_border(nullptr);
    set_focusable(true);

    icon_image_view_ = new views::ImageView();
    icon_image_view_->SetImage(icon);
    icon_image_view_->set_interactive(false);

    caption_label_ = new BililiveLabel(caption);
    // caption_label_->SetTextColor(GetColor(WindowTitleText));
    // 这里统一字体颜色
    caption_label_->SetTextColor(SkColorSetRGB(159, 161, 178));
    caption_label_->SetFont(ftPrimary);

    InitViews(titlebar_button_combination);
}

BililiveFrameTitleBarView::~BililiveFrameTitleBarView()
{
}

void BililiveFrameTitleBarView::SetTitle(const base::string16& title)
{
    caption_label_->SetText(title);
    InvalidateLayout();
    Layout();
}

void BililiveFrameTitleBarView::InitViews(int button_combination)
{
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();

    // 这个的标题需要统一颜色
    set_background(views::Background::CreateSolidBackground(SkColorSetRGB(55, 56, 75)));

    views::GridLayout *layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    views::ColumnSet *column_set = layout->AddColumnSet(0);
    column_set->AddPaddingColumn(0, 5);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
    if (title_center_) {
        column_set->AddPaddingColumn(1.0f, 0);
    }
    else {
        column_set->AddPaddingColumn(0, 5);
    }
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(1.0f, 0);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);

    views::View *btn_view = new views::View();
    views::GridLayout *btns_layout = new views::GridLayout(btn_view);
    btn_view->SetLayoutManager(btns_layout);
    {
        views::ColumnSet *column_set = btns_layout->AddColumnSet(0);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
        if(button_combination >= TBB_CLOSE)
        {
            btns_layout->StartRow(1.0f, 0);
        }
    }
    if (button_combination & TBB_HELP)
    {
        views::ImageButton *button = new views::ImageButton(this);
        button->SetImage(views::Button::STATE_NORMAL, rb.GetImageSkiaNamed(IDR_LIVEMAIN_MSGBOX_TITLEBAR_HELP));
        button->SetImage(views::Button::STATE_HOVERED, rb.GetImageSkiaNamed(IDR_LIVEMAIN_MSGBOX_TITLEBAR_HELP_HV));
        button->SetImage(views::Button::STATE_PRESSED, rb.GetImageSkiaNamed(IDR_LIVEMAIN_MSGBOX_TITLEBAR_HELP_HV));
        button->SetTooltipText(GetLocalizedString(IDS_HELP_VIEW_HELP));

        button->set_id(TBB_HELP);
        btns_layout->AddView(button);
        button_map_[TBB_HELP] = button;
    }
    if (button_combination & TBB_MIN)
    {
        views::ImageButton *button = new views::ImageButton(this);
        button->SetImage(views::Button::STATE_NORMAL, rb.GetImageSkiaNamed(IDR_LIVEMAIN_MSGBOX_TOOLBAR_MIN));
        button->SetImage(views::Button::STATE_HOVERED, rb.GetImageSkiaNamed(IDR_LIVEMAIN_MSGBOX_TOOLBAR_MIN_HV));
        button->SetImage(views::Button::STATE_PRESSED, rb.GetImageSkiaNamed(IDR_LIVEMAIN_MSGBOX_TOOLBAR_MIN_HV));
        button->SetTooltipText(rb.GetLocalizedString(IDS_UGC_MINIMISE));
        button->set_id(TBB_MIN);
        btns_layout->AddView(button);
        button_map_[TBB_MIN] = button;
    }
    if (button_combination & TBB_MAX)
    {
        views::ImageButton *button = new views::ImageButton(this);
        button->SetImage(views::Button::STATE_NORMAL, rb.GetImageSkiaNamed(IDR_LIVEHIME_TITLEBAR_MAXIMIZE));
        button->SetImage(views::Button::STATE_HOVERED, rb.GetImageSkiaNamed(IDR_LIVEHIME_TITLEBAR_MAXIMIZE_HV));
        button->SetImage(views::Button::STATE_PRESSED, rb.GetImageSkiaNamed(IDR_LIVEHIME_TITLEBAR_MAXIMIZE_HV));
        button->SetTooltipText(rb.GetLocalizedString(IDS_UGC_MAXIMIN));
        button->set_id(TBB_MAX);
        btns_layout->AddView(button);
        button_map_[TBB_MAX] = button;
    }
    if (button_combination & TBB_CLOSE)
    {
        views::ImageButton *button = new views::ImageButton(this);
        button->SetImage(views::Button::STATE_NORMAL, rb.GetImageSkiaNamed(IDR_LIVEMAIN_MSGBOX_TOOLBAR_CLOSE));
        button->SetImage(views::Button::STATE_HOVERED, rb.GetImageSkiaNamed(IDR_LIVEMAIN_MSGBOX_TOOLBAR_CLOSE_HV));
        button->SetImage(views::Button::STATE_PRESSED, rb.GetImageSkiaNamed(IDR_LIVEMAIN_MSGBOX_TOOLBAR_CLOSE_HV));
        button->SetTooltipText(rb.GetLocalizedString(IDS_UGC_CLOSE));
        button->set_id(TBB_CLOSE);
        btns_layout->AddView(button);
        button_map_[TBB_CLOSE] = button;
    }

    layout->StartRow(1.0f, 0);
    layout->AddView(icon_image_view_);
    layout->AddView(caption_label_);
    layout->AddView(btn_view);
}

gfx::Size BililiveFrameTitleBarView::GetPreferredSize()
{
    gfx::Size size = __super::GetPreferredSize();
    size.set_height(std::max(kMsgboxTitleBarHeight, size.height()));
    return size;
}

void BililiveFrameTitleBarView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    TitleBarButton btn_id = (TitleBarButton)sender->id();
    if (delegate() && !delegate()->OnTitleBarButtonPressed(btn_id))
    {
        return;
    }

    switch (btn_id)
    {
    case TBB_CLOSE:
            GetWidget()->Close();
        break;
    case TBB_MIN:
            GetWidget()->Minimize();
        break;
    case TBB_MAX:
        if (GetWidget()->IsMaximized())
        {
            GetWidget()->Restore();
        }
        else
        {
            GetWidget()->Maximize();
        }
        break;
    case TBB_HELP:
        NOTREACHED() << "help not impl!";
        break;
    default:
        break;
    }
}

void BililiveFrameTitleBarView::OnActiveChanged(bool prev_state)
{
    caption_label_->SetTextColor(active() ? GetColor(WindowTitleText) :
                                 ui::NativeTheme::instance()->GetSystemColor(ui::NativeTheme::kColorId_LabelDisabledColor));
}

void BililiveFrameTitleBarView::OnBoundsChanged(const gfx::Rect& previous_bounds)
{
    ChangeMaxButtonStateImage();
}

void BililiveFrameTitleBarView::ChangeMaxButtonStateImage()
{
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();
    views::ImageButton *button = button_map_[TBB_MAX];
    if (button)
    {
        if (GetWidget()->IsMaximized())
        {
            button->SetImage(views::Button::STATE_NORMAL, rb.GetImageSkiaNamed(IDR_LIVEHIME_TITLEBAR_RESTORE));
            button->SetImage(views::Button::STATE_HOVERED, rb.GetImageSkiaNamed(IDR_LIVEHIME_TITLEBAR_RESTORE_HV));
            button->SetImage(views::Button::STATE_PRESSED, rb.GetImageSkiaNamed(IDR_LIVEHIME_TITLEBAR_RESTORE_HV));
            button->SetTooltipText(rb.GetLocalizedString(IDS_UGC_RESTORE));
        }
        else
        {
            button->SetImage(views::Button::STATE_NORMAL, rb.GetImageSkiaNamed(IDR_LIVEHIME_TITLEBAR_MAXIMIZE));
            button->SetImage(views::Button::STATE_HOVERED, rb.GetImageSkiaNamed(IDR_LIVEHIME_TITLEBAR_MAXIMIZE_HV));
            button->SetImage(views::Button::STATE_PRESSED, rb.GetImageSkiaNamed(IDR_LIVEHIME_TITLEBAR_MAXIMIZE_HV));
            button->SetTooltipText(rb.GetLocalizedString(IDS_UGC_MAXIMIN));
        }
    }
}

bool BililiveFrameTitleBarView::OnMousePressed(const ui::MouseEvent& event)
{
    RequestFocus();
    return false;
}



GeneralMsgBoxTitleBarView::GeneralMsgBoxTitleBarView(base::string16 caption)
{
    set_focus_border(nullptr);
    set_focusable(true);
    caption_label_ = new BililiveLabel(caption);
    caption_label_->SetTextColor(GetColor(LabelTitle));
    caption_label_->SetFont(ftSixteenBold);

    InitViews();
}

GeneralMsgBoxTitleBarView::~GeneralMsgBoxTitleBarView()
{
}

void GeneralMsgBoxTitleBarView::SetTitle(const base::string16& title)
{
    caption_label_->SetText(title);
    InvalidateLayout();
    Layout();
}

void GeneralMsgBoxTitleBarView::SetCloseBtnVisible(bool flag)
{
    button_->SetVisible(flag);
}

gfx::Size GeneralMsgBoxTitleBarView::GetPreferredSize()
{
    gfx::Size size = __super::GetPreferredSize();
    size.set_height(std::max(kMsgboxTitleBarHeight, size.height()));
    return size;
}

bool GeneralMsgBoxTitleBarView::OnMousePressed(const ui::MouseEvent& event)
{
    RequestFocus();
    return false;
}

void GeneralMsgBoxTitleBarView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    TitleBarButton btn_id = (TitleBarButton)sender->id();
    if (delegate() && !delegate()->OnTitleBarButtonPressed(btn_id))
    {
        return;
    }

    switch (btn_id)
    {
    case TBB_CLOSE:
        GetWidget()->Close();
        break;
    default:
        break;
    }
}

void GeneralMsgBoxTitleBarView::OnActiveChanged(bool prev_state)
{
    caption_label_->SetTextColor(active() ? GetColor(WindowTitleText) :
        ui::NativeTheme::instance()->GetSystemColor(ui::NativeTheme::kColorId_LabelDisabledColor));
}

void GeneralMsgBoxTitleBarView::InitViews()
{
    ResourceBundle& rb = ResourceBundle::GetSharedInstance();

    views::GridLayout* layout = new views::GridLayout(this);
    SetLayoutManager(layout);
    views::ColumnSet* column_set = layout->AddColumnSet(0);
    column_set->AddPaddingColumn(0, kGeneralMsgBoxTitleBarPadding);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(1.0f, 0);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kGeneralMsgBoxTitleBarPadding);

    views::View* btn_view = new views::View(); {
        views::GridLayout* btns_layout = new views::GridLayout(btn_view);
        btn_view->SetLayoutManager(btns_layout);
        views::ColumnSet* column_set = btns_layout->AddColumnSet(0);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
        btns_layout->StartRow(0, 0);
        button_ = new views::ImageButton(this);
        button_->SetImage(views::Button::STATE_NORMAL, rb.GetImageSkiaNamed(IDR_LIVEMAIN_MSGBOX_GEN_TOOLBAR_CLOSE));
        button_->SetImage(views::Button::STATE_HOVERED, rb.GetImageSkiaNamed(IDR_LIVEMAIN_MSGBOX_GEN_TOOLBAR_CLOSE_HV));
        button_->SetImage(views::Button::STATE_PRESSED, rb.GetImageSkiaNamed(IDR_LIVEMAIN_MSGBOX_GEN_TOOLBAR_CLOSE_HV));
        button_->set_id(TBB_CLOSE);
        btns_layout->AddView(button_);
    }
    layout->AddPaddingRow(0, kGeneralMsgBoxTitleBarPadding);
    layout->StartRow(0, 0);
    layout->AddView(caption_label_);
    layout->AddView(btn_view);
    layout->AddPaddingRow(0, kGeneralMsgBoxTitleBarPadding);
}
