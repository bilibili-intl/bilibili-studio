#include "bililive/bililive/ui/views/viddup/titlebar/titlebar_viddup_view.h"

#include "base/ext/callable_callback.h"
#include "base/prefs/pref_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/controls/button/image_button.h"
#include "ui/views/controls/menu/menu_runner.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/animation/bounds_animator.h"
#include "ui/views/animation/bounds_animator_observer.h"

#include "bililive/bililive/livehime/common_pref/common_pref_names.h"
#include "bililive/bililive/livehime/help_center/help_center.h"
#include "bililive/bililive/livehime/user_info/user_info_service.h"
#include "bililive/bililive/livehime/function_control/app_function_controller.h"
#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/livehime/streaming_report/streaming_report_service.h"
#include "bililive/bililive/ui/bililive_commands.h"
#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/hotkey_notify/bililive_hotkey_notify_view.h"
#include "bililive/bililive/ui/views/livehime/anchor_notice/anchor_notice_box_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_menu.h"
#include "bililive/bililive/ui/views/livehime/settings/settings_frame_view.h"
#include "bililive/bililive/ui/views/livehime/web_browser/livehime_web_browser_datatype.h"
#include "bililive/bililive/ui/views/viddup/main_view/livehime_main_viddup_view.h"
#include "bililive/bililive/ui/views/preview/livehime_preview_view.h"
#include "bililive/bililive/ui/views/tabarea/bottom_bar/bottom_area_popup_view.h"
#include "bililive/bililive/utils/bililive_canvas_drawer.h"
#include "bililive/bililive/utils/bililive_image_util.h"
#include "bililive/bililive/utils/net_util.h"
#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/bililive/utils/fast_forward_url_convert.h"

namespace {

    const int kHeadshotAndArrowThickness = 4;
    const int kAvatarImageInsets = GetLengthByDPIScale(1);

    const SkColor clrTitleBtnHoverBk = SkColorSetRGB(0x7f, 0xdd, 0xff);
	  const int kRefreshNewsTimeDeltaInMin = 30; // 30min
	  const int kAnimationTimeDelta = 5;

    enum TitleBarCtrlId
    {
        Button_Menu,
        Button_Anchor,
        Button_Setup,
        Button_Share,
        Button_Min,
        Button_Max,
        Button_Close,
    };

    enum GuideStatus
    {
        ClickToTurnOn,
        ClickToTurnOff,
    };

    // Display information
    struct MonitorInfo
    {
        MonitorInfo()
            : x_(0), y_(0), cx_(0), cy_(0) {
        }

        MonitorInfo(int32_t x, int32_t y, uint32_t cx, uint32_t cy)
            :x_(x), y_(y), cx_(cx), cy_(cy)
        {
        }

        int32_t x_, y_;
        uint32_t cx_, cy_;
    };

    // Display enumeration
    BOOL CALLBACK MonitorEnumCallbackProc(HMONITOR, HDC, LPRECT rect, LPARAM param)
    {
        std::vector<MonitorInfo>& monitors = *reinterpret_cast<std::vector<MonitorInfo>*>(param);

        monitors.emplace_back(
            rect->left, rect->top,
            rect->right - rect->left, rect->bottom - rect->top);

        return true;
    }

    void EventTracking(secret::LivehimeBehaviorEvent event_id) {
        auto secret_core = GetBililiveProcess()->secret_core();
        secret_core->event_tracking_service()->ReportLivehimeBehaviorEvent(
            event_id, secret_core->account_info().mid(), std::string()).Call();
    }

