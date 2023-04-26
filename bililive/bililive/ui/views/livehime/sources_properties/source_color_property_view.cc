#include "bililive/bililive/ui/views/livehime/sources_properties/source_color_property_view.h"

#include "base/strings/stringprintf.h"
#include "base/strings/string_number_conversions.h"
#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/livehime/sources_properties/source_color_property_presenter_impl.h"
#include "bililive/bililive/ui/dialog/shell_dialogs.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/bililive/utils/bililive_filetype_exts.h"

#include "ui/views/layout/grid_layout.h"
#include "ui/base/resource/resource_bundle.h"

#include "grit/generated_resources.h"
#include "grit/theme_resources.h"

namespace
{
    using namespace bililive;

    enum ColorPropDetailColumnSet
    {
        NAMECOLUMNSET = 0,
        COLORCOLUMNSET,
    };

    int StrToIntWithDefault(const std::wstring& str,int default_val)
    {
        int val = 0;
        if (base::StringToInt(str, &val))
        {
            return val;
        }
        else
        {
            return default_val;
        }
    }
}   // namespace

namespace livehime
{

    BasePropertyView* CreateColorSourcePropertyDetailView(obs_proxy::SceneItem* scene_item)
    {
        return new ColorPropDetailView(scene_item);
    }

}   // namespace livehime

ColorPropDetailView::ColorPropDetailView(obs_proxy::SceneItem* scene_item)
    : data_loaded_(false)
    , name_edit_(nullptr)
    , color_button_(nullptr)
    , width_edit_(nullptr)
    , height_edit_(nullptr)
    , transparent_slider_(nullptr)
    , transparent_value_label_(nullptr)
    , presenter_(std::make_unique<SourceColorPropertyPresenterImpl>(scene_item))
{
}

ColorPropDetailView::~ColorPropDetailView()
{

}

void ColorPropDetailView::InitView()
{
    auto& res = ui::ResourceBundle::GetSharedInstance();
    views::GridLayout* layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    views::ColumnSet* column_set = layout->AddColumnSet(ColorPropDetailColumnSet::NAMECOLUMNSET);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::FIXED, MaxLabelWidth(), 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

    column_set = layout->AddColumnSet(ColorPropDetailColumnSet::COLORCOLUMNSET);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::FIXED, MaxLabelWidth(), 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);


    BililiveLabel* label = new LivehimeTitleLabel(res.GetLocalizedString(IDS_SRCPROP_COMMON_SOURCENAME));
    label->SetHorizontalAlignment(gfx::ALIGN_RIGHT);
    name_edit_ = new LivehimeNativeEditView();
    layout->AddPaddingRow(0, kPaddingRowHeightEnds);
    layout->StartRow(0, ColorPropDetailColumnSet::NAMECOLUMNSET);
    layout->AddView(label);
    layout->AddView(name_edit_);

    label = new LivehimeTitleLabel(res.GetLocalizedString(IDS_SRCPROP_COLOR_COLOR));
    label->SetHorizontalAlignment(gfx::ALIGN_RIGHT);
    color_button_ = new LivehimeFunctionLabelButton(this, L"");
    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, ColorPropDetailColumnSet::COLORCOLUMNSET);
    layout->AddView(label);
    layout->AddView(color_button_);

    label = new LivehimeTitleLabel(res.GetLocalizedString(IDS_SRCPROP_COLOR_WIDTH));
    label->SetHorizontalAlignment(gfx::ALIGN_RIGHT);
    width_edit_ = new LivehimeNativeEditView();
    width_edit_->SetNumberOnly(true);
    width_edit_->SetController(this);
    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, ColorPropDetailColumnSet::COLORCOLUMNSET);
    layout->AddView(label);
    layout->AddView(width_edit_);

    label = new LivehimeTitleLabel(res.GetLocalizedString(IDS_SRCPROP_COLOR_HEIGHT));
    label->SetHorizontalAlignment(gfx::ALIGN_RIGHT);
    height_edit_ = new LivehimeNativeEditView();
    height_edit_->SetNumberOnly(true);
    height_edit_->SetController(this);
    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, ColorPropDetailColumnSet::COLORCOLUMNSET);
    layout->AddView(label);
    layout->AddView(height_edit_);

    label = new LivehimeTitleLabel(res.GetLocalizedString(IDS_SRCPROP_COLOR_TRANSPARENT));
    label->SetHorizontalAlignment(gfx::ALIGN_RIGHT);
    views::View* view = new views::View();
    {
        views::GridLayout* grid_layout = new views::GridLayout(view);
        view->SetLayoutManager(grid_layout);

        views::ColumnSet* column_set = grid_layout->AddColumnSet(0);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::FIXED,
            LivehimeContentLabel::GetFont().GetStringWidth(L"100"), 0);

        transparent_slider_ = new LivehimeSlider(this, views::Slider::HORIZONTAL);
        transparent_value_label_ = new LivehimeContentLabel(L"0");

        grid_layout->StartRow(0, ColorPropDetailColumnSet::NAMECOLUMNSET);
        grid_layout->AddView(transparent_slider_);
        grid_layout->AddView(transparent_value_label_);
    }
    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, ColorPropDetailColumnSet::NAMECOLUMNSET);
    layout->AddView(label);
    layout->AddView(view);   
}

