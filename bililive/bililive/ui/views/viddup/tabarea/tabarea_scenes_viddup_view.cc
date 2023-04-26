#include "bililive/bililive/ui/views/viddup/tabarea/tabarea_scenes_viddup_view.h"

#include "SkRRect.h"

#include "bililive/bililive/command_updater_delegate.h"
#include "bililive/bililive/livehime/common_pref/common_pref_names.h"
#include "bililive/bililive/livehime/event_report/event_report_center.h"
#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/profiles/profile.h"
#include "bililive/bililive/ui/bililive_commands.h"
#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/bililive/ui/views/controls/bililive_theme_common.h"
#include "bililive/bililive/ui/views/controls/util/bililive_util_views.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/bililive/ui/views/viddup/tabarea/tabarea_scene_content_viddup_view.h"
#include "bililive/bililive/ui/views/tabarea/tabarea_materials_widget.h"
#include "bililive/bililive/ui/views/tabarea/livehime_module_view.h"
#include "bililive/bililive/ui/views/controls/bililive_labelbutton.h"
#include "bililive/bililive/ui/views/hotkey_notify/bililive_toast_notify_view.h"
#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/secret/public/event_tracking_service.h"

#include "ui/gfx/screen.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/grid_layout.h"

#include "base/ext/callable_callback.h"
#include "base/notification/notification_observer.h"
#include "base/prefs/pref_service.h"
#include "base/strings/utf_string_conversions.h"

#include "obs/obs_proxy/public/common/pref_constants.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_bubble.h"
#include "base/notification/notification_observer.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "bililive/public/common/pref_names.h"

namespace
{
    const int kRefreshBannerTimeDeltaInMin = 5;// 5min
    const int kTabareaSeparatorLineThickness = GetLengthByDPIScale(2);

    int GetSeparatorLineThickness()
    {
        static int img_cy = GetImageSkiaNamed(IDR_LIVEHIME_DANMAKUHIME_DRAG_HANDLE)->height();
        return img_cy;
    }

    enum ToolCtrlID
    {
        Button_Add,
        Button_Clear,
        Button_Plugin
    };

    class ControlButtonView : public views::View
    {
       public:
           ControlButtonView(views::ButtonListener* listen,bool plugin)
               : add_btn_(nullptr),
               plugin_open_(plugin),
               clear_btn_(nullptr)
           {
               set_background(views::Background::CreateSolidBackground(GetColor(WindowTitle)));

               views::GridLayout* button_layout = new views::GridLayout(this);
               SetLayoutManager(button_layout);
               views::ColumnSet* button_column_set = button_layout->AddColumnSet(0);

               if (!plugin_open_) {
                   button_column_set->AddPaddingColumn(0, GetLengthByDPIScale(17));
                   button_column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
                   button_column_set->AddPaddingColumn(1.0f, 0);
                   button_column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
                   button_column_set->AddPaddingColumn(0, GetLengthByDPIScale(18));

                   add_btn_ = new MaterialControlButton(listen, MaterialControlButton::BtType_Add);
                   add_btn_->set_id(Button_Add);
                   clear_btn_ = new MaterialControlButton(listen, MaterialControlButton::BtType_Clear);
                   clear_btn_->set_id(Button_Clear);
                   button_layout->AddPaddingRow(0, GetLengthByDPIScale(6));
                   button_layout->StartRow(0, 0);
                   button_layout->AddView(add_btn_);
                   button_layout->AddView(clear_btn_);
                   button_layout->AddPaddingRow(0, GetLengthByDPIScale(6));
               }
               else {
                   button_column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
                   button_column_set->AddColumn(views::GridLayout::CENTER, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
                   button_column_set->AddColumn(views::GridLayout::CENTER, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);

                   add_btn_ = new MaterialControlButton(listen, MaterialControlButton::BtType_Add);
                   add_btn_->set_id(Button_Add);
                   clear_btn_ = new MaterialControlButton(listen, MaterialControlButton::BtType_Clear);
                   clear_btn_->set_id(Button_Clear);
                   plugin_btn_ = new MaterialControlButton(listen, MaterialControlButton::BtType_Plugin);
                   plugin_btn_->set_id(Button_Plugin);
                   button_layout->AddPaddingRow(0, GetLengthByDPIScale(6));
                   button_layout->StartRow(0, 0);
                   button_layout->AddView(add_btn_);
                   button_layout->AddView(plugin_btn_);
                   button_layout->AddView(clear_btn_);
                   button_layout->AddPaddingRow(0, GetLengthByDPIScale(6));

                   PrefService* pref = GetBililiveProcess()->profile()->GetPrefs();
                   if (pref->GetBoolean(prefs::kBililivePluginClick)) {
                       show_red_ = true;
                   }

                   livehime::PolarisEventReport(
                       secret::LivehimePolarisBehaviorEvent::PluginEntranceShow, "");
               }
           }

           views::View *GetAddButton()
           {
               return add_btn_;
           }

           void SetRedFlag(bool red) {
               show_red_ = red;
           }

           void SetLayout() {
           }

           void OnPaint(gfx::Canvas* canvas) override
           {
               __super::OnPaint(canvas);
                gfx::Rect rect = this->bounds();

                if (!plugin_open_) {
                    canvas->DrawLine(gfx::Point(rect.width() * 0.51, rect.height() * 0.3),
                        gfx::Point(rect.width() * 0.51, rect.height() * 0.7), SkColorSetARGB(0.2f * 255, 83, 103, 119));
                }
                else {
                    canvas->DrawLine(gfx::Point(rect.width() * 0.33, rect.height() * 0.3),
                        gfx::Point(rect.width() * 0.33, rect.height() * 0.7), SkColorSetARGB(0.2f * 255, 83, 103, 119));

                    canvas->DrawLine(gfx::Point(rect.width() * 0.66, rect.height() * 0.3),
                        gfx::Point(rect.width() * 0.66, rect.height() * 0.7), SkColorSetARGB(0.2f * 255, 83, 103, 119));

                    if (show_red_) {
                        static const int kRadius = GetLengthByDPIScale(5);
                        static const int kPadding = GetLengthByDPIScale(2);
                        gfx::Point point(rect.width() * 0.64, rect.height() * 0.3);

                        SkPaint paint;
                        paint.setAntiAlias(true);
                        paint.setColor(SK_ColorRED);
                        canvas->DrawCircle(point, kRadius - kPadding, paint);
                    }
                }
           }

     private:
         MaterialControlButton* add_btn_ = nullptr;
         MaterialControlButton* clear_btn_ = nullptr;
         MaterialControlButton* plugin_btn_ = nullptr;
         views::BoxLayout* box_layout_ = nullptr;
         bool show_red_ = false;
         bool plugin_open_ = false;
    };

