#include "base_setup_view.h"


BaseSetupView::BaseSetupView()
{

}

void BaseSetupView::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            InitView();
            InitData();
        }
        else
        {
            UninitView();
        }
    }
}


CustomLivehimeTopStripPosStripView::CustomLivehimeTopStripPosStripView(const base::string16& text, gfx::ImageSkia* image_skia) :
    LivehimeTopStripPosStripView(text, image_skia)
{

}

void CustomLivehimeTopStripPosStripView::CustomLivehimeTopStripPosStripView::OnPaintBackground(gfx::Canvas* canvas)
{
    canvas->FillRect(GetLocalBounds(), clrWindowsContent);
}

gfx::Size CustomLivehimeTopStripPosStripView::GetPreferredSize()
{
    return GetSizeByDPIScale({ 57, 20 });
}

void CustomLivehimeTopStripPosStripView::Layout()
{
    gfx::Size size = label_->GetPreferredSize();
    label_->SetBounds(0, (height() - size.height()) / 2, size.width(), size.height());
}
