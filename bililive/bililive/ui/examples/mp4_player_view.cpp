#include "mp4_player_view.h"

#include "grit/theme_resources.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/grid_layout.h"


Mp4PlayerView::Mp4PlayerView()
{
}

Mp4PlayerView::~Mp4PlayerView()
{
}

void Mp4PlayerView::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details)
{
    if (details.child == this && details.is_add)
    {
        InitView();
    }
}

void Mp4PlayerView::InitView()
{
    auto& rb = ui::ResourceBundle::GetSharedInstance();

    mp4_view_ = new LivehimeMp4PlayerView();
    auto gird_layout = new views::GridLayout(this);
    this->SetLayoutManager(gird_layout);
    auto col = gird_layout->AddColumnSet(0);
    col->AddColumn(views::GridLayout::CENTER, views::GridLayout::CENTER, 1.0f, views::GridLayout::USE_PREF, 0, 0);

    gird_layout->StartRow(0, 0);
    gird_layout->AddView(mp4_view_);

    auto string_piece = rb.GetRawDataResource(IDR_LIVEMAIN_MP4_VIDEO_COUNTDOWN).as_string();
    //mp4_view_->SetMap4FilePath(L"G:/123.mp4");
    mp4_view_->SetMp4Buffer(string_piece.data(), string_piece.length());
    mp4_view_->Play();
}
