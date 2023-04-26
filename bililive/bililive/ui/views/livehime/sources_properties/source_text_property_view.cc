#include "bililive/bililive/ui/views/livehime/sources_properties/source_text_property_view.h"

#include "base/ext/callable_callback.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"

#include "bilibase/scope_guard.h"

#include "bililive/bililive/bililive_process_impl.h"
#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/livehime/sources_properties/source_text_property_presenter_impl.h"
#include "bililive/bililive/ui/dialog/shell_dialogs.h"
#include "bililive/bililive/ui/views/hotkey_notify/bililive_hotkey_notify_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/bililive/utils/bililive_filetype_exts.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/grid_layout.h"

#include "grit/generated_resources.h"
#include "grit/theme_resources.h"

namespace
{

using namespace bililive;

enum TextPropDetailColumnSet
{
    NAMECOLUMNSET = 0,
    TEXTAREACOLUMNSET,
    FILECOLUMNSET
};
const int kScrollSpeedMin = 0;
const int kScrollSpeedMax = 300;

const int kOutlineSizeMin = 1;
const int kOutlineSizeMax = 20;

}   // namespace

namespace livehime
{

BasePropertyView* CreateTextSourcePropertyDetailView(obs_proxy::SceneItem* scene_item)
{
    return new TextPropDetailView(scene_item);
}

}   // namespace livehime


// TextPropDetailView
TextPropDetailView::TextPropDetailView(obs_proxy::SceneItem* scene_item)
    : data_loaded_(false),
      name_edit_(nullptr),
      text_area_(nullptr),
      load_file_checkbox_(nullptr),
      browse_file_button_(nullptr),
      file_path_edit_(nullptr),
      font_name_combo_(nullptr),
      font_size_combo_(nullptr),
      font_style_combo_(nullptr),
      color_button_(nullptr),
      use_outline_checkbox_(nullptr),
      outline_color_button_(nullptr),
      outline_slider_(nullptr),
      outline_value_label_(nullptr),
      horizontal_scroll_slider_(nullptr),
      horizontal_scroll_value_label_(nullptr),
      vertical_scroll_slider_(nullptr),
      vertical_scroll_value_label_(nullptr),
      transparent_slider_(nullptr),
      transparent_value_label_(nullptr),
      weakptr_factory_(this),
      presenter_(std::make_unique<SourceTextPropertyPresenterImpl>(scene_item))
{
}

void TextPropDetailView::InitData()
{
    name_edit_->SetText(presenter_->GetName());
    auto text = presenter_->GetText();
    text_area_->SetText(text);
    load_file_checkbox_->SetChecked(presenter_->GetIsFromFile());
    file_path_edit_->text_field()->SetText(presenter_->GetTextFilePath());
    font_color_ = presenter_->GetFontColor();

    use_outline_checkbox_->SetChecked(presenter_->GetOutline());
    outline_color_ = presenter_->GetOutlineColor();
    outline_slider_->SetValue(presenter_->GetOutlineSize());

    horizontal_scroll_slider_->SetValue(presenter_->GetHorizontalScrollSpeed());
    vertical_scroll_slider_->SetValue(presenter_->GetVerticalScrollSpeed());

    transparent_slider_->SetValue(presenter_->GetTransparent());

    InitFontData();

    UpdateControlStatus();

    presenter_->Snapshot();

    data_loaded_ = true;
}

bool TextPropDetailView::CheckSetupValid()
{
    return true;
}

bool TextPropDetailView::SaveSetupChange()
{
    if (!data_loaded_)
    {
        return false;
    }

    presenter_->SetText(text_area_->GetText());
    presenter_->SetIsFromFile(load_file_checkbox_->checked());
    presenter_->SetTextFilePath(file_path_edit_->text_field()->text());

    presenter_->Update();

    if (presenter_->SetName(name_edit_->GetText()) == false)
    {
        livehime::ShowMessageBox(livehime::UniversalMsgboxType::CannotRenameSceneItem);
        return false;
    }

    return true;
}

bool TextPropDetailView::Cancel()
{
    presenter_->Restore();

    return true;
}