	class AddBtnBubbleView
		: public views::View
	{
	public:
		static void Show(views::View* anchor) {
			Close();
      
      PrefService* pref = GetBililiveProcess()->global_profile()->GetPrefs();
      bool showed = false;

			if (pref) {
          showed = pref->GetBoolean(prefs::kAddBtnBubbleShowed);
          if (!showed) {
              if (!add_btn_bubble_view_) {
                  add_btn_bubble_view_ = new AddBtnBubbleView();
              }
                    
              auto add_btn_bubble = livehime::ShowBubble(
                  anchor, views::BubbleBorder::BOTTOM_LEFT, add_btn_bubble_view_);
              if (add_btn_bubble && add_btn_bubble_view_) {
                  add_btn_bubble->StartFade(false, 5000, 500);
                  add_btn_bubble->set_background_color(GetColor(Theme));
                  add_btn_bubble->set_close_on_deactivate(false);
              }
              
              pref->SetBoolean(prefs::kAddBtnBubbleShowed, true);
          }
			}
		}

		static void Close() {
        if (add_btn_bubble_view_) {
            if (add_btn_bubble_view_->GetWidget()) {
                add_btn_bubble_view_->GetWidget()->Close();
            }
            
            add_btn_bubble_view_ = nullptr;
        }
		}
    
    AddBtnBubbleView() {
        auto layout = new views::GridLayout(this);
        SetLayoutManager(layout);
        
        auto col = layout->AddColumnSet(0);
        col->AddColumn(views::GridLayout::CENTER, views::GridLayout::CENTER, 1.0f, views::GridLayout::USE_PREF, 0, 0);
        
        layout->StartRow(0, 0);
        auto label = new LivehimeContentLabel(L"New wired projection screen, more stable projection screen£¬\r\n Come and try it");
        label->SetTextColor(GetColor(WindowTitleText));
        layout->AddView(label);
		}

		virtual void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override
		{
        if (details.child == this)
        {
            if (details.is_add)
            {
            }
            else
            {
            }
			}
		}

		~AddBtnBubbleView() {
        add_btn_bubble_view_ = nullptr;
		}

	private:
		static AddBtnBubbleView* add_btn_bubble_view_;
	};