    // Head frame
    class HeadshotButton
        : public views::CustomButton
        , public base::NotificationObserver
    {
    public:
        HeadshotButton()
            : views::CustomButton(nullptr)
        {
            set_notify_enter_exit_on_child(true);

            image_view_ = new views::ImageView();
            image_view_->set_interactive(false);
            AddChildView(image_view_);

            //static gfx::ImageSkia *cls = GetImageSkiaNamed(IDR_LIVEHIME_TITLEBAR_SHUT_DOWN_HV);
            img_cy_ = GetLengthByImgDPIScale(22);//cls->height() - GetLengthByImgDPIScale(12);

            down_arrow_image_ = GetImageSkiaNamed(IDR_LIVEHIME_V3_HOSTCENTER_HEADSHOT_DOWN);
            up_arrow_image_ = GetImageSkiaNamed(IDR_LIVEHIME_V3_HOSTCENTER_HEADSHOT_UP);
            arrow_cx_ = std::max(down_arrow_image_->width(), up_arrow_image_->width());

            int img_size = img_cy_ - kAvatarImageInsets * 2;
            image_view_->SetImageSize(gfx::Size(img_size, img_size));
        }

        void UpdateHeadshot(const gfx::ImageSkia& img)
        {
            int img_size = img_cy_ - kAvatarImageInsets * 2;
            auto tmp = bililive::CreateCircleImageEx(img, gfx::Size(img_size, img_size));
            image_view_->SetImage(&tmp);
            SchedulePaint();
        }

    protected:
        // CustomButton
        void StateChanged() override
        {
            views::Button::ButtonState bs = state();
            if (bs != Button::STATE_NORMAL)
            {
                TitleBarUserInfoViddupView::ShowForm(image_view_, this);
            }
        }

        // View
        void ViewHierarchyChanged(const ViewHierarchyChangedDetails &details) override
        {
            if (details.child == this)
            {
                if (details.is_add)
                {
                    notifation_register_.Add(this,
                        bililive::NOTIFICATION_LIVEHIME_USERINFO_VIEW_SHOW,
                        base::NotificationService::AllSources());
                    notifation_register_.Add(this,
                        bililive::NOTIFICATION_LIVEHIME_USERINFO_VIEW_CLOSE,
                        base::NotificationService::AllSources());
                }
                else
                {
                    notifation_register_.RemoveAll();
                }
            }
        }

        gfx::Size GetPreferredSize() override
        {
            return gfx::Size(img_cy_ + GetLengthByImgDPIScale(kHeadshotAndArrowThickness) + arrow_cx_, img_cy_);
        }

        View* GetEventHandlerForPoint(const gfx::Point& point) override
        {
            return this;
        }

        void Layout() override
        {
            int img_size = img_cy_ - kAvatarImageInsets * 2;
            image_view_->SetBounds(
                (width() - GetLengthByImgDPIScale(kHeadshotAndArrowThickness) - arrow_cx_ - img_size) / 2,
                (height() - img_size) / 2, img_size, img_size);
        }

        void OnPaintBackground(gfx::Canvas* canvas) override
        {
            __super::OnPaintBackground(canvas);

            int circle_size = img_cy_;
            SkPaint paint;
            paint.setAntiAlias(true);
            paint.setColor(SkColorSetA(SK_ColorWHITE, 0.5f * 256));
            canvas->DrawCircle(gfx::Point(circle_size /2, circle_size/ 2), circle_size / 2, paint);

            gfx::ImageSkia* arrow_img = hover_widget_show_now_ ? up_arrow_image_ : down_arrow_image_;
            if (arrow_img)
            {
                canvas->DrawImageInt(
                    *arrow_img,
                    img_cy_ + GetLengthByImgDPIScale(kHeadshotAndArrowThickness), (height() - arrow_img->height()) / 2);
            }
        }

        void Paint(gfx::Canvas* canvas) override
        {
            __super::Paint(canvas);

            if (GetBililiveProcess()->secret_core()->anchor_info().get_captain_warn_status())
            {
                if (this->visible())
                {
                    auto bound = bounds();
                    int circle_size = img_cy_ / 3;
                    SkPaint paint;
                    paint.setAntiAlias(true);
                    paint.setColor(SkColorSetA(SK_ColorRED, 255));
                    canvas->DrawCircle(gfx::Point(bound.right() - circle_size, bound.y() + (circle_size / 2)), circle_size / 2, paint);
                }
            }
        }

        // NotificationObserver
        void Observe(int type,
                     const base::NotificationSource& source,
                     const base::NotificationDetails& details) override
        {
            switch (type)
            {
            case bililive::NOTIFICATION_LIVEHIME_USERINFO_VIEW_SHOW:
            case bililive::NOTIFICATION_LIVEHIME_USERINFO_VIEW_CLOSE:
                hover_widget_show_now_ = (type == bililive::NOTIFICATION_LIVEHIME_USERINFO_VIEW_SHOW);
                SchedulePaint();
                break;
            default:
                break;
            }
        }

    private:
        views::ImageView *image_view_ = nullptr;
        gfx::ImageSkia* down_arrow_image_ = nullptr;
        gfx::ImageSkia* up_arrow_image_ = nullptr;
        int img_cy_ = 0;
        int arrow_cx_ = 0;
        bool hover_widget_show_now_ = false;

        base::NotificationRegistrar notifation_register_;
    };