void TextPropDetailView::OnCheckSourceText(bool valid_response, int code, bool is_sensitive)
{
    if (valid_response && code == 0)
    {
        check_sensitive_(is_sensitive);
        if (is_sensitive)
        {
            HotkeyNotifyView::ShowForm(GetLocalizedString(IDS_SRCPROP_TEXT_CONTAIN_SENSITIVE),
                GetBoundsInScreen(), nullptr, &ftPrimary);

            base::string16 area_text = text_area_->GetText();
            livehime::PolarisEventReport(
                secret::LivehimePolarisBehaviorEvent::SourceTextSensitiveShow,
                "word:" + base::UTF16ToUTF8(area_text));
        }
    }
    else
    {
        check_sensitive_(true);
        HotkeyNotifyView::ShowForm(GetLocalizedString(IDS_SRCPROP_TEXT_INVALID_OPERATION),
            GetBoundsInScreen(), nullptr, &ftPrimary);
    }

}

void TextPropDetailView::CheckSourceText(CheckSorceTextHandler handle)
{
    // 海外开播不需要敏感词检测
    check_sensitive_ = handle;
    check_sensitive_(false);
}

gfx::ImageSkia* TextPropDetailView::GetSkiaIcon()
{
    return ui::ResourceBundle::GetSharedInstance().
        GetImageSkiaNamed(IDR_LIVEHIME_SOURCE_PROPERTY_TEXT_TITLE);
}

std::wstring TextPropDetailView::GetCaption()
{
    auto& res = ui::ResourceBundle::GetSharedInstance();
    return res.GetLocalizedString(IDS_SRCPROP_TEXT_CAPTION);
}

void TextPropDetailView::InitFontData()
{
    auto& res = ui::ResourceBundle::GetSharedInstance();

    // init font name
    std::vector<string16> font_families = presenter_->GetSystemFontFamilies();
    string16 font_name = presenter_->GetFontName();
    for (auto it = font_families.begin(); it != font_families.end(); ++it)
    {
        int index = font_name_combo_->AddItem(*it);
        if (font_name.compare(*it) == 0)
        {
            font_name_combo_->SetSelectedIndex(index);
        }
    }

    // init font size
    presenter_->InitFontSizeFromString(
        res.GetLocalizedString(IDS_SRCPROP_TEXT_FONT_SIZE));
    std::vector<string16> font_size = presenter_->GetInitFontSize();
    string16 current_font_size(presenter_->GetFontSize());

    for (auto it = font_size.begin(); it != font_size.end(); ++it)
    {
        int index = font_size_combo_->AddItem(*it);
    }

    int index = font_size_combo_->FindItem(current_font_size);
    if (index == -1)
    {
        font_size_combo_->SetSelectedIndex(font_size_combo_->AddItem(current_font_size));
    }
    else
    {
        font_size_combo_->SetSelectedIndex(index);
    }

    // init font weight
    font_style_combo_->AddItem(res.GetLocalizedString(IDS_SRCPROP_TEXT_DEFAULT));
    font_style_combo_->AddItem(res.GetLocalizedString(IDS_SRCPROP_TEXT_BOLD));
    font_style_combo_->AddItem(res.GetLocalizedString(IDS_SRCPROP_TEXT_ITALIC));
    font_style_combo_->AddItem(res.GetLocalizedString(IDS_SRCPROP_TEXT_BOLD_ITALIC));

    font_style_combo_->SetSelectedIndex(presenter_->GetFontStyle());
}

void TextPropDetailView::UpdateControlStatus()
{
    text_area_->SetEnabled(!load_file_checkbox_->checked());
    file_path_edit_->SetEnabled(load_file_checkbox_->checked());
    browse_file_button_->SetEnabled(load_file_checkbox_->checked());

    outline_color_button_->SetEnabled(use_outline_checkbox_->checked());
    outline_slider_->SetEnabled(use_outline_checkbox_->checked());

    color_button_->SetFixedBkColor(font_color_);
    outline_color_button_->SetFixedBkColor(outline_color_);
}

