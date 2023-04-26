#include "bililive/bililive/ui/views/livehime/sources_properties/source_media_property_view.h"

#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"

#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/livehime/sources_properties/source_media_property_presenter_impl.h"
#include "bililive/bililive/ui/dialog/shell_dialogs.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_hover_tip_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/utils/bililive_filetype_exts.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/grid_layout.h"

#include "grit/generated_resources.h"
#include "grit/theme_resources.h"

namespace{

using namespace bililive;

enum MediaPropDetailColumnSet
{
    NAMECOLUMNSET = 0,
    FILEPATHCOLUMNSET,
    CHECKBOXCOLUMNSET,
    AUDIOCTRLSET
};

};  // namespace

namespace livehime
{

BasePropertyView* CreateMediaSourcePropertyDetailView(obs_proxy::SceneItem* scene_item)
{
    return new MediaPropDetailView(scene_item);
}

}   // namespace livehime

// MediaPropDetailView
MediaPropDetailView::MediaPropDetailView(obs_proxy::SceneItem* scene_item)
    : data_loaded_(false),
      name_edit_(nullptr),
      volume_control_view_(nullptr),
      file_path_(nullptr),
      browse_button_(nullptr),
      loop_checkbox_(nullptr),
      presenter_(std::make_shared<SourceMediaPropertyPresenterImpl>(scene_item, this))
{
    presenter_->Initialize();
}

MediaPropDetailView::~MediaPropDetailView()
{
}

void MediaPropDetailView::InitData()
{
    name_edit_->SetText(presenter_->GetName());
    file_path_->SetText(presenter_->GetFilePath());
    loop_checkbox_->SetChecked(presenter_->GetIsLoop());

    presenter_->Snapshot();

    data_loaded_ = true;
}

bool MediaPropDetailView::CheckSetupValid()
{
    return true;
}

bool MediaPropDetailView::SaveSetupChange()
{
    if (!data_loaded_)
    {
        return false;
    }

    presenter_->SetFilePath(file_path_->GetText());
    presenter_->SetIsLoop(loop_checkbox_->checked());
    presenter_->Update();

    if (presenter_->SetName(name_edit_->GetText()) == false)
    {
        livehime::ShowMessageBox(livehime::UniversalMsgboxType::CannotRenameSceneItem);
        return false;
    }

    return true;
}

void MediaPropDetailView::SetVtuber(const std::string& mp4_path, OnMediaStateCallHandler handler)
{
    media_state_handler_ = handler;
    presenter_->SetMP4MotionFilePath(base::UTF8ToUTF16(mp4_path));
    presenter_->SetIsLoop(false);
    presenter_->Update();
}

bool MediaPropDetailView::Cancel()
{
    presenter_->Restore();

    return true;
}

gfx::ImageSkia* MediaPropDetailView::GetSkiaIcon()
{
    return ui::ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEHIME_SOURCE_PROPERTY_MEDIA_TITLE);
}

std::wstring MediaPropDetailView::GetCaption()
{
    auto& res = ui::ResourceBundle::GetSharedInstance();
    return res.GetLocalizedString(IDS_SRCPROP_FFMPEG_CAPTION);
}


void MediaPropDetailView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    if (sender == browse_button_)
    {
        auto& res = ui::ResourceBundle::GetSharedInstance();

        std::unique_ptr<ISelectFileDialog> dialog{ CreateSelectFileDialog(sender->GetWidget()) };

        dialog->SetDefaultPath(file_path_->GetText());
        dialog->ClearFilter();
        dialog->AddFilter(res.GetLocalizedString(IDS_FILEDIALOG_MEDIA_FILTER), GetMediaFileExts());
        bool selected = dialog->DoModel();
        if (selected)
        {
            file_path_->SetText(dialog->GetSelectedFileName());
        }
    }
}