    // Title bar button, we cover a layer of hover mask, design too lazy to give hover state diagram
    class TitlebarImageButton : public BililiveImageButton
    {
    public:
        explicit TitlebarImageButton(views::ButtonListener* listener)
            : BililiveImageButton(listener)
        {
        }

    protected:
        // View
        void OnPaintBackground(gfx::Canvas* canvas) override
        {
            __super::OnPaintBackground(canvas);

            if (state() != views::Button::STATE_NORMAL)
            {
                canvas->FillRect(GetLocalBounds(), SkColorSetA(clrTitleBtnHoverBk, kHoverMaskAlpha));
            }
        }
    };

    // Anchor message button, support red dot
    class AnchorNoticeButton : public TitlebarImageButton
    {
    public:
        explicit AnchorNoticeButton(views::ButtonListener* listener)
            : TitlebarImageButton(listener)
        {
        }

        void ShowRedpoint(int unread_count)
        {
            last_unread_count_ = unread_count;
            draw_redpoint_ = last_unread_count_ > 0;
            if (AnchorNoticeBoxView::IsShowing())
            {
                draw_redpoint_ = false;
            }

            SchedulePaint();
        }

        int last_unread_count() const
        {
            return last_unread_count_;
        }

    protected:
        // View
        void OnPaint(gfx::Canvas* canvas) override
        {
            __super::OnPaint(canvas);

            if (draw_redpoint_)
            {
                gfx::Rect rect = GetContentsBounds();
                gfx::Point pt(
                    rect.CenterPoint().x() + GetLengthByImgDPIScale(8),
                    rect.CenterPoint().y() - GetLengthByImgDPIScale(8));
                SkPaint paint;
                paint.setAntiAlias(true);
                paint.setColor(SK_ColorRED);
                canvas->DrawCircle(pt, GetLengthByImgDPIScale(4), paint);
            }
        }

    private:
        bool draw_redpoint_ = false;
        int last_unread_count_ = 0;
    };

    // The More menu
    class TitleBarMoreMenu : public LivehimeMenuItemView
    {
    public:
        enum CommandID
        {
            COMMAND_NOTHING = -1,
            COMMAND_PREVIEW_PROJECTOR,
            COMMAND_HELP,
            COMMAND_DETECT,

            COMMAND_LOGFILE,
            SUB_COMMAND_LOGFILE_DISK,
            SUB_COMMAND_LOGFILE_UPLOAD,

            COMMAND_SERVICE,
            COMMAND_USER_AGREEMENT,
            COMMAND_PRIVACY_AGREEMENT,

#ifdef _DEBUG
            COMMAND_WEBTEST,
#endif
            COMMAND_SETTING,

            COMMAND_PROJECTOR_BEGIN = 50,
            COMMAND_PROJECTOR_END = 100,
        };