void TextPropDetailView::EffectiveImmediately(EffectiveControl effective_control)
{
    switch (effective_control)
    {
    case TEXT_INVALID:
        return;
    case TEXT_FONT:
        EffectiveFontData();
        break;
    case TEXT_OUTLINE:
        EffectiveOutlineData();
        break;
    case TEXT_HORIZONTAL_SCROLL:
        presenter_->SetHorizontalScrollSpeed(horizontal_scroll_slider_->value());
        break;
    case TEXT_VERTICAL_SCROLL:
        presenter_->SetVerticalScrollSpeed(vertical_scroll_slider_->value());
        break;
    case TEXT_TRANSPARENT:
        presenter_->SetTransparent(transparent_slider_->value());
        break;
    }

    presenter_->Update();
}

void TextPropDetailView::EffectiveFontData()
{
    // save font name
    presenter_->SetFontName(
        font_name_combo_->GetItemText(
        font_name_combo_->selected_index()));

    // save font height
    presenter_->SetFontSize(
        font_size_combo_->GetItemText(
        font_size_combo_->selected_index()));

    // save font weight
    presenter_->SetFontStyle(
        font_style_combo_->selected_index());

    // save font color
    presenter_->SetFontColor(font_color_);
}

void TextPropDetailView::EffectiveOutlineData()
{
    presenter_->SetOutline(use_outline_checkbox_->checked());

    presenter_->SetOutlineColor(outline_color_);

    presenter_->SetOutlineSize(outline_slider_->value());
}

void TextPropDetailView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    EffectiveControl effective_control = TEXT_INVALID;
    if (sender == load_file_checkbox_)
    {
        UpdateControlStatus();
    }
    else if (sender == browse_file_button_)
    {
        auto& res = ui::ResourceBundle::GetSharedInstance();

        std::unique_ptr<ISelectFileDialog> dialog{ CreateSelectFileDialog(sender->GetWidget()) };
        dialog->SetDefaultPath(file_path_edit_->text_field()->text());
        dialog->ClearFilter();
        dialog->AddFilter(res.GetLocalizedString(IDS_FILEDIALOG_TEXT_FILTER), GetTextFileExts());
        bool selected = dialog->DoModel();
        if (selected)
        {
            file_path_edit_->text_field()->SetText(dialog->GetSelectedFileName());
        }
    }
    else if (sender == color_button_ || sender == outline_color_button_)
    {
        SkColor& color =
            sender == color_button_ ? font_color_ : outline_color_;
        SelectColorDialog dlg;
        dlg.SetCurrentColor(color);
        if (dlg.DoModel(sender->GetWidget()) == SelectColorDialog::RESULT_OK)
            color = dlg.GetCurrentColor();

        UpdateControlStatus();

        effective_control =
            sender == color_button_ ? TEXT_FONT : TEXT_OUTLINE;
    }
    else if (sender == use_outline_checkbox_)
    {
        UpdateControlStatus();

        effective_control = TEXT_OUTLINE;
    }

    EffectiveImmediately(effective_control);
}

void TextPropDetailView::SliderValueChanged(
    views::Slider* sender,
    float value,
    float old_value,
    views::SliderChangeReason reason)
{
    EffectiveControl effective_control = TEXT_INVALID;
    if (data_loaded_ && reason == views::SliderChangeReason::VALUE_CHANGED_BY_API)
    {
        ;
    }
    else if (sender == outline_slider_)
    {
        outline_value_label_->SetText(
            base::StringPrintf(L"%d",
            MapFloatToInt(outline_slider_->value(),
            kOutlineSizeMin, kOutlineSizeMax)));

        effective_control = TEXT_OUTLINE;
    }
    else if (sender == transparent_slider_)
    {
        transparent_value_label_->SetText(
            base::StringPrintf(L"%d",
            MapFloatToInt(transparent_slider_->value())));

        effective_control = TEXT_TRANSPARENT;
    }
    else if (sender == horizontal_scroll_slider_)
    {
        horizontal_scroll_value_label_->SetText(
            base::StringPrintf(L"%d",
            MapFloatToInt(horizontal_scroll_slider_->value(),
            kScrollSpeedMin, kScrollSpeedMax)));

        effective_control = TEXT_HORIZONTAL_SCROLL;
    }
    else if (sender == vertical_scroll_slider_)
    {
        vertical_scroll_value_label_->SetText(
            base::StringPrintf(L"%d",
            MapFloatToInt(vertical_scroll_slider_->value(),
            kScrollSpeedMin, kScrollSpeedMax)));

        effective_control = TEXT_VERTICAL_SCROLL;
    }

    EffectiveImmediately(effective_control);
}

