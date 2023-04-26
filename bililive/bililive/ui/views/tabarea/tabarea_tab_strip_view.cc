#include "tabarea_tab_strip_view.h"

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

TabAreaTabStripView::TabAreaTabStripView(int index, const base::string16 &name, bool vmodel/* = false*/)
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

TabAreaTabStripView::~TabAreaTabStripView()
{
}

void TabAreaTabStripView::OnPaintBackground(gfx::Canvas* canvas)
{
    canvas->FillRect(GetContentsBounds(), SkColorSetRGB(0xf4, 0xf4, 0xf4));
    /*switch (state())
    {
    case NavigationStripView::SS_SELECTED:
    {
        gfx::Rect line_rect = GetLineRegion();
        line_rect.Inset(0, -1, 0, 1);
        if (draw_full_line_)
        {
            gfx::Rect bds = GetContentsBounds();
            line_rect.set_x(bds.x());
            line_rect.set_width(bds.width());
        }
        canvas->FillRect(line_rect, GetColor(WindowTitleText));
    }
    break;
    default:
        break;
    }

    if (tpt_ == SCENE)
    {
        bool is_selected = (state() == NavigationStripView::SS_SELECTED);
        gfx::ImageSkia *arrow_img = is_menu_down_ ?
            (is_selected ? arrow_up_hv_img_ : arrow_up_img_) :
            (is_selected ? arrow_down_hv_img_ : arrow_down_img_);
        if (arrow_img)
        {
            gfx::Rect arrow_rect = GetArrowRegion();
            canvas->DrawImageInt(*arrow_img,
                                 arrow_rect.x(),
                                 arrow_rect.y() + (arrow_rect.height() - arrow_img->height()) / 2,
                                 is_selected ? 255 : kInactiveAlpha);
        }
    }*/


   switch (state())
   {
   case NavigationStripView::SS_SELECTED:
   {
       canvas->FillRect(GetContentsBounds(), SkColorSetRGB(0xf9, 0xf9, 0xf9));
   }
   break;
   default:
       break;
   }
}

void TabAreaTabStripView::OnStateChanged()
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

gfx::Size TabAreaTabStripView::GetPreferredSize()
{
    gfx::Size size = __super::GetPreferredSize();
    size.set_height(GetLengthByDPIScale(28));
    return visible() ? size : gfx::Size();
}

int TabAreaTabStripView::GetHeightForWidth(int w)
{
    return visible() ? __super::GetHeightForWidth(w) : 0;
}

//bool TabAreaTabStripView::IsItemChecked(int command_id) const
//{
//    return (command_id == current_menu_command_id_);
//}
//
//void TabAreaTabStripView::ExecuteCommand(int command_id)
//{
//    int cmd = COMMAND_NOTHING;
//    switch (command_id)
//    {
//    case COMMAND_CHECK_SCENE0:{
//        cmd = IDC_LIVEHIME_HOTKEY_SENCE1;
//        break; }
//    case COMMAND_CHECK_SCENE1: {
//        cmd = IDC_LIVEHIME_HOTKEY_SENCE2;
//        break; }
//    case COMMAND_CHECK_SCENE2: {
//        cmd = IDC_LIVEHIME_HOTKEY_SENCE3;
//        break; }
//    default:
//        break;
//    }
//    if (cmd != COMMAND_NOTHING)
//    {
//        bililive::ExecuteCommandWithParams(GetBililiveProcess()->bililive_obs(), cmd, EmptyCommandParams());
//    }
//}
//
//void TabAreaTabStripView::SetMenu(int menu_index)
//{
//    ResourceBundle &rb = ResourceBundle::GetSharedInstance();
//    base::string16 wstr = rb.GetLocalizedString(IDS_TABAREA_SCENE);
//
//    switch (menu_index)
//    {
//    case COMMAND_CHECK_SCENE0: {
//        SetText(wstr + L"1");
//        break; }
//    case COMMAND_CHECK_SCENE1: {
//        SetText(wstr + L"2");
//        break; }
//    case COMMAND_CHECK_SCENE2: {
//        SetText(wstr + L"3");
//        break; }
//    default: {
//        break; }
//    }
//}
//
//void TabAreaTabStripView::RunMenu()
//{
//    if (tpt_ == SCENE)
//    {
//        ResourceBundle &rb = ResourceBundle::GetSharedInstance();
//        gfx::Rect rt = GetBoundsInScreen();
//        gfx::Rect line_rt = GetLineRegion();
//        gfx::Point line_lb = line_rt.bottom_left();
//        ConvertPointToScreen(this, &line_lb);
//        LivehimeMenuItemView *mv = new LivehimeMenuItemView(this);
//        mv->set_maximize_preferred_width(line_rt.width());// 使菜单宽度与标签头宽度一致，好看
//        base::string16 wstr = rb.GetLocalizedString(IDS_TABAREA_SCENE);
//        mv->AppendMenuItem(COMMAND_CHECK_SCENE0, wstr + L"1", views::MenuItemView::CHECKBOX);
//        mv->AppendMenuItem(COMMAND_CHECK_SCENE1, wstr + L"2", views::MenuItemView::CHECKBOX);
//        mv->AppendMenuItem(COMMAND_CHECK_SCENE2, wstr + L"3", views::MenuItemView::CHECKBOX);
//
//        SetEnabled(false);
//        is_menu_down_ = true;
//        SchedulePaint();
//
//        views::MenuRunner runner(mv);
//        runner.RunMenuAt(GetWidget()->GetTopLevelWidget(), NULL, gfx::Rect(line_lb.x(), rt.bottom(), 0, 0),
//            views::MenuItemView::TOPLEFT, ui::MENU_SOURCE_MOUSE, views::MenuRunner::HAS_MNEMONICS);
//
//        is_menu_down_ = false;
//        SchedulePaint();
//
//        base::MessageLoop::current()->PostTask(FROM_HERE,
//            base::Bind(&TabAreaTabStripView::SetEnabled, weakptr_factory_.GetWeakPtr(), true));
//    }
//}

/*void TabAreaTabStripView::OnCurrentSceneChanged(int scene_index)
{
    if (current_scene_index_ != scene_index)
    {
        current_scene_index_ = scene_index;

        delegate_->SetSceneContentView(scene_index);

        GetBililiveProcess()->secret_core()->event_tracking_service()->ReportLivehimeBehaviorEvent(
            secret::LivehimeBehaviorEvent::LivehimeSceneChange,
            GetBililiveProcess()->secret_core()->account_info().mid(),
            std::string()).Call();
    }
}*/