        explicit TitleBarMoreMenu(views::MenuDelegate* delegate)
            : LivehimeMenuItemView(delegate)
        {
            ResourceBundle &rb = ResourceBundle::GetSharedInstance();

            auto projector_menu = static_cast<LivehimeMenuItemView*>(AppendMenuItem(
                COMMAND_PREVIEW_PROJECTOR, rb.GetLocalizedString(IDS_TITLEBAR_MORE_PREVIEWPROJECT)));

            monitors_.clear();
            //auto add_item_to_menumodel_fn = [this, &rb, projector_menu]()
            {
                EnumDisplayMonitors(NULL, NULL, MonitorEnumCallbackProc, (LPARAM)&monitors_);

                int monitor_index = 0;
                for (MonitorInfo& monitor_info : monitors_)
                {
                    base::string16 monitor_name =
                        base::StringPrintf(L"%ls: %d, %ld*%ld, %d,%d",
                        rb.GetLocalizedString(IDS_TITLEBAR_MORE_PROJECT_MONITOR).c_str(), monitor_index,
                        monitor_info.cx_, monitor_info.cy_,
                        monitor_info.x_, monitor_info.y_);
                    projector_menu->AppendMenuItem(monitor_index + COMMAND_PROJECTOR_BEGIN, monitor_name);
                    monitor_index++;
                }
            };

            //add_item_to_menumodel_fn();
            AppendMenuItem(COMMAND_SETTING, rb.GetLocalizedString(IDS_DANMAKUHIME_TOOLTIP_SETTING));

            auto logfile_menu = static_cast<LivehimeMenuItemView*>(AppendMenuItem(
                COMMAND_LOGFILE, rb.GetLocalizedString(IDS_TITLEBAR_MORE_LOG_FILE)));
            logfile_menu->AppendMenuItem(SUB_COMMAND_LOGFILE_DISK, rb.GetLocalizedString(IDS_TITLEBAR_MORE_LOG_SUB_MENU_FILE_LOCATE));

            AppendMenuItem(COMMAND_USER_AGREEMENT, rb.GetLocalizedString(IDS_TITLEBAR_MORE_USER_AGREEMENT));
            AppendMenuItem(COMMAND_PRIVACY_AGREEMENT, rb.GetLocalizedString(IDS_TITLEBAR_MORE_PRIVACY));

        }

        static std::vector<MonitorInfo> monitors_;

    private:
        DISALLOW_COPY_AND_ASSIGN(TitleBarMoreMenu);
    };

    std::vector<MonitorInfo> TitleBarMoreMenu::monitors_;

    class ButtonArea : public views::View
    {
    public:
        gfx::Size GetPreferredSize() override
        {
            gfx::Size size = View::GetPreferredSize();
            size.set_width(0);
            for (int i = 0; i < child_count(); ++i)
            {
                auto view = child_at(i);
                if (view->visible())
                {
                    auto child_size = view->GetPreferredSize();
                    size.Enlarge(child_size.width(), 0);
                    size.set_height(child_size.height());
                }
            }

            return size;
        }

    protected:
        // View
        void Layout() override
        {
            int current_px_pos = 0;
            for (int i = 0; i < child_count(); ++i)
            {
                if (child_at(i)->visible()) {
                    auto size = child_at(i)->GetPreferredSize();
                    child_at(i)->SetBounds(current_px_pos, 0, size.width(), size.height());

                    current_px_pos += size.width();
                }
            }
        }

        void ChildPreferredSizeChanged(views::View* child) override
        {
            PreferredSizeChanged();
        }
    };

class AnimateNoticeView
		: public views::View
		, public views::ButtonListener
		, public views::BoundsAnimatorObserver
	{
	public:
		AnimateNoticeView()
			: animator_(this)
			, weakptr_factory_(this)
		{
		}

	protected:
		void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override
		{
			if (details.child == this)
			{
				if (details.is_add)
				{
					notice_list_view_ = new views::View();
					notice_list_view_->set_notify_enter_exit_on_child(true);
					AddChildView(notice_list_view_);

					set_notify_enter_exit_on_child(true);

					animator_.AddObserver(this);

					RefreshNotices();
				}
				else
				{
					animator_.RemoveObserver(this);
				}
			}
		}

		gfx::Size GetPreferredSize() override
		{
			return gfx::Size(GetLengthByDPIScale(240)/*notice_list_view_->GetPreferredSize().width()*/, ftPrimary.GetHeight());
		}

		// ButtonListener
		void ButtonPressed(views::Button* sender, const ui::Event& event) override
		{
			
		}

		// BoundsAnimatorObserver
		void OnBoundsAnimatorProgressed(views::BoundsAnimator* animator) override
		{}

		void OnBoundsAnimatorDone(views::BoundsAnimator* animator) override
		{
			if (notice_list_view_->y() <=
				-notice_list_view_->height() + notice_list_view_->height() / notice_list_view_->child_count())
			{
				notice_list_view_->SetBounds(0, 0, width(), notice_list_view_->height());
			}
		}

		View* GetEventHandlerForPoint(const gfx::Point& point) override
		{
        View* view = __super::GetEventHandlerForPoint(point);
        if (view == this || view == notice_list_view_)
        {
            return parent();
        }
        
        return view;
		}

	private:
		void RefreshNotices()
		{

		}

		void ResetNoticeListView()
		{
			notice_list_view_->RemoveAllChildViews(true);
			notice_list_view_->SetLayoutManager(nullptr);

			views::GridLayout* layout = new views::GridLayout(notice_list_view_);
			views::ColumnSet* columnset = layout->AddColumnSet(0);
			columnset->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);
			notice_list_view_->SetLayoutManager(layout);
		}