void TextPropDetailView::OnSelectedIndexChanged(BililiveComboboxEx* combobox)
{
    if (combobox == font_name_combo_ || combobox == font_size_combo_ || combobox == font_style_combo_)
    {
        EffectiveImmediately(TEXT_FONT);
    }
    if (combobox == font_name_combo_)
    {
        cur_sel_font_name_ = font_name_combo_->GetItemText(font_name_combo_->selected_index());
    }
}

void TextPropDetailView::OnBililiveComboboxExEditBlur(
    BililiveComboboxEx* combobox,
    const base::string16 &text,
    int reason)
{
    if (combobox == font_size_combo_)
    {
        int64 data = 0;
        base::StringToInt64(text, &data);
        if (data > 0)
        {
            int index = combobox->FindItem(text);
            if (-1 == index)
            {
                index = combobox->AddItem(text);
            }
            combobox->SetSelectedIndex(index);
        }
        EffectiveImmediately(TEXT_FONT);
    }
    else if (combobox == font_name_combo_)
    {
        font_name_combo_->ClearItems();
        std::vector<string16> search_result_data = presenter_->GetSystemFontFamilies();
        if (!search_result_data.empty())
        {
            for (auto list_it = search_result_data.begin(); list_it != search_result_data.end(); list_it++)
            {
                font_name_combo_->AddItem(*list_it);
            }
        }
        int index = font_name_combo_->FindItem(cur_sel_font_name_);
        if (index != -1)
        {
            font_name_combo_->SetSelectedIndex(index);
        }
        else
        {
            font_name_combo_->SetSelectedIndex(0);
        }
    }

}

base::string16 TextPropDetailView::OnBililiveComboboxExEditShow(BililiveComboboxEx* combobox,
    const base::string16 &label_text)
{
    if (combobox == font_name_combo_)
    {
        cur_sel_font_name_ = label_text;
    }
    return label_text;
}

gfx::Size TextPropDetailView::GetPreferredSize()
{
    gfx::Size size = __super::GetPreferredSize();
    size.set_width(std::min(size.width(), GetLengthByDPIScale(490)));
    return size;
}

