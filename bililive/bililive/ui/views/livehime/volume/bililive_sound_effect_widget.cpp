#include "bililive/bililive/ui/views/livehime/volume/bililive_sound_effect_widget.h"

#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/volume/bililive_sound_effect_view.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/grid_layout.h"

#include "grit/generated_resources.h"


// static
BililiveSoundEffectWidget* BililiveSoundEffectWidget::instance_ = nullptr;

void BililiveSoundEffectWidget::ShowForm(views::Widget *parent)
{
    if (!instance_) {
        views::Widget *widget_ = new views::Widget();
        views::Widget::InitParams params;
        params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
        params.native_widget = new BililiveNativeWidgetWin(widget_);
        if (parent) {
            params.parent = parent->GetNativeWindow();
            parent->Activate();
        }

        instance_ = new BililiveSoundEffectWidget();
        DoModalWidget(instance_, widget_, params);
    }
    else {
        if (instance_->GetWidget()) {
            instance_->GetWidget()->Activate();
        }
    }
}

// BililiveSoundEffectWidget
BililiveSoundEffectWidget::BililiveSoundEffectWidget()
    : BililiveWidgetDelegate(gfx::ImageSkia(),
    ResourceBundle::GetSharedInstance().GetLocalizedString(IDS_VOLUME_SOUND_EFFECT_CAPTION)),
    sound_effect_view_(nullptr),
    ok_button_(nullptr),
    cancel_button_(nullptr)
{
}

BililiveSoundEffectWidget::~BililiveSoundEffectWidget()
{
    instance_ = nullptr;
}

void BililiveSoundEffectWidget::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails &details)
{
    if (details.is_add && details.child == this)
    {
        InitViews();
    }
}

void BililiveSoundEffectWidget::InitViews()
{
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();

    views::GridLayout* layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    views::ColumnSet* column_set = layout->AddColumnSet(0);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);

    sound_effect_view_ = new BililiveSoundEffectView();
    sound_effect_view_->SetInsets({ 0, GetLengthByDPIScale(30), 0, GetLengthByDPIScale(30) });

    layout->AddPaddingRow(0, kPaddingRowHeightEnds);
    layout->StartRow(1.0f, 0);
    layout->AddView(sound_effect_view_->Container());

    ok_button_ = new LivehimeActionLabelButton(this, rb.GetLocalizedString(IDS_SRCPROP_COMMON_OK), true);
    ok_button_->SetIsDefault(true);
    cancel_button_ = new LivehimeActionLabelButton(this, rb.GetLocalizedString(IDS_SRCPROP_COMMON_CANCEL), false);

    column_set = layout->AddColumnSet(1);
    column_set->AddPaddingColumn(1.0f, 0);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForActionButton);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(1.0f, 0);

    layout->AddPaddingRow(0, kPaddingRowHeightForDiffGroups);
    layout->StartRow(0, 1);
    layout->AddView(ok_button_);
    layout->AddView(cancel_button_);

    layout->AddPaddingRow(0, kPaddingRowHeightEnds);
}

gfx::Size BililiveSoundEffectWidget::GetPreferredSize()
{
    return gfx::Size(GetLengthByDPIScale(765), GetLengthByDPIScale(500));
}

void BililiveSoundEffectWidget::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    if (ok_button_ == sender)
    {
        BililiveSoundEffectView::ChangeType result{};
        sound_effect_view_->SaveOrCheckStreamingSettingsChange(false, result);

        SetResultCode(IDOK);
    }
    else
    {
        SetResultCode(IDCANCEL);
    }
    GetWidget()->Close();
}

void BililiveSoundEffectWidget::WindowClosing()
{
    if (GetResultCode() != IDOK)
    {
        sound_effect_view_->OnCancel();
    }
}