		void AddNoticeItemView(views::View* view)
		{
			views::GridLayout* layout = (views::GridLayout*)notice_list_view_->GetLayoutManager();
			if (layout)
			{
				layout->StartRow(0, 0);
				layout->AddView(view);
			}
		}

		void AnimationCountdownTimerMethod()
		{
			if (!notice_list_view_->visible() || (notice_list_view_->child_count() < 2))
			{
				return;
			}

			if (IsMouseHovered()) {
				return;
			}

			current_frame_holder_time_count_++;

			if (current_frame_holder_time_count_ < kAnimationTimeDelta) {
				return;
			}
			else {
				current_frame_holder_time_count_ = 0;
			}

			int dst_y = notice_list_view_->y() - notice_list_view_->height() / notice_list_view_->child_count();
			DCHECK(std::abs(dst_y) <= notice_list_view_->height());
			animator_.AnimateViewTo(notice_list_view_, gfx::Rect(0, dst_y, width(), notice_list_view_->height()));
		}

	private:
		int current_frame_holder_time_count_ = 0;
		views::View* notice_list_view_ = nullptr;
		views::BoundsAnimator animator_;
		base::RepeatingTimer<AnimateNoticeView> animation_countdown_timer_;
		base::WeakPtrFactory<AnimateNoticeView> weakptr_factory_;
	};
}   // namespace

TitleBarViddupView::TitleBarViddupView()
    : weakptr_factory_(this)
{
}

TitleBarViddupView::~TitleBarViddupView()
{
}

void TitleBarViddupView::ViewHierarchyChanged(const ViewHierarchyChangedDetails &details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            InitViews();

            notifation_register_.Add(this,
                bililive::NOTIFICATION_LIVEHIME_LOAD_ROOMINFO_SUCCESS,
                base::NotificationService::AllSources());
            notifation_register_.Add(this,
                bililive::NOTIFICATION_LIVEHIME_UPDATE_AVATAR,
                base::NotificationService::AllSources());

            DCHECK(GetWidget() != nullptr);
            GetWidget()->AddObserver(this);

            profile_pref_registrar_.Init(GetBililiveProcess()->profile()->GetPrefs());
            profile_pref_registrar_.Add(prefs::kLivehimeGlobalSettingShow,
                base::Bind(&TitleBarViddupView::OnGlobalSettingRedPointShowChanged, weakptr_factory_.GetWeakPtr()));
            profile_pref_registrar_.Add(prefs::kLivehimeDanmuSettingShow,
                base::Bind(&TitleBarViddupView::OnGlobalSettingRedPointShowChanged, weakptr_factory_.GetWeakPtr()));
        }
        else
        {
            notifation_register_.RemoveAll();
            GetWidget()->RemoveObserver(this);
        }
    }
}

bool TitleBarViddupView::OnMousePressed(const ui::MouseEvent& event)
{
    GetFocusManager()->ClearFocus();
    return __super::OnMousePressed(event);
}

