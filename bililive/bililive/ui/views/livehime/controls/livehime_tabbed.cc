#include "bililive/bililive/ui/views/livehime/controls/livehime_tabbed.h"

#include "ui/views/layout/box_layout.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/secret/public/event_tracking_service.h"
#include "bililive/bililive/livehime/event_report/event_report_center.h"
#include "base/strings/utf_string_conversions.h"
#include "ui/base/resource/resource_bundle.h"
#include "grit/theme_resources.h"
#include "grit/generated_resources.h"

namespace
{
    const int kLineSize = GetLengthByDPIScale(2);
}

LivehimeTopStripPosStripView::LivehimeTopStripPosStripView(const base::string16 &text, gfx::ImageSkia* image_skia/* = nullptr*/)
{
    label_ = new LivehimeTitleLabel(text);
    label_->SetImage(image_skia);
    //label_->SetTextColor(clrTabbedTextNor);
    label_->SetTextColor(SkColorSetRGB(255, 0, 0));
    AddChildView(label_);
}

LivehimeTopStripPosStripView::~LivehimeTopStripPosStripView()
{
}

void LivehimeTopStripPosStripView::SetText(const base::string16 &text)
{
    label_->SetText(text);
    PreferredSizeChanged();
}

void LivehimeTopStripPosStripView::Layout()
{
    gfx::Size size = label_->GetPreferredSize();
    label_->SetBounds((width() - size.width()) / 2, (height() - kLineSize - size.height()) / 2, size.width(), size.height());
}

gfx::Size LivehimeTopStripPosStripView::GetPreferredSize()
{
    gfx::Size size = label_->GetPreferredSize();
    size.Enlarge(LivehimePaddingCharWidth(ftPrimary) * 2, 0);
    size.set_height(kMainWndTabAreaStripHeight);
    return size;
}

int LivehimeTopStripPosStripView::GetHeightForWidth(int w)
{
    return GetPreferredSize().height();
}

void LivehimeTopStripPosStripView::OnPaintBackground(gfx::Canvas* canvas)
{
    canvas->FillRect(GetLocalBounds(), clrWindowsContent);

    switch (state())
    {
    case NavigationStripView::SS_SELECTED:
    {
        gfx::Rect line_rect = GetLineRegion();
        canvas->FillRect(line_rect, clrLivehime);
    }
    break;
    default:
        break;
    }
}

void LivehimeTopStripPosStripView::OnStateChanged()
{
    switch (state())
    {
    case NavigationStripView::SS_INACTIVE:
    case NavigationStripView::SS_HOVER:
        label_->SetTextColor(clrTabbedTextNor);
        break;
    case NavigationStripView::SS_PRESSED:
    case NavigationStripView::SS_SELECTED:
        label_->SetTextColor(clrTabbedTextPre);
        break;
    default:
        break;
    }
}

gfx::Rect LivehimeTopStripPosStripView::GetLineRegion()
{
    static int cx = LivehimePaddingCharWidth(ftPrimary) * 6;
    gfx::Rect rect = GetLocalBounds();
    return gfx::Rect((width() - cx) / 2, height() - kLineSize, cx, kLineSize);
}


// LivehimeLeftStripPosStripView
LivehimeLeftStripPosStripView::LivehimeLeftStripPosStripView(const base::string16 &text, gfx::ImageSkia* image_skia/* = nullptr*/)
    :title_text_(text){
    size_.set_width(0);
    label_ = new LivehimeTitleLabel(text);
    label_->SetImage(image_skia);
    label_->SetTextColor(GetColor(TabbedTextNor));
    AddChildView(label_);
}

LivehimeLeftStripPosStripView::LivehimeLeftStripPosStripView(View* count_view)
{
    size_.set_width(0);
    label_ = nullptr;
    count_view_ = count_view;
    AddChildView(count_view_);
}

LivehimeLeftStripPosStripView::~LivehimeLeftStripPosStripView()
{
}

void LivehimeLeftStripPosStripView::Layout()
{
    if (!size().IsEmpty())
    {
        if (count_view_)
        {
            gfx::Size count_size = count_view_->GetPreferredSize();
            count_view_->SetBounds((width() - count_size.width())/2, (height() - count_size.height()) / 2, width(), count_size.height());
        }
        else
        {
            gfx::Size label_size = label_->GetPreferredSize();
            label_->SetBounds(0, (height() - label_size.height()) / 2, width(), label_size.height());
        }
    }
}

void LivehimeLeftStripPosStripView::SetPreferredSize(int width, int hight)
{
    size_.set_width(width);
    size_.set_height(hight);
}

gfx::Size LivehimeLeftStripPosStripView::GetPreferredSize()
{
    if (size_.width() != 0)
    {
        return size_;
    }
    else
    {
        return kLeftStripPosStripSize;
    }
}

int LivehimeLeftStripPosStripView::GetHeightForWidth(int w)
{
    return GetPreferredSize().height();
}

void LivehimeLeftStripPosStripView::OnPaintBackground(gfx::Canvas* canvas)
{
    gfx::Rect rect = GetLocalBounds();

    switch (state())
    {
    case NavigationStripView::SS_INACTIVE:
        //canvas->FillRect(rect, clrTabbedBkNor);
        // canvas->FillRect(rect, SkColorSetARGB(0, 0, 0, 0));
        canvas->FillRect(rect, GetColor(TabbedBkNor));
        break;
    case NavigationStripView::SS_HOVER:
        //canvas->FillRect(rect, clrTabbedBkHov);
        // canvas->FillRect(rect, SkColorSetARGB(0, 0, 0, 0));
        canvas->FillRect(rect, GetColor(TabbedBkHov));
        break;
    case NavigationStripView::SS_SELECTED:
    {
        // canvas->FillRect(rect, clrTabbedBkPre);
        // canvas->FillRect(rect, SkColorSetRGB(74, 75, 97));
        canvas->FillRect(rect, GetColor(TabbedBkSel));
        //canvas->FillRect(gfx::Rect(0, 2, kLineSize, height() - 4), clrLivehime);
    }
    break;
    case NavigationStripView::SS_PRESSED:
        // canvas->FillRect(rect, clrTabbedBkPre);
        // canvas->FillRect(rect, SkColorSetARGB(0, 0, 0, 0));
        canvas->FillRect(rect, GetColor(TabbedBkPre));
        break;
    default:
        break;
    }
}

void LivehimeLeftStripPosStripView::OnStateChanged()
{
    if ((state() == NavigationStripView::SS_SELECTED)) {
        livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::BeautyIconClick, UTF16ToUTF8(title_text_));
    }
    return;
    switch (state())
    {
    case NavigationStripView::SS_INACTIVE:
        label_->SetTextColor(clrTabbedTextNor);
        break;
    case NavigationStripView::SS_HOVER:
        label_->SetTextColor(clrTabbedTextHov);
        break;
    case NavigationStripView::SS_PRESSED:
    case NavigationStripView::SS_SELECTED:
        label_->SetTextColor(clrTabbedTextPre);
        break;
    default:
        break;
    }
}