void MediaPropDetailView::OnMediaStateChanged(contracts::MediaState state) {
    if (state == contracts::MediaState::Ended)
    {
        if (media_state_handler_) {
            media_state_handler_(true);
        }
    }
    else if (state == contracts::MediaState::Error)
    {
        if (media_state_handler_) {
            media_state_handler_(false);
        }
    }
}



void MediaPropDetailView::InitView()
{
    auto& res = ui::ResourceBundle::GetSharedInstance();
    views::GridLayout* layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    views::ColumnSet* column_set = layout->AddColumnSet(MediaPropDetailColumnSet::NAMECOLUMNSET);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    column_set->AddColumn(views::GridLayout::TRAILING, views::GridLayout::CENTER, 0, views::GridLayout::FIXED, GetLengthByDPIScale(75), 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

    BililiveLabel* label = new LivehimeTitleLabel(res.GetLocalizedString(IDS_SRCPROP_COMMON_SOURCENAME));
    label->SetHorizontalAlignment(gfx::ALIGN_RIGHT);
    name_edit_ = new LivehimeNativeEditView();

    layout->AddPaddingRow(0, kPaddingRowHeightEnds);
    layout->StartRow(0, MediaPropDetailColumnSet::NAMECOLUMNSET);
    layout->AddView(label);
    layout->AddView(name_edit_);

    views::View* view = new views::View();
    {
        views::GridLayout* grid_layout = new views::GridLayout(view);
        view->SetLayoutManager(grid_layout);

        views::ColumnSet* column_set = grid_layout->AddColumnSet(0);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);

        volume_control_view_ = new SouceVolumeControlView(presenter_->GetSceneItem(), SourceVolumeCtrlType::Media);

        grid_layout->StartRow(0, 0);
        grid_layout->AddView(volume_control_view_);

    }

    column_set = layout->AddColumnSet(MediaPropDetailColumnSet::AUDIOCTRLSET);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, MediaPropDetailColumnSet::AUDIOCTRLSET);
    layout->AddView(view);

    column_set = layout->AddColumnSet(MediaPropDetailColumnSet::FILEPATHCOLUMNSET);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    column_set->AddColumn(views::GridLayout::TRAILING, views::GridLayout::CENTER, 0, views::GridLayout::FIXED, GetLengthByDPIScale(75), 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::FIXED, MaxControlWidth(), 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);


    file_path_ = new LivehimeNativeEditView();
    browse_button_ = new LivehimeFunctionLabelButton(this, res.GetLocalizedString(IDS_SRCPROP_IMAGE_BROWSE));
    LivehimeHoverTipButton* file_tip = new LivehimeHoverTipButton(res.GetLocalizedString(IDS_SRCPROP_COMMON_TIPS_TITLE),
        res.GetLocalizedString(IDS_SRCPROP_FFMPEG_TIP),
        views::BubbleBorder::Arrow::BOTTOM_RIGHT);

    label = new LivehimeTitleLabel(GetLocalizedString(IDS_SRCPROP_FFMPEG_STREAM_OR_FILE));
    label->SetHorizontalAlignment(gfx::ALIGN_RIGHT);
    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, MediaPropDetailColumnSet::FILEPATHCOLUMNSET);
    layout->AddView(label);
    layout->AddView(file_path_);
    layout->AddView(browse_button_);
    layout->AddView(file_tip);

    column_set = layout->AddColumnSet(MediaPropDetailColumnSet::CHECKBOXCOLUMNSET);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    column_set->AddColumn(views::GridLayout::TRAILING, views::GridLayout::CENTER, 0, views::GridLayout::FIXED, GetLengthByDPIScale(75), 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(1.0f, 0);

    loop_checkbox_ = new LivehimeCheckbox(res.GetLocalizedString(IDS_SRCPROP_FFMPEG_LOOP));

    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, MediaPropDetailColumnSet::CHECKBOXCOLUMNSET);
    layout->SkipColumns(1);
    layout->AddView(loop_checkbox_);
}