void TitleBarViddupView::InitViews()
{
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();

    views::GridLayout *layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    views::ColumnSet *column_set = layout->AddColumnSet(0);
    column_set->AddPaddingColumn(0, GetLengthByDPIScale(24));
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);

    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(1.0, GetLengthByDPIScale(15));

    //column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    //column_set->AddPaddingColumn(0, GetLengthByDPIScale(15));

    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForCtrlTips);

    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, GetLengthByDPIScale(15));

    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);

    image_view_ = new views::ImageView();
    image_view_->set_interactive(false);
    image_view_->SetImage(rb.GetImageSkiaNamed(IDR_LIVEHIME_TITLEBAR_BILIBILI));

    auto titlebar_label = new LivehimeTitleLabel(GetLocalizedString(IDS_BILILIVE_TITLE));
    titlebar_label->SetTextColor(SK_ColorWHITE);
    titlebar_label->SetFont(ftFourteen);

    headshot_button_ = new HeadshotButton();

    gfx::ImageSkia img_skia;
    auto& avatar_data = GetBililiveProcess()->secret_core()->user_info().avatar();
    if (!avatar_data.empty()) {
        img_skia = bililive::MakeSkiaImage(avatar_data.data(), avatar_data.size());
    }
    else
    {
        img_skia = *GetImageSkiaNamed(IDR_LIVEMAIN_TITLEBAR_USER_FACE);
    }

    DrawImage(img_skia);

    button_area_ = new ButtonArea;

    menu_button_ = new TitlebarImageButton(this);
    menu_button_->SetAllStateImage(GetImageSkiaNamed(IDR_LIVEHIME_V3_TITLEBAR_MORE));
    menu_button_->SetTooltipText(GetLocalizedString(IDS_UGC_MORE));
    menu_button_->set_id(Button_Menu);

    menu_button_->SetReportEventDetails(secret::LivehimeBehaviorEventV2::TitlebarItemClick, "button_type:1");
    button_area_->AddChildView(menu_button_);

    mini_button_ = new TitlebarImageButton(this);
    mini_button_->SetImage(views::Button::STATE_NORMAL, GetImageSkiaNamed(IDR_LIVEHIME_V3_TITLEBAR_MIN));
    mini_button_->SetTooltipText(GetLocalizedString(IDS_UGC_MINIMISE));
    mini_button_->set_id(Button_Min);

    max_button_ = new TitlebarImageButton(this);
    max_button_->SetImage(views::Button::STATE_NORMAL, GetImageSkiaNamed(IDR_LIVEHIME_V3_TITLEBAR_MAX));
    max_button_->SetTooltipText(GetLocalizedString(IDS_UGC_MAXIMIN));
    max_button_->set_id(Button_Max);

    close_button_ = new TitlebarImageButton(this);
    close_button_->SetImage(views::Button::STATE_NORMAL, rb.GetImageSkiaNamed(IDR_LIVEHIME_V3_TITLEBAR_CLOSE));
    close_button_->SetTooltipText(GetLocalizedString(IDS_UGC_CLOSE));
    close_button_->set_id(Button_Close);

    layout->StartRow(0, 0);
    layout->AddView(image_view_);
    layout->AddView(titlebar_label);
    layout->AddView(headshot_button_);
    layout->AddView(button_area_);
    layout->AddView(mini_button_);
    layout->AddView(max_button_);
    layout->AddView(close_button_);

    base::MessageLoop::current()->PostTask(FROM_HERE, base::Bind(&TitleBarViddupView::InitUserInfo, weakptr_factory_.GetWeakPtr()));
}

void TitleBarViddupView::InitUserInfo()
{
    GetBililiveProcess()->bililive_obs()->user_info_service()->GetUserInfo();
    GetBililiveProcess()->bililive_obs()->user_info_service()->GetRoomInfo();
}

void TitleBarViddupView::OnWidgetBoundsChanged(views::Widget* widget, const gfx::Rect& new_bounds)
{
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();
    if (widget->IsMaximized())
    {
        max_button_->SetImage(views::Button::STATE_NORMAL, rb.GetImageSkiaNamed(IDR_LIVEHIME_V3_TITLEBAR_RESTORE));
        max_button_->SetTooltipText(rb.GetLocalizedString(IDS_UGC_RESTORE));
    }
    else
    {
        max_button_->SetImage(views::Button::STATE_NORMAL, rb.GetImageSkiaNamed(IDR_LIVEHIME_V3_TITLEBAR_MAX));
        max_button_->SetTooltipText(rb.GetLocalizedString(IDS_UGC_MAXIMIN));
    }
}

