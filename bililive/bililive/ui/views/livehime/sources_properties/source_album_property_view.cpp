#include "bililive/bililive/ui/views/livehime/sources_properties/source_album_property_view.h"

#include "base/strings/stringprintf.h"
#include "bilibase/basic_types.h"

#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/livehime/sources_properties/source_album_property_presenter_impl.h"
#include "bililive/bililive/ui/dialog/shell_dialogs.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_hover_tip_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/utils/bililive_filetype_exts.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/grid_layout.h"

#include "grit/generated_resources.h"
#include "grit/theme_resources.h"


namespace
{

using namespace bililive;

const int kSlidetimeMin = 0;
const int kSlidetimeMax = 10000;

const int kTransitiontimeMin = 0;
const int kTransitiontimeMax = 5000;

const int kListViewWidth = GetLengthByDPIScale(325);
const int kListViewHeight = GetLengthByDPIScale(150);

// FileListItemView
class FileListItemView : public ListItemView
{
public:
    FileListItemView(ListView *list_view, const string16& file_name)
        : ListItemView(list_view),
          list_view_(list_view)
    {
        SetLayoutManager(new views::FillLayout());
        set_border(views::Border::CreateEmptyBorder(0, kCtrlLeftInset, 0, kCtrlRightInset));

        content_ = new LivehimeContentLabel(file_name);
        content_->SetTooltipText(file_name);
        content_->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
        content_->SetElideBehavior(views::Label::ELIDE_AT_END);

        AddChildView(content_);
    }

protected:
    void OnSelectedChanged(ListItemViewChangeReason reason) override
    {
        content_->SetTextColor(checked() ? clrLivehime : clrTextSecondary);
    }

    int GetHeightForWidth(int w) override
    {
        return GetPreferredSize().height();
    }

private:
    LivehimeContentLabel *content_;

    ListView *list_view_;

    DISALLOW_COPY_AND_ASSIGN(FileListItemView);
};

}   // namespace

namespace livehime
{

BasePropertyView* CreateAlbumSourcePropertyDetailView(obs_proxy::SceneItem* scene_item)
{
    return new AlbumPropDetailView(scene_item);
}

}   // namespace livehime


// AlbumPropDetailView
AlbumPropDetailView::AlbumPropDetailView(obs_proxy::SceneItem* scene_item)
    : data_loaded_(false),
      name_edit_(nullptr),
      add_file_button_(nullptr),
      add_directory_button_(nullptr),
      delete_button_(nullptr),
      empty_button_(nullptr),
      file_list_view_(nullptr),
      transition_combobox_(nullptr),
      play_time_slider_(nullptr),
      play_time_value_label_(nullptr),
      transition_slider_(nullptr),
      transition_value_label_(nullptr),
      loop_checkbox_(nullptr),
      random_checkbox_(nullptr),
      presenter_(std::make_unique<SourceAlbumPropertyPresenterImpl>(scene_item))
{
}