void TextPropDetailView::InitView()
{
    auto& res = ui::ResourceBundle::GetSharedInstance();
    views::GridLayout* layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    views::ColumnSet* column_set = layout->AddColumnSet(TextPropDetailColumnSet::NAMECOLUMNSET);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::FIXED, MaxLabelWidth(), 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

    column_set = layout->AddColumnSet(TextPropDetailColumnSet::TEXTAREACOLUMNSET);
    column_set->AddPaddingColumn(0, NonHeaderColumnPadding());
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

    column_set = layout->AddColumnSet(TextPropDetailColumnSet::FILECOLUMNSET);
    column_set->AddPaddingColumn(0, NonHeaderColumnPadding());
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

    BililiveLabel* label = new LivehimeTitleLabel(res.GetLocalizedString(IDS_SRCPROP_COMMON_SOURCENAME));
    label->SetHorizontalAlignment(gfx::ALIGN_RIGHT);
    name_edit_ = new LivehimeNativeEditView();

    layout->AddPaddingRow(0, kPaddingRowHeightEnds);
    layout->StartRow(0, TextPropDetailColumnSet::NAMECOLUMNSET);
    layout->AddView(label);
    layout->AddView(name_edit_);

    text_area_ = new LivehimeNativeEditView();
    text_area_->SetMultiLine(true, false, true);

    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, TextPropDetailColumnSet::TEXTAREACOLUMNSET);
    layout->AddView(text_area_, 1, 1, views::GridLayout::Alignment::FILL, views::GridLayout::Alignment::FILL, 0, GetLengthByDPIScale(105));

    load_file_checkbox_ = new LivehimeCheckbox(res.GetLocalizedString(IDS_SRCPROP_TEXT_LOADFILE));
    load_file_checkbox_->set_listener(this);
    views::View* view = new views::View();
    {
        views::GridLayout* grid_layout = new views::GridLayout(view);
        view->SetLayoutManager(grid_layout);

        views::ColumnSet* column_set = grid_layout->AddColumnSet(0);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0.0f, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
        column_set->AddColumn(
            views::GridLayout::FILL,
            views::GridLayout::FILL,
            0.0f,
            views::GridLayout::FIXED,
            LivehimePaddingCharWidth(ftPrimary) * 10,
            0);

        browse_file_button_ = new LivehimeFunctionLabelButton(this, res.GetLocalizedString(IDS_SRCPROP_TEXT_BROWSE));
        file_path_edit_ = new LivehimeTextfield();

        grid_layout->StartRow(0, 0);
        grid_layout->AddView(browse_file_button_);
        grid_layout->AddView(file_path_edit_);
    }

    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, TextPropDetailColumnSet::FILECOLUMNSET);
    layout->AddView(load_file_checkbox_);
    layout->AddView(view);

    label = new LivehimeTitleLabel(res.GetLocalizedString(IDS_SRCPROP_TEXT_FONT));
    label->SetHorizontalAlignment(gfx::ALIGN_RIGHT);
    view = new views::View();
    {
        views::GridLayout* grid_layout = new views::GridLayout(view);
        view->SetLayoutManager(grid_layout);

        int column_width = LivehimePaddingCharWidth(ftPrimary) * 8;

        views::ColumnSet* column_set = grid_layout->AddColumnSet(0);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0.0f, views::GridLayout::FIXED, column_width, 0);
        column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0.0f, views::GridLayout::FIXED, LivehimePaddingCharWidth(ftPrimary) * 5, 0);
        column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0.0f, views::GridLayout::FIXED, column_width, 0);
        column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0.0f, views::GridLayout::FIXED, 50, 0);

        font_name_combo_ = new LivehimeCombobox(true);
        font_name_combo_->set_listener(this);
        font_size_combo_ = new LivehimeCombobox(true);
        font_size_combo_->set_listener(this);
        font_size_combo_->SetEditInputType(BililiveComboboxEx::EditInputType::EDIT_INPUT_TYPE_NUMBER);
        font_style_combo_ = new LivehimeCombobox();
        font_style_combo_->set_listener(this);
        color_button_ = new LivehimeFunctionLabelButton(this, L"");

        grid_layout->StartRow(0, 0);
        grid_layout->AddView(font_name_combo_);
        grid_layout->AddView(font_size_combo_);
        grid_layout->AddView(font_style_combo_);
        grid_layout->AddView(color_button_);
    }

    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, TextPropDetailColumnSet::NAMECOLUMNSET);
    layout->AddView(label);
    layout->AddView(view);

    label = new LivehimeTitleLabel(res.GetLocalizedString(IDS_SRCPROP_TEXT_OUTLINE));
    label->SetHorizontalAlignment(gfx::ALIGN_RIGHT);
    view = new views::View();
    {
        views::GridLayout* grid_layout = new views::GridLayout(view);
        view->SetLayoutManager(grid_layout);

        views::ColumnSet* column_set = grid_layout->AddColumnSet(0);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0.0f, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0.0f, views::GridLayout::FIXED, 50, 0);
        column_set->AddPaddingColumn(1.0f, 0);

        use_outline_checkbox_ = new LivehimeCheckbox(res.GetLocalizedString(IDS_SRCPROP_TEXT_USE_OUTLINE));
        use_outline_checkbox_->set_listener(this);
        outline_color_button_ = new LivehimeFunctionLabelButton(this, L"");

        grid_layout->StartRow(0, 0);
        grid_layout->AddView(use_outline_checkbox_);
        grid_layout->AddView(outline_color_button_);
    }

    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, TextPropDetailColumnSet::NAMECOLUMNSET);
    layout->AddView(label);
    layout->AddView(view);

    label = new LivehimeTitleLabel();
    view = new views::View();
    {
        views::GridLayout* grid_layout = new views::GridLayout(view);
        view->SetLayoutManager(grid_layout);

        column_set = grid_layout->AddColumnSet(0);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::FIXED,
            LivehimeContentLabel::GetFont().GetStringWidth(L"100"), 0);

        outline_slider_ = new LivehimeSlider(this, views::Slider::HORIZONTAL);
        outline_slider_->SetKeyboardIncrement(1.0f / 20.0f);
        outline_value_label_ = new LivehimeContentLabel(L"1");

        grid_layout->StartRow(0, 0);
        grid_layout->AddView(outline_slider_);
        grid_layout->AddView(outline_value_label_);
    }

    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, TextPropDetailColumnSet::NAMECOLUMNSET);
    layout->AddView(label);
    layout->AddView(view);

    label = new LivehimeTitleLabel(res.GetLocalizedString(IDS_SRCPROP_TEXT_HORIZONTAL_SCROLL));
    label->SetHorizontalAlignment(gfx::ALIGN_RIGHT);
    view = new views::View();
    {
        views::GridLayout* grid_layout = new views::GridLayout(view);
        view->SetLayoutManager(grid_layout);

        views::ColumnSet* column_set = grid_layout->AddColumnSet(0);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::FIXED,
            LivehimeContentLabel::GetFont().GetStringWidth(L"100"), 0);

        horizontal_scroll_slider_ = new LivehimeSlider(this, views::Slider::HORIZONTAL);
        horizontal_scroll_value_label_ = new LivehimeContentLabel(L"0");

        grid_layout->StartRow(0, 0);
        grid_layout->AddView(horizontal_scroll_slider_);
        grid_layout->AddView(horizontal_scroll_value_label_);
    }

    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, TextPropDetailColumnSet::NAMECOLUMNSET);
    layout->AddView(label);
    layout->AddView(view);

    label = new LivehimeTitleLabel(res.GetLocalizedString(IDS_SRCPROP_TEXT_VERTICAL_SCROLL));
    label->SetHorizontalAlignment(gfx::ALIGN_RIGHT);
    view = new views::View();
    {
        views::GridLayout* grid_layout = new views::GridLayout(view);
        view->SetLayoutManager(grid_layout);

        views::ColumnSet* column_set = grid_layout->AddColumnSet(0);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::FIXED,
            LivehimeContentLabel::GetFont().GetStringWidth(L"100"), 0);

        vertical_scroll_slider_ = new LivehimeSlider(this, views::Slider::HORIZONTAL);
        vertical_scroll_value_label_ = new LivehimeContentLabel(L"0");

        grid_layout->StartRow(0, 0);
        grid_layout->AddView(vertical_scroll_slider_);
        grid_layout->AddView(vertical_scroll_value_label_);
    }

    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, TextPropDetailColumnSet::NAMECOLUMNSET);
    layout->AddView(label);
    layout->AddView(view);

    label = new LivehimeTitleLabel(res.GetLocalizedString(IDS_SRCPROP_TEXT_TRANSPARENT));
    label->SetHorizontalAlignment(gfx::ALIGN_RIGHT);
    view = new views::View();
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

        grid_layout->StartRow(0, 0);
        grid_layout->AddView(transparent_slider_);
        grid_layout->AddView(transparent_value_label_);
    }

    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, TextPropDetailColumnSet::NAMECOLUMNSET);
    layout->AddView(label);
    layout->AddView(view);
}

void TextPropDetailView::OnBililiveComboboxExEditContentsChanged(BililiveComboboxEx* combobox, const string16& new_contents)
{
    if (combobox == font_name_combo_ && combobox->IsEditFocus())
    {
        base::string16 search_text = combobox->GetText();
        font_name_combo_->ClearItems();
        SearchStringInData(combobox->GetText());
    }
}

void TextPropDetailView::SearchStringInData(base::string16 source_str)
{
    std::vector<string16> search_result_data;
    if (source_str.empty())
    {
        search_result_data = presenter_->GetSystemFontFamilies();
    }
    else
    {
        search_result_data = presenter_->SearchString(source_str);
    }

    if (!search_result_data.empty())
    {
        for (auto list_it = search_result_data.begin(); list_it != search_result_data.end(); list_it++)
        {
            font_name_combo_->AddItem(*list_it);
        }
    }

    font_name_combo_->DoDropDown();
}