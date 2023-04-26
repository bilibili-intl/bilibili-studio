#pragma once

#include "bililive/bililive/ui/views/controls/navigation_bar.h"

class BililiveLabel;

class LivehimeTopStripPosStripView : public NavigationStripView
{
public:
    LivehimeTopStripPosStripView(const base::string16 &text, gfx::ImageSkia* image_skia = nullptr);
    virtual ~LivehimeTopStripPosStripView();

    void SetText(const base::string16 &text);

protected:
    // Overridden from View:
    gfx::Size GetPreferredSize() override;
    int GetHeightForWidth(int w) override;
    void OnPaintBackground(gfx::Canvas* canvas) override;
    void Layout() override;

    // NavigationStripView
    void OnStateChanged() override;

    BililiveLabel *label() { return label_; }
    virtual gfx::Rect GetLineRegion();

protected:
    BililiveLabel *label_;
};


class LivehimeLeftStripPosStripView : public NavigationStripView
{
public:
    LivehimeLeftStripPosStripView(const base::string16 &text, gfx::ImageSkia* image_skia = nullptr);
    LivehimeLeftStripPosStripView(View* count_view);
    virtual ~LivehimeLeftStripPosStripView();

    void SetPreferredSize(int width,int hight);
protected:
    // Overridden from View:
    gfx::Size GetPreferredSize() override;
    int GetHeightForWidth(int w) override;
    void OnPaintBackground(gfx::Canvas* canvas) override;
    void Layout() override;

    // NavigationStripView
    void OnStateChanged() override;

protected:
    BililiveLabel *label_;
    View* count_view_ = nullptr;
    gfx::Size size_;

    string16 title_text_;
};