void ColorPropDetailView::InitData()
{
    name_edit_->SetText(presenter_->GetName());
    color_button_->SetFixedBkColor(presenter_->GetColor());
    width_edit_->SetText(std::to_wstring(presenter_->GetWidth()));
    height_edit_->SetText(std::to_wstring(presenter_->GetHeight()));
    transparent_slider_->SetValue(presenter_->GetTransparent());
    presenter_->Snapshot();
    data_loaded_ = true;
}

bool ColorPropDetailView::CheckSetupValid()
{
    return true;
}

bool ColorPropDetailView::SaveSetupChange()
{
    if (!data_loaded_)
    {
        return false;
    }

    presenter_->SetColor(color_button_->GetNormalBkColor());
    presenter_->SetWidth(StrToIntWithDefault(width_edit_->GetText(), presenter_->GetWidth()));
    presenter_->SetHeight(StrToIntWithDefault(height_edit_->GetText(), presenter_->GetHeight()));
    presenter_->SetTransparent(transparent_slider_->value());
    presenter_->Update();

    if (presenter_->SetName(name_edit_->GetText()) == false)
    {
        livehime::ShowMessageBox(livehime::UniversalMsgboxType::CannotRenameSceneItem);
        return false;
    }

    return true;
}

bool ColorPropDetailView::Cancel()
{
    presenter_->Restore();

    return true;
}

gfx::ImageSkia* ColorPropDetailView::GetSkiaIcon()
{
    return ui::ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEHIME_SOURCE_PROPERTY_PICTURE_TITLE);
}

std::wstring ColorPropDetailView::GetCaption()
{
    auto& res = ui::ResourceBundle::GetSharedInstance();
    return res.GetLocalizedString(IDS_SRCPROP_COLOR_CAPTION);
}

void ColorPropDetailView::SliderValueChanged(views::Slider* sender, float value,
    float old_value, views::SliderChangeReason reason)
{
    if (sender == transparent_slider_)
    {
        transparent_value_label_->SetText(base::StringPrintf(L"%d", MapFloatToInt(value)));
        EffectiveImmediately(COLOR_TRANSPARENT);
    }
}

void ColorPropDetailView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    EffectiveControl effective_control = COLOR_INVALID;
    if (sender == color_button_)
    {
        SkColor color = color_button_->GetNormalBkColor();
        SelectColorDialog dlg;
        dlg.SetCurrentColor(color);
        if (dlg.DoModel(sender->GetWidget()) == SelectColorDialog::RESULT_OK)
        {
            color_button_->SetFixedBkColor(dlg.GetCurrentColor());
        }
        effective_control = COLOR_COLOR;
    }
    EffectiveImmediately(effective_control);
}

void ColorPropDetailView::EffectiveImmediately(EffectiveControl effective_control)
{
    switch (effective_control)
    {
    case COLOR_COLOR:
        presenter_->SetColor(color_button_->GetNormalBkColor());
        break;
    case COLOR_WIDTH:
        presenter_->SetWidth(std::stoi(width_edit_->GetText()));
        break;
    case COLOR_HEIGHT:
        presenter_->SetHeight(std::stoi(height_edit_->GetText()));
        break;
    case COLOR_TRANSPARENT:
        presenter_->SetTransparent(transparent_slider_->value());
        break;
    default:
        return;
    }

    presenter_->Update();
}

void ColorPropDetailView::ContentsChanged(BilibiliNativeEditView* sender, const string16& new_contents)
{
    if (!new_contents.empty() && (sender == width_edit_ || sender == height_edit_))
    {
        if (std::stoi(new_contents) > 1000)
        {
            sender->SetText(L"1000");
        }
        if (new_contents.length() > 1 && new_contents.at(0) == '0')
        {
            int val = std::stoi(new_contents);
            auto val_str = std::to_wstring(val);
            sender->SetText(val_str);
        }
        EffectiveControl effective_control;
        if (sender == width_edit_)
        {
            effective_control = COLOR_WIDTH;
        }
        else
        {
            effective_control = COLOR_HEIGHT;
        }
        EffectiveImmediately(effective_control);
    }
}