	// static
  AddBtnBubbleView* AddBtnBubbleView::add_btn_bubble_view_ = nullptr;
}

// TabAreaScenesView
TabAreaScenesViddupView::TabAreaScenesViddupView()
    : current_show_scene_index_(UISI_NOTHING)
    , strip_views_(UISI_COUNT)
    , content_views_(UISI_COUNT)
    , weak_ptr_factory_(this)
{
    static std::map<int, std::wstring> kSceneNameMap{
        // Landscape scene
        { UISI_LANDSCAPE_SCENE0, UTF8ToUTF16(prefs::kFirstDefaultSceneName) },
        { UISI_LANDSCAPE_SCENE1, UTF8ToUTF16(prefs::kSecondDefaultSceneName) },
        { UISI_LANDSCAPE_SCENE2, UTF8ToUTF16(prefs::kThirdDefaultSceneName) },
        // Portrait scene
        { UISI_VERTICAL_SCENE0, UTF8ToUTF16(prefs::kVerticalFirstDefaultSceneName) },
        { UISI_VERTICAL_SCENE1, UTF8ToUTF16(prefs::kVerticalSecondDefaultSceneName) },
        { UISI_VERTICAL_SCENE2, UTF8ToUTF16(prefs::kVerticalThirdDefaultSceneName) },
    };

    tabbed_pane_ = new NavigationBar(this, NavigationBarTabStripPos::NAVIGATIONBAR_TABSTRIP_TOP);
    tabbed_pane_->SetTabAreaColor(GetColor(WindowClient));
    need_schedule_tabbed_sel_change_ = false;
    for (int i = UISI_LANDSCAPE_SCENE0; i < UISI_COUNT; ++i)
    {
        TabAreaTabStripViddupView* stv = new TabAreaTabStripViddupView(i, kSceneNameMap[i], i >= UISI_VERTICAL_SCENE0);
        strip_views_[i] = stv;
        TabAreaSceneContentViddupView* scv = new TabAreaSceneContentViddupView(i, kSceneNameMap[i]);
        content_views_[i] = scv;
        tabbed_pane_->AddTab(L"", stv, scv);
    }

    need_schedule_tabbed_sel_change_ = true;
    control_view_ = new ControlButtonView(this,false);

    AddChildView(tabbed_pane_);
    AddChildView(control_view_);

    // Reads the height of the story list set by the last drag from the configuration file
    PrefService *pref = GetBililiveProcess()->profile()->GetPrefs();
    materials_view_height_ = pref->GetInteger(prefs::kLastTabAreaMaterialsHeight);
}

TabAreaScenesViddupView::~TabAreaScenesViddupView()
{
}

void TabAreaScenesViddupView::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails &details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            LivehimeLiveRoomController::GetInstance()->AddObserver(this);
            OBSProxyService::GetInstance().obs_ui_proxy()->AddObserver(this);
            LiveModelController::GetInstance()->AddObserver(this);
            AppFunctionController::GetInstance()->AddObserver(this);
        }
        else
        {
            LivehimeLiveRoomController::GetInstance()->RemoveObserver(this);
            if (LiveModelController::GetInstance())
            {
                LiveModelController::GetInstance()->RemoveObserver(this);
            }

            if (OBSProxyService::GetInstance().obs_ui_proxy())
            {
                OBSProxyService::GetInstance().obs_ui_proxy()->RemoveObserver(this);
            }

            AppFunctionController::GetInstance()->RemoveObserver(this);
            AddBtnBubbleView::Close();
        }
    }
}

void TabAreaScenesViddupView::OnPaintBackground(gfx::Canvas* canvas)
{
}

void TabAreaScenesViddupView::Layout()
{
    gfx::Rect rect(GetContentsBounds());

    if (is_initing_)
    {
        if (!rect.IsEmpty())
        {
            is_initing_ = false;

            materials_view_height_ = CheckMaterialAreaValidHeight(materials_view_height_);
            materials_view_height_ = std::min(materials_view_height_, DetermineMaterialAreaMaxHeight());
        }
    }

    int area_bottom = rect.bottom();
    int scene_bottom = /*livehime_module_view_->bounds().y()*/area_bottom - kTabareaSeparatorLineThickness;
    auto it = control_view_->GetPreferredSize();
    if(control_view_->visible()){
        control_view_->SetBounds(rect.x(), scene_bottom - it.height(), rect.width(), it.height());
    }

    //scene_bottom -= it.height();
    tabbed_pane_->SetBounds(
        rect.x(), rect.y(), rect.width(), rect.height());
}

