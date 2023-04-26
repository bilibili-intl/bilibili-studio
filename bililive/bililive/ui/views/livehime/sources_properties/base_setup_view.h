#pragma once

#include "bililive/bililive/ui/views/controls/bililive_floating_scroll_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_tabbed.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_property_base_view.h"


class BaseSetupView : 
    public livehime::BasePropertyData, 
    public BililiveViewWithFloatingScrollbar
{
public:
    BaseSetupView();
    virtual ~BaseSetupView() = default;

    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;
};


class CustomLivehimeTopStripPosStripView : public LivehimeTopStripPosStripView {
public:
    CustomLivehimeTopStripPosStripView(const base::string16& text, gfx::ImageSkia* image_skia = nullptr);

    void OnPaintBackground(gfx::Canvas* canvas) override;
    gfx::Size GetPreferredSize() override;
    void Layout() override;
};