void TitleBarViddupView::OnGlobalSettingRedPointShowChanged()
{
}

void TitleBarViddupView::Observe(int type, const base::NotificationSource& source, const base::NotificationDetails& details)
{
    switch (type)
    {
    case bililive::NOTIFICATION_LIVEHIME_LOAD_ROOMINFO_SUCCESS:
    {
        UpdateRoomInfo();
    }
        break;
    case bililive::NOTIFICATION_LIVEHIME_UPDATE_AVATAR:
    {
        UpdateAvatar();
    }
        break;
    default:
        break;
    }
}

void TitleBarViddupView::PaintChildren(gfx::Canvas* canvas)
{
    __super::PaintChildren(canvas);
}

void TitleBarViddupView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    switch (sender->id())
    {
    case Button_Menu:
    {
        gfx::Rect rt = menu_button_->GetBoundsInScreen();
        TitleBarMoreMenu *menu = new TitleBarMoreMenu(this);
        views::MenuRunner runner(menu);
        runner.RunMenuAt(
            sender->GetWidget()->GetTopLevelWidget(), nullptr, gfx::Rect(rt.x(), rt.bottom(), 0, 0),
            views::MenuItemView::TOPLEFT, ui::MENU_SOURCE_MOUSE, views::MenuRunner::HAS_MNEMONICS);
    }
        break;
    case Button_Setup:
    {
        int index = static_cast<int>(SettingIndex::VideoSettings);
        bililive::ExecuteCommandWithParams(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_SETUP, CommandParams<int>(&index));
    }
        break;
    case Button_Share:
        bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_SHARE);
        break;
    case Button_Min:
        bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_MINIMIZE_WINDOW);
        break;
    case Button_Max:
    {
        if (GetWidget()->IsMaximized())
        {
            bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_RESTORE_WINDOW);
        }
        else
        {
            bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_MAXIMIZE_WINDOW);
        }
    }
        break;
    case Button_Close:
        bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_CLOSE_APP);
        break;
    default:
        break;
    }
}