void TabAreaScenesViddupView::DoPendingLayout()
{
    Layout();
}

bool TabAreaScenesViddupView::OnMousePressed(const ui::MouseEvent& event)
{
    gfx::Rect draw_bound = GetDragSplitBounds();
    if (draw_bound.Contains(event.location()))
    {
        start_drag_mouse_down_point_ = event.location();
        start_drag_materials_view_height_ = materials_view_height_;
        return true;
    }

    return false;
}

bool TabAreaScenesViddupView::OnMouseDragged(const ui::MouseEvent& event)
{
    gfx::Point cursor_pos = event.location();
    int offset_y = cursor_pos.y() - start_drag_mouse_down_point_.y();
    int drag_materials_view_height = start_drag_materials_view_height_ - offset_y;
    drag_materials_view_height = std::max(LivehimeModuleView::GetMinimumHeight(), drag_materials_view_height);
    int new_materials_view_height = std::min(drag_materials_view_height, DetermineMaterialAreaMaxHeight());
    if (new_materials_view_height != materials_view_height_)
    {
        PrefService *pref = GetBililiveProcess()->profile()->GetPrefs();
        pref->SetInteger(prefs::kLastTabAreaMaterialsHeight, new_materials_view_height);

        materials_view_height_ = new_materials_view_height;
        Layout();
        SchedulePaint();
    }

    return true;
}

gfx::NativeCursor TabAreaScenesViddupView::GetCursor(const ui::MouseEvent& event)
{
    if (GetDragSplitBounds().Contains(event.location())) {
        //return ::LoadCursor(nullptr, IDC_SIZENS);
    }

    return __super::GetCursor(event);
}

void TabAreaScenesViddupView::ChildPreferredSizeChanged(views::View* child)
{
    Layout();
}

gfx::Rect TabAreaScenesViddupView::GetDragSplitBounds()
{
    gfx::Rect rect = GetContentsBounds();
    int bottom = rect.bottom();
    int top = bottom - GetSeparatorLineThickness();
    return gfx::Rect(rect.x(), top, rect.width(), bottom - top);
}

int TabAreaScenesViddupView::CheckMaterialAreaValidHeight(int check_height)
{
    static int materials_min_height = LivehimeModuleView::GetMinimumHeight();
    int materials_pref_height = /*livehime_module_view_->GetPreferredSize().height()*/0;
    if (check_height <= 0)
    {
        check_height = materials_pref_height;
    }
    else
    {
        check_height = std::max(check_height, materials_min_height);
        check_height = std::min(check_height, materials_pref_height);
    }

    return check_height;
}

int TabAreaScenesViddupView::DetermineMaterialAreaMaxHeight()
{
    static const int kMinShowSceneItemCount = 2;
    int max_cy = 0;
    gfx::Rect rect(GetContentsBounds());
    // tab strip header + operate header + items
    int expected_scene_min_height = kDataTitleHeight * kMinShowSceneItemCount + kDataTitleHeight + kDataItemHeight;
    int banner_height = GetSeparatorLineThickness();
    int materials_min_height = LivehimeModuleView::GetMinimumHeight();

    int need_min_cy = expected_scene_min_height + banner_height + materials_min_height;
    if (need_min_cy > rect.height())
    {
        max_cy = materials_min_height;
    }
    else
    {
        int materials_max_height = CheckMaterialAreaValidHeight(0);
        int perfect_min_cy = expected_scene_min_height + banner_height + materials_max_height;
        if (perfect_min_cy > rect.height())
        {
            max_cy = rect.height() - (expected_scene_min_height + banner_height);
        }
        else
        {
            max_cy = materials_max_height;
        }
    }

    return max_cy;
}

void TabAreaScenesViddupView::OnCurrentSceneChanged(int scene_index)
{
    DCHECK((scene_index >= UISI_LANDSCAPE_SCENE0) && (scene_index < UISI_COUNT));

    if (scene_index != current_show_scene_index_)
    {
        if (current_show_scene_index_ != -1)
        {
            content_views_[current_show_scene_index_]->ShowOrHideCanvas(false);
        }

        current_show_scene_index_ = scene_index;

        if (strip_views_[current_show_scene_index_])
        {
            need_schedule_tabbed_sel_change_ = false;
            tabbed_pane_->SelectTab(strip_views_[current_show_scene_index_]);
            need_schedule_tabbed_sel_change_ = true;
        }

        content_views_[current_show_scene_index_]->ShowOrHideCanvas(true);
    }
}