void AlbumPropDetailView::InitView()
{
    auto& res = ui::ResourceBundle::GetSharedInstance();
    views::GridLayout* layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    views::ColumnSet* column_set = layout->AddColumnSet(0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::FIXED, MaxLabelWidth(), 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

    BililiveLabel* label = new LivehimeTitleLabel(res.GetLocalizedString(IDS_SRCPROP_ALBUM_CAPTION));
    label->SetHorizontalAlignment(gfx::ALIGN_RIGHT);
    views::View* view = new views::View();
    {
        views::GridLayout* layout = new views::GridLayout(view);
        view->SetLayoutManager(layout);

        views::ColumnSet* column_set = layout->AddColumnSet(0);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);

        name_edit_ = new LivehimeNativeEditView();
        LivehimeHoverTipButton* album_tips = new LivehimeHoverTipButton(res.GetLocalizedString(IDS_SRCPROP_COMMON_TIPS_TITLE),
            res.GetLocalizedString(IDS_SRCPROP_ALBUM_TIPS),
            views::BubbleBorder::Arrow::BOTTOM_RIGHT);

        layout->StartRow(0, 0);
        layout->AddView(name_edit_);
        layout->AddView(album_tips);
    }

    layout->AddPaddingRow(0, kPaddingRowHeightEnds);
    layout->StartRow(0, 0);
    layout->AddView(label);
    layout->AddView(view);

    label = new LivehimeTitleLabel(res.GetLocalizedString(IDS_SRCPROP_ALBUM_ADD));
    label->SetHorizontalAlignment(gfx::ALIGN_RIGHT);
    view = new views::View();
    {
        views::GridLayout* layout = new views::GridLayout(view);
        view->SetLayoutManager(layout);

        views::ColumnSet* column_set = layout->AddColumnSet(0);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddPaddingColumn(1.0f, 0);

        add_file_button_ = new LivehimeFunctionLabelButton(this, res.GetLocalizedString(IDS_SRCPROP_ALBUM_ADD_FILE));
        add_directory_button_ = new LivehimeFunctionLabelButton(this, res.GetLocalizedString(IDS_SRCPROP_ALBUM_ADD_DIRECTORY));
        delete_button_ = new LivehimeFunctionLabelButton(this, res.GetLocalizedString(IDS_SRCPROP_ALBUM_DELETE));
        empty_button_ = new LivehimeFunctionLabelButton(this, res.GetLocalizedString(IDS_SRCPROP_ALBUM_EMPTY));

        layout->StartRow(0, 0);
        layout->AddView(add_file_button_);
        layout->AddView(add_directory_button_);
        layout->AddView(delete_button_);
        layout->AddView(empty_button_);
    }

    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, 0);
    layout->AddView(label);
    layout->AddView(view);

    label = new LivehimeTitleLabel();
    file_list_view_ = new ListView(nullptr);
    file_list_view_->SetItemSize(gfx::Size(0, ftPrimary.GetHeight() * 1.5f));
    file_list_view_->SetItemBackgroundColor(views::Button::STATE_NORMAL, SK_ColorWHITE);
    file_list_view_->SetItemBackgroundColor(views::Button::STATE_HOVERED, SK_ColorWHITE);
    file_list_view_->SetItemBackgroundColor(views::Button::STATE_PRESSED, SK_ColorWHITE);
    file_list_view_->Container()->
        set_border(views::Border::CreateSolidBorder(1, clrControlBorder));

    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, 0);
    layout->AddView(label);
    layout->AddView(file_list_view_->Container(), 1, 1,
        views::GridLayout::Alignment::FILL,
        views::GridLayout::Alignment::FILL,
        0, kListViewHeight);

    LivehimeTipLabel* tips = new LivehimeTipLabel(res.GetLocalizedString(IDS_SRCPROP_ALBUM_FILES_TIPS));
    tips->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);

    layout->AddPaddingRow(0, kPaddingRowHeightForCtrlTips);
    layout->StartRow(0, 0);
    layout->AddView(label);
    layout->AddView(tips);

    label = new LivehimeTitleLabel(res.GetLocalizedString(IDS_SRCPROP_ALBUM_TRANSITION));
    label->SetHorizontalAlignment(gfx::ALIGN_RIGHT);
    view = new views::View();
    {
        views::GridLayout* layout = new views::GridLayout(view);
        view->SetLayoutManager(layout);

        views::ColumnSet* column_set = layout->AddColumnSet(0);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);

        transition_combobox_ = new LivehimeCombobox();
        transition_combobox_->set_listener(this);

        layout->StartRow(0, 0);
        layout->AddView(transition_combobox_);
    }

    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, 0);
    layout->AddView(label);
    layout->AddView(view);

    label = new LivehimeTitleLabel(res.GetLocalizedString(IDS_SRCPROP_ALBUM_PLAYTIME));
    label->SetHorizontalAlignment(gfx::ALIGN_RIGHT);

    int slider_label_width = LivehimeContentLabel::GetFont().GetStringWidth(L"10000");
    view = new views::View();
    {
        views::GridLayout* layout = new views::GridLayout(view);
        view->SetLayoutManager(layout);

        views::ColumnSet* column_set = layout->AddColumnSet(0);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::FIXED, slider_label_width, 0);

        play_time_slider_ = new LivehimeSlider(this, views::Slider::HORIZONTAL);
        play_time_slider_->SetKeyboardIncrement(100.0f / 9900.0f);
        play_time_slider_->SetMouseWheelIncrement(100.0f / 9900.0f);
        play_time_value_label_ = new LivehimeContentLabel(L"100");

        layout->StartRow(0, 0);
        layout->AddView(play_time_slider_);
        layout->AddView(play_time_value_label_);
    }

    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, 0);
    layout->AddView(label);
    layout->AddView(view);

    label = new LivehimeTitleLabel(res.GetLocalizedString(IDS_SRCPROP_ALBUM_TRANSITION_TIME));
    label->SetHorizontalAlignment(gfx::ALIGN_RIGHT);
    view = new views::View();
    {
        views::GridLayout* layout = new views::GridLayout(view);
        view->SetLayoutManager(layout);

        views::ColumnSet* column_set = layout->AddColumnSet(0);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::FIXED, slider_label_width, 0);

        transition_slider_ = new LivehimeSlider(this, views::Slider::HORIZONTAL);
        transition_slider_->SetKeyboardIncrement(50.0f / 4950.0f);
        transition_slider_->SetMouseWheelIncrement(50.0f / 4950.0f);
        transition_value_label_ = new LivehimeContentLabel(L"50");

        layout->StartRow(0, 0);
        layout->AddView(transition_slider_);
        layout->AddView(transition_value_label_);
    }

    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, 0);
    layout->AddView(label);
    layout->AddView(view);

    label = new LivehimeTitleLabel(res.GetLocalizedString(IDS_SRCPROP_ALBUM_PLAY));
    label->SetHorizontalAlignment(gfx::ALIGN_RIGHT);
    view = new views::View();
    {
        views::GridLayout* layout = new views::GridLayout(view);
        view->SetLayoutManager(layout);

        views::ColumnSet* column_set = layout->AddColumnSet(0);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);

        loop_checkbox_ = new LivehimeCheckbox(res.GetLocalizedString(IDS_SRCPROP_ALBUM_LOOP));
        loop_checkbox_->set_listener(this);
        random_checkbox_ = new LivehimeCheckbox(res.GetLocalizedString(IDS_SRCPROP_ALBUM_RANDOM));
        random_checkbox_->set_listener(this);

        layout->StartRow(0, 0);
        layout->AddView(loop_checkbox_);
        layout->AddView(random_checkbox_);
    }

    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, 0);
    layout->AddView(label);
    layout->AddView(view);
}

