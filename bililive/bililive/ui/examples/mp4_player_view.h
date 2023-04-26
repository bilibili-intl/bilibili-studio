#pragma once

#include "ui/views/view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_mp4_player_viewer.h"

class Mp4PlayerView :
    public views::View
{
public:
    Mp4PlayerView();
    ~Mp4PlayerView();

protected:
    void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) override;


private:
    LivehimeMp4PlayerView* mp4_view_ = nullptr;

    void InitView();
};