void TabAreaScenesViddupView::NavigationBarSelectedAt(int strip_id)
{
    if (!need_schedule_tabbed_sel_change_)
    {
        return;
    }

    TabAreaTabStripViddupView* strip_view = (TabAreaTabStripViddupView*)tabbed_pane_->GetSelectedTab();

    int cmd = IDC_LIVEHIME_HOTKEY_SENCE1;
    cmd += strip_view->scene_index();
    DCHECK_LE(cmd, IDC_LIVEHIME_HOTKEY_VERTICAL_SENCE3);
    DCHECK_GE(cmd, IDC_LIVEHIME_HOTKEY_SENCE1);
    if (cmd >= IDC_LIVEHIME_HOTKEY_SENCE1 && cmd <= IDC_LIVEHIME_HOTKEY_VERTICAL_SENCE3)
    {
        bililive::ExecuteCommandWithParams(GetBililiveProcess()->bililive_obs(), cmd, EmptyCommandParams());
    }
}

gfx::Rect TabAreaScenesViddupView::GetCameraItemEditBtnBounds(const std::string& name) const
{
    gfx::Rect rect;
    if (current_show_scene_index_ >= 0 &&
        current_show_scene_index_ < (int)content_views_.size())
    {
        auto scene = content_views_[current_show_scene_index_];
        if (scene)
        {
            rect = scene->GetCameraItemEditBtnBounds(name);
        }
    }

    return rect;
}

void TabAreaScenesViddupView::ShowOperateView(const std::string& name, bool show)
{
    if (current_show_scene_index_ >= 0 &&
        current_show_scene_index_ < (int)content_views_.size())
    {
        auto scene = content_views_[current_show_scene_index_];
        if (scene)
        {
            scene->ShowOperateView(name, show);
        }
    }
}

void TabAreaScenesViddupView::OpenChatRoomView()
{
    if (current_show_scene_index_ >= 0 &&
        current_show_scene_index_ < (int)content_views_.size())
    {
        tabbed_pane_->SetEnabled(false);
        content_views_[current_show_scene_index_]->ShowChatRoomView(true);
    }
}

void TabAreaScenesViddupView::CloseChatRoomView()
{
    if (current_show_scene_index_ >= 0 &&
        current_show_scene_index_ < (int)content_views_.size())
    {
        tabbed_pane_->SetEnabled(true);
        content_views_[current_show_scene_index_]->ShowChatRoomView(false);
    }
}

// AppFunctionCtrlObserver
void TabAreaScenesViddupView::OnAllAppKVCtrlInfoCompleted()
{
}

gfx::Rect TabAreaScenesViddupView::GetMiddleAreaBounds() const
{
    int y = tabbed_pane_->bounds().y() + tabbed_pane_->strip_bounds().height();
    return gfx::Rect(0, y, width(), height() - y/* - size.height()*/);
}

void TabAreaScenesViddupView::OnEnterIntoThirdPartyStreamingMode()
{
    tabbed_pane_->SetEnabled(false);
    control_view_->SetEnabled(false);
}

void TabAreaScenesViddupView::OnLeaveThirdPartyStreamingMode()
{
    tabbed_pane_->SetEnabled(true);
    control_view_->SetEnabled(true);
}

void TabAreaScenesViddupView::OnLiveLayoutModelChanged(bool user_invoke)
{
    bool portrait = !LiveModelController::GetInstance()->IsLandscapeModel();
    bool relayout = false;
    for (int i = UISI_LANDSCAPE_SCENE0; i < UISI_COUNT; ++i)
    {
        if (strip_views_[i])
        {
            if (portrait)
            {
                strip_views_[i]->SetVisible(i >= UISI_VERTICAL_SCENE0);
            }
            else
            {
                strip_views_[i]->SetVisible(i <= UISI_LANDSCAPE_SCENE2);
            }
            if (!relayout)
            {
                relayout = true;
                strip_views_[i]->InvalidateLayout();
            }
        }
    }

    tabbed_pane_->Layout();
}

void TabAreaScenesViddupView::ButtonPressed(views::Button * sender, const ui::Event & event)
{
    if (sender->id() == Button_Add)
    {
        TabAreaMaterialsWidget::ShowWindow();

        livehime::PolarisEventReport(
            secret::LivehimePolarisBehaviorEvent::AppMaterialEntryClick, "");
    }

    if (sender->id() == Button_Clear)
    {
        content_views_[current_show_scene_index_]->RemoveAllSceneItem();
    }
}