void AlbumPropDetailView::InitData()
{
    name_edit_->SetText(presenter_->GetName());

    InitFileList();
    InitTransitionComb();
    play_time_slider_->SetValue(presenter_->GetSlideTime());
    transition_slider_->SetValue(presenter_->GetTransitionTime());
    loop_checkbox_->SetChecked(presenter_->GetLoop());
    random_checkbox_->SetChecked(presenter_->GetRandom());

    presenter_->Snapshot();

    data_loaded_ = true;
}

void AlbumPropDetailView::InitFileList()
{
    auto file_list = presenter_->GetFileList();

    for (auto it = file_list.begin(); it != file_list.end(); ++it)
    {
        file_list_view_->AddItemView(new FileListItemView(file_list_view_, *it));
        presenter_->AddFile(*it);
    }
}

void AlbumPropDetailView::InitTransitionComb()
{
    auto& res = ui::ResourceBundle::GetSharedInstance();
    using TransitionStyle = SlideShowSceneItemHelper::TransitionT;

    transition_combobox_->AddItem(
        res.GetLocalizedString(IDS_SRCPROP_ALBUM_TRANSITION_STYLE_DEFAULT),
        bilibase::enum_cast(TransitionStyle::kDefault));
    transition_combobox_->AddItem(
        res.GetLocalizedString(IDS_SRCPROP_ALBUM_TRANSITION_STYLE_LEFT_OUT),
        bilibase::enum_cast(TransitionStyle::kLeftOut));
    transition_combobox_->AddItem(
        res.GetLocalizedString(IDS_SRCPROP_ALBUM_TRANSITION_STYLE_RIGHT_IN),
        bilibase::enum_cast(TransitionStyle::kRightIn));
    transition_combobox_->AddItem(
        res.GetLocalizedString(IDS_SRCPROP_ALBUM_TRANSITION_STYLE_FADE_OUT),
        bilibase::enum_cast(TransitionStyle::kFadeOut));

    transition_combobox_->SetSelectedIndex(
        transition_combobox_->FindItemData(
        bilibase::enum_cast(presenter_->GetTransition())));
}

bool AlbumPropDetailView::CheckSetupValid()
{
    return true;
}

bool AlbumPropDetailView::SaveSetupChange()
{
    if (!data_loaded_)
    {
        return false;
    }

    presenter_->SetFileList();

    presenter_->Update();

    if (presenter_->SetName(
        name_edit_->GetText()) == false)
    {
        livehime::ShowMessageBox(livehime::UniversalMsgboxType::CannotRenameSceneItem);
        return false;
    }

    return true;
}

bool AlbumPropDetailView::Cancel()
{
    presenter_->Restore();

    return true;
}

gfx::ImageSkia* AlbumPropDetailView::GetSkiaIcon()
{
    return ui::ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEHIME_SOURCE_PROPERTY_PICTURE_TITLE);
}

std::wstring AlbumPropDetailView::GetCaption()
{
    auto& res = ui::ResourceBundle::GetSharedInstance();
    return res.GetLocalizedString(IDS_SRCPROP_ALBUM_CAPTION);
}

void AlbumPropDetailView::OnSelectedIndexChanged(BililiveComboboxEx* combobox)
{
    if (combobox == transition_combobox_)
    {
        EffectiveImmediately(ALBUM_TRANSITION);
    }
}