void TitleBarViddupView::ExecuteCommand(int command_id, int event_flags)
{
    bool handle = true;
    int update_type = 1;
    switch (command_id)
    {
    case TitleBarMoreMenu::COMMAND_HELP:
        base::NotificationService::current()->Notify(
            bililive::NOTIFICATION_LIVEHIME_MAYBE_USER_CONFUSED,
            base::NotificationService::AllSources(),
            base::NotificationService::NoDetails());
        livehime::ShowHelp(livehime::HelpType::Home);
        livehime::BehaviorEventReport(secret::LivehimeBehaviorEventV2::MoreMenuItemClick, "button_type:2");
        break;
    case TitleBarMoreMenu::COMMAND_DETECT:
        EventTracking(secret::LivehimeBehaviorEvent::LivehimeMoreVersionsCheck);
        bililive::ExecuteCommandWithParams(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_CHECK_FOR_UPDATE, CommandParams<int>(&update_type));
        livehime::BehaviorEventReport(secret::LivehimeBehaviorEventV2::MoreMenuItemClick, "button_type:3");
        break;
    case TitleBarMoreMenu::SUB_COMMAND_LOGFILE_DISK:
        {
            base::NotificationService::current()->Notify(
                bililive::NOTIFICATION_LIVEHIME_MAYBE_USER_CONFUSED,
                base::NotificationService::AllSources(),
                base::NotificationService::NoDetails());
            EventTracking(secret::LivehimeBehaviorEvent::LivehimeMoreLogfile);
            bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_SHOW_LOG_FILE);
            livehime::BehaviorEventReport(secret::LivehimeBehaviorEventV2::MoreMenuItemClick, "button_type:4");
        }
        break;
    case TitleBarMoreMenu::SUB_COMMAND_LOGFILE_UPLOAD:
        {
            livehime::UploadLogFilesAndReport(true);

            HotkeyNotifyView::ShowForm(GetLocalizedString(IDS_MAIN_VIEW_FILE_LOGFILE_UPLOAD_FINISH),
                GetBililiveProcess()->bililive_obs()->obs_view()->get_preview_rect(), nullptr, &ftPrimary);
        }
        break;
    case TitleBarMoreMenu::COMMAND_SERVICE:
    {

    }
        break;
    case TitleBarMoreMenu::COMMAND_USER_AGREEMENT:
    {
        std::string url = bililive::AppendURLQueryParams("https://www.bilibili.tv/en/user-agreement", "pc_ui=400,600,0,1");
        ShowWebWidget(url);
        break;
    }
    case TitleBarMoreMenu::COMMAND_PRIVACY_AGREEMENT: {
        std::string url = bililive::AppendURLQueryParams("https://www.bilibili.tv/en/privacy-policy", "pc_ui=400,600,0,1");
        ShowWebWidget(url);
        break;      
    }
#ifdef _DEBUG
    case TitleBarMoreMenu::COMMAND_WEBTEST:
        livehime_test::ShowPopupLivehimeBrowser(GetWidget());
        break;
#endif
    case TitleBarMoreMenu::COMMAND_SETTING: {
        int index = static_cast<int>(SettingIndex::VideoSettings);
        bililive::ExecuteCommandWithParams(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_SETUP, CommandParams<int>(&index));
        break;
    }
    default:
        handle = false;
        break;
    }
    if (!handle)
    {
        if (command_id >= TitleBarMoreMenu::COMMAND_PROJECTOR_BEGIN &&
            command_id <= TitleBarMoreMenu::COMMAND_PROJECTOR_END)
        {
            EventTracking(secret::LivehimeBehaviorEvent::LivehimeMorePrejection);

            MonitorInfo cur_monitor = TitleBarMoreMenu::monitors_[command_id - TitleBarMoreMenu::COMMAND_PROJECTOR_BEGIN];
            PreviewProjectorView::ShowForm(gfx::Rect(cur_monitor.x_, cur_monitor.y_, cur_monitor.cx_, cur_monitor.cy_));
            livehime::BehaviorEventReport(secret::LivehimeBehaviorEventV2::MoreMenuItemClick, "button_type:1");
        }
    }
}

void TitleBarViddupView::ShowWebWidget(std::string web_url) {

    WebBrowserPopupDetails wbpd;
    wbpd.web_url = web_url;
    wbpd.allow_popup = WebViewPopupType::System;
    bililive::ExecuteCommandWithParams(GetBililiveProcess()->bililive_obs(),
        IDC_LIVEHIME_SHOW_BILIBILI_COMMON_WEB_WINDOW,
        CommandParams<WebBrowserPopupDetails>(&wbpd));
}

int TitleBarViddupView::GetHTComponentForFrame(const gfx::Point &point)
{
    if (GetEventHandlerForPoint(point) == this) {
        return HTCAPTION;
    }

    return HTNOWHERE;
}

void TitleBarViddupView::DrawImage(const gfx::ImageSkia& img)
{
    headshot_button_->UpdateHeadshot(img);
}

void TitleBarViddupView::UpdateRoomInfo() {
    auto room_id = GetBililiveProcess()->secret_core()->user_info().room_id();
    if (room_id > 0)
    {
        LOG(INFO) << "roomid valid, start liveroom socket connect.";
        auto uid = GetBililiveProcess()->secret_core()->account_info().mid();
        GetBililiveProcess()->bililive_obs()->broadcast_viddup_service()->StartListening(uid, room_id);
    }
    else
    {
        LOG(WARNING) << "roomid invalid, will start liveroom socket connect next time.";
    }
}

void TitleBarViddupView::UpdateAvatar() {
    auto& avatar_data = GetBililiveProcess()->secret_core()->user_info().avatar();
    auto img = bililive::MakeSkiaImage(avatar_data.data(), avatar_data.size());
    if (!img.isNull()) {
        DrawImage(img);
    }
}

void TitleBarViddupView::SetBusinessFunctionButtonVisible(bool visible)
{
    headshot_button_->SetVisible(visible);
    button_area_->SetVisible(visible);
}