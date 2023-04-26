#include "bililive/bililive/ui/views/viddup/tabarea/tabarea_tab_strip_viddup_view.h"

#include "bililive/bililive/ui/bililive_commands.h"
#include "bililive/bililive/ui/views/controls/bililive_theme_common.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_menu.h"
#include "bililive/bililive/ui/views/menu/sceneitem_menu_cmd.h"

#include "ui/views/controls/menu/menu_runner.h"

#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"

#include "base/strings/utf_string_conversions.h"
#include "base/strings/stringprintf.h"

#include "obs/obs_proxy/ui_proxy/ui_proxy_impl.h"


// TabAreaTabStripView
TabAreaTabStripViddupView::TabAreaTabStripViddupView(int index, const base::string16 &name, bool vmodel/* = false*/)
    : LivehimeTopStripPosStripView(L"")
    , scene_index_(index)
    , scene_name_(name)
    , weakptr_factory_(this)
{
    base::string16 wstr = GetLocalizedString(IDS_TABAREA_SCENE);
    SetText(wstr + std::to_wstring(index + 1));
    label()->SetTextColor(SkColorSetA(clrTextALL, 0.6f * 255));
    label()->SetFont(ftTwelve);
}

TabAreaTabStripViddupView::~TabAreaTabStripViddupView()
{
}

void TabAreaTabStripViddupView::OnPaintBackground(gfx::Canvas* canvas)
{
    canvas->FillRect(GetContentsBounds(), GetColor(Theme));
    
    switch (state())
    {
    case NavigationStripView::SS_SELECTED:
    {
        canvas->FillRect(GetContentsBounds(), GetColor(Theme));
    }
    break;
    default:
        break;
    }
}

void TabAreaTabStripViddupView::OnStateChanged()
{
    switch (state())
    {
    case NavigationStripView::SS_INACTIVE:
        label_->SetTextColor(SkColorSetA(clrTextALL, 0.6f * 255));
		break;
    case NavigationStripView::SS_HOVER:
        label_->SetTextColor(clrTextALLHov);
        break;
    case NavigationStripView::SS_PRESSED:
    case NavigationStripView::SS_SELECTED:
        label_->SetTextColor(clrTextALL);
        break;
    default:
        break;
    }
}

gfx::Size TabAreaTabStripViddupView::GetPreferredSize()
{
    gfx::Size size = __super::GetPreferredSize();
    size.set_height(GetLengthByDPIScale(28));
    return visible() ? size : gfx::Size();
}

int TabAreaTabStripViddupView::GetHeightForWidth(int w)
{
    return visible() ? __super::GetHeightForWidth(w) : 0;
}