void AlbumPropDetailView::SliderValueChanged(
    views::Slider* sender,
    float value,
    float old_value,
    views::SliderChangeReason reason)
{
    EffectiveControl effective_control = ALBUM_INVALID;
    if (data_loaded_ && reason == views::SliderChangeReason::VALUE_CHANGED_BY_API)
    {
    ;
    }
    else if (sender == play_time_slider_)
    {
        float val = std::round(play_time_slider_->value() * 100) / 100.0;
        if (val < 0.01)
        {
            val = 0.01f;
        }
        play_time_value_label_->SetText(
            base::StringPrintf(L"%d",
            MapFloatToInt(val,
            kSlidetimeMin, kSlidetimeMax)));

        effective_control = ALBUM_PLAY_TIME;
    }
    else if (sender == transition_slider_)
    {
        float val = std::round(transition_slider_->value() * 100) / 100.0;
        if (val < 0.01)
        {
            val = 0.01f;
        }
        transition_value_label_->SetText(
            base::StringPrintf(L"%d",
            MapFloatToInt(val,
            kTransitiontimeMin, kTransitiontimeMax)));

        effective_control = ALBUM_TRANSITION_TIME;
    }
    EffectiveImmediately(effective_control);
}

void AlbumPropDetailView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    EffectiveControl effective_control = ALBUM_INVALID;
    if (sender == add_file_button_)
    {
        auto& res = ui::ResourceBundle::GetSharedInstance();
        std::unique_ptr<ISelectFileDialog> dialog{ CreateSelectFileDialog(sender->GetWidget()) };
        dialog->ClearFilter();
        dialog->AddFilter(res.GetLocalizedString(IDS_FILEDIALOG_IMAGE_FILTER), GetImageFileExts());
        bool selected = dialog->DoModel(ui::SelectFileDialog::Type::SELECT_OPEN_MULTI_FILE);
        if (selected)
        {
            for (string16 file_path : dialog->GetSelectedFileNames())
            {
                file_list_view_->AddItemView(new FileListItemView(file_list_view_, file_path));
                presenter_->AddFile(file_path);
            }
            Layout();
        }
    }
    else if (sender == add_directory_button_)
    {
        auto& res = ui::ResourceBundle::GetSharedInstance();
        std::unique_ptr<ISelectFileDialog> dialog{ CreateSelectFileDialog(sender->GetWidget()) };
        dialog->ClearFilter();
        bool selected = dialog->DoModel(ui::SelectFileDialog::Type::SELECT_FOLDER);
        if (selected)
        {
            string16 file_path = dialog->GetSelectedFileName();
            file_list_view_->AddItemView(new FileListItemView(file_list_view_, file_path));
            presenter_->AddFile(file_path);

            Layout();
        }
    }
    else if (sender == delete_button_)
    {
        int select_index = file_list_view_->GetFirstSelectedItemPosition();
        ListItemView *view = file_list_view_->DeleteItem(select_index);
        if (view)
        {
            delete view;
        }
        presenter_->RemoveFile(select_index);
    }
    else if (sender == empty_button_)
    {
        file_list_view_->DeleteAllItems(true);

        presenter_->EmptyFileList();
    }
    else if (sender == loop_checkbox_)
    {
        effective_control = ALBUM_LOOP;
    }
    else if (sender == random_checkbox_)
    {
        effective_control = ALBUM_RANDOM;
    }

    EffectiveImmediately(effective_control);
}

void AlbumPropDetailView::EffectiveImmediately(EffectiveControl effective_control)
{
    switch (effective_control)
    {
    case ALBUM_INVALID:
        return;
    case ALBUM_TRANSITION:
        presenter_->SetTransition(
            static_cast<SlideShowSceneItemHelper::TransitionT>(
            transition_combobox_->GetItemData<int>(
            transition_combobox_->selected_index())));
        break;
    case ALBUM_PLAY_TIME:
    {
        float val = std::round(play_time_slider_->value() * 100) / 100.0;
        if (val < 0.01)
        {
            val = 0.01f;
        }
        presenter_->SetSlideTime(val);
        break;
    }
    case ALBUM_TRANSITION_TIME:
    {
        float val = std::round(transition_slider_->value() * 100) / 100.0;
        if (val < 0.01)
        {
            val = 0.01f;
        }
        presenter_->SetTransitionTime(val);
        break;
    }
    case ALBUM_LOOP:
        presenter_->SetLoop(loop_checkbox_->checked());
        break;
    case ALBUM_RANDOM:
        presenter_->SetRandom(random_checkbox_->checked());
        break;
    }

    presenter_->Update();
}