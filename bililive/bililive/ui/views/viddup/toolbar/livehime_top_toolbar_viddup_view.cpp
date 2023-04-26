#include "bililive/bililive/ui/views/viddup/toolbar/livehime_top_toolbar_viddup_view.h"

#include "base/ext/callable_callback.h"
#include "base/notification/notification_service.h"
#include "base/prefs/pref_service.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/strings/string_number_conversions.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/grid_layout.h"

#include "bililive/bililive/livehime/common_pref/common_pref_names.h"
#include "bililive/bililive/livehime/user_info/user_info_service.h"
#include "bililive/bililive/livehime/function_control/app_function_controller.h"
#include "bililive/bililive/livehime/danmaku_hime/danmakuhime_data_handler.h"
#include "bililive/bililive/utils/bililive_canvas_drawer.h"
#include "bililive/bililive/ui/bililive_commands.h"
#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/controls/util/bililive_util_views.h"
#include "bililive/bililive/ui/views/hotkey_notify/bililive_hotkey_notify_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_bubble.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_search_list_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_image_view.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/bililive/ui/views/livehime/web_browser/livehime_web_browser_datatype.h"
#include "bililive/bililive/ui/views/preview/livehime_preview_area_utils.h"
#include "bililive/bililive/utils/bililive_image_util.h"
#include "bililive/bililive/utils/time_span.h"
#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/common/bililive_context.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/secret/public/anchor_info.h"


namespace
{
    enum CtrlID
    {
        Button_Partition,
        Button_RoomTitle,
        Button_ChangeRoomTitle,
        Button_Cover,
        Button_Notice,
        Button_Subject,
        Button_Share,
    };

    enum TitleButtonStatus
    {
        ClickToEdit,
        ClickToSave,
    };

    const int kMinTitleEditWidth = GetLengthByDPIScale(150);
    const int kMinPartitionBtnWidth = GetLengthByDPIScale(80);
    const int kMinPartitionBtnHeight = GetLengthByDPIScale(20);
    const int kPaddingColWidth = GetLengthByDPIScale(10);
    const int kVtuberAreaID = 9;            // ID of the virtual anchor zone
    const unsigned int kLimitTitleNumb = 20;
    const unsigned int kLimitHistoryTitleNumb = 5;
    const SkColor kClrSearchEditBk = SkColorSetRGB(0x0f, 0x89, 0xcf);
    const int kLiveLiveTimeSpanInMin = 5;

    void ShowWarningBubble(views::View* view, const base::string16& text)
    {
        BililiveBubbleView* bubble = livehime::ShowBubble(view, views::BubbleBorder::Arrow::BOTTOM_CENTER,
            text);
        bubble->set_background_color(GetColor(TextWarning));
        //bubble->SetAlignment(views::BubbleBorder::BubbleAlignment::ALIGN_EDGE_TO_ANCHOR_EDGE);
        bubble->set_text_color(SK_ColorWHITE);
        bubble->StartFade(false, 2000, 1000);
    }

    const std::vector<base::string16> kTitleList{
        L"Cute new arrival! Give more advice£¡",
        L"First report! Pay attention to the button£¡",
        L"First broadcast! Please take care of me£¡",
    };

    class TopToolbarRoomTitleEdit : public BilibiliSearchListView
    {
    public:
        TopToolbarRoomTitleEdit(BilibiliSearchListListener *listener, BililiveLabel* label)
            : BilibiliSearchListView(listener, label)
        {
            //edit_ctrl()->RemoveBorder();
            edit_ctrl()->set_placeholder_text(L"Enter the title to attract more fans");
            SetLimitText(kLimitTitleNumb);
            SetFont(ftTwelve);
            SetTextColor(clrTextALL);
        }

        virtual ~TopToolbarRoomTitleEdit() = default;

        void EditFocus()
        {
            edit_ctrl()->SetFocus();
            edit_ctrl()->SelectAll();
        }

    protected:
        // View
        gfx::Size GetPreferredSize()
        {
            gfx::Size size;
            if (visible())
            {
                size = View::GetPreferredSize();
                size.set_width(std::max(kMinTitleEditWidth, size.width()));
                size.set_height(GetLengthByDPIScale(24));
            }

            return size;
        }

        int GetHeightForWidth(int w)
        {
            return visible() ? View::GetHeightForWidth(w) : 0;
        }
    };

    class PartitionButton : public LivehimeCapsuleButton
    {
    public:
        PartitionButton(views::ButtonListener* listener, const string16& text)
            : LivehimeCapsuleButton(listener, text)
        {
            BililiveLabelButton::ColorStruct clrs_details;
            BililiveLabelButton::GetButtonStyles(LivehimeButtonStyle_CoverButton, clrs_details);
            SetColorDetails(clrs_details);
            label()->SetTextColor(clrTextCover);
        }

    protected:
        // View
        gfx::Size GetPreferredSize() override
        {
            gfx::Size size;
            auto it = label()->GetPreferredSize();
            size.SetSize(std::max(it.width() + GetLengthByDPIScale(20), GetLengthByDPIScale(68)), GetLengthByDPIScale(20));
            return size;
        }

    private:
    };

    // Portrait new feature introduction
    class PreviewButtonGuideView :
        public views::View,
        views::ButtonListener
    {
    public:
        PreviewButtonGuideView()
        {
            views::GridLayout* layout = new views::GridLayout(this);
            SetLayoutManager(layout);

            static const int kLabelCX = GetLengthByDPIScale(170);

            auto col = layout->AddColumnSet(0);
            col->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
            col->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED,
                kLabelCX, kLabelCX);
            col->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);

            col = layout->AddColumnSet(1);
            col->AddPaddingColumn(1.0f, kPaddingColWidthForGroupCtrls);
            col->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
            col->AddPaddingColumn(1.0f, kPaddingColWidthForGroupCtrls);

            BililiveLabel* label = new LivehimeContentLabel(GetLocalizedString(IDS_TABAREA_MODEL_PREVIEW_GUIDE));
            label->SetHorizontalAlignment(gfx::ALIGN_LEFT);
            label->SetMultiLine(true);
            label->SetAllowCharacterBreak(true);
            label->SetTextColor(SK_ColorWHITE);

            layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForGroupCtrls);
            layout->AddView(label);

            LivehimeTitlebarButton* close_btn = new LivehimeTitlebarButton(this, GetLocalizedString(IDS_IKNOW));

            layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForGroupCtrls);
            layout->AddView(close_btn);

            layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
        }

        ~PreviewButtonGuideView() = default;

    protected:
        // ButtonListener
        void ButtonPressed(views::Button* sender, const ui::Event& event) override
        {
            GetWidget()->Close();
        }
    };

    class BookLiveTimeView : public views::View
    {
    public:
        BookLiveTimeView()
        {
            views::GridLayout *layout = new views::GridLayout(this);
            SetLayoutManager(layout);
            views::ColumnSet *columnset = layout->AddColumnSet(0);
            columnset->AddPaddingColumn(0, GetLengthByDPIScale(30));
            columnset->AddColumn(views::GridLayout::CENTER, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
            columnset->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);

            book_label_ = new LivehimeTitleLabel(L"");
            book_label_->SetFont(ftEleven);
            layout->AddPaddingRow(0, GetLengthByDPIScale(5));
            layout->StartRow(0, 0);
            layout->AddView(book_label_);
            layout->AddPaddingRow(0, GetLengthByDPIScale(5));
        }

        void SetBookText(const base::string16& book_info) {
            book_label_->SetText(book_info);
        }

    protected:
        // View
        void OnPaintBackground(gfx::Canvas* canvas) override
        {
            __super::OnPaintBackground(canvas);
            auto bound = GetContentsBounds();
            SkPaint paint;
            paint.setAntiAlias(true);
            paint.setColor(SkColorSetRGB(0xED,0xF1,0xF5));
            paint.setStyle(SkPaint::kFill_Style);
            bililive::FillRoundRect(
                canvas, bound.x(), bound.y(), bound.width(), bound.height(), 4, paint);
            auto image = *GetImageSkiaNamed(IDR_LIVEHIME_V3_ACTIVITY_BOOK_TIME);
            canvas->DrawImageInt(image, GetLengthByDPIScale(8), bound.y() + (bound.height() - image.height())/2);
        }

    private:
        BililiveLabel* book_label_;
    };

    class CoverAdviceImageView : public LivehimeImageView
    {
    public:
        CoverAdviceImageView()
        {
            bg_paint_.setAntiAlias(true);
            bg_paint_.setStyle(SkPaint::kFill_Style);
            bg_paint_.setColor(SkColorSetA(SK_ColorBLACK, 255 * 0.76f));

            advice_img_ = GetImageSkiaNamed(IDR_LIVEHIME_COVER_ADVICE_BAR);
            SetScaleType(ST_FILL);
        }

    protected:
        void OnPaint(gfx::Canvas* canvas) override
        {
            super::OnPaint(canvas);
            auto bounds = GetContentsBounds();

            canvas->DrawRoundRect(bounds, GetLengthByDPIScale(2), bg_paint_);
            canvas->DrawImageInt(*advice_img_, bounds.x(), bounds.y());
        }

    private:
        using super = LivehimeImageView;
        gfx::ImageSkia* advice_img_ = nullptr;

        SkPaint bg_paint_;
        gfx::Rect bg_rect_;
    };

    class  CoverButtonView : public LivehimeImageView
    {
     public:
         enum Status { kNormal, kAudit, kNotPass };

         explicit CoverButtonView(views::ButtonListener* listener )
            : LivehimeImageView(listener)
         {
             bg_paint_.setAntiAlias(true);
             bg_paint_.setStyle(SkPaint::kStrokeAndFill_Style);
             bg_paint_.setColor(SkColorSetA(SK_ColorBLACK, 255 * 0.76f));
             upload_img_ = GetImageSkiaNamed(IDR_LIVEHIME_UPLOAD_BAR);
             audit_img_ = GetImageSkiaNamed(IDR_LIVEHIME_AUDIT_MASK);
             audit_not_pass_img_ = GetImageSkiaNamed(IDR_LIVEHIME_AUDIT_NOT_PASS_MASK);
             advice_warning_img_ = GetImageSkiaNamed(IDR_LIVEHIME_COVER_ADVICE_WARNING_BAR);
             advice_tip_img_ = GetImageSkiaNamed(IDR_LIVEHIME_COVER_ADVICE_TIP_BAR);

             SetImage(*GetImageSkiaNamed(IDR_LIVEHIME_COVER_BAR));
             SetScaleType(ST_SHOW_ALL);
         }

         void SetStatus(Status status)
         {
             status_ = status;
         }

         void SetImageRaw(std::string& data, bool is_vert)
         {
             img_data_ = std::move(data);
             cover_image_ = bililive::MakeSkiaImage((const unsigned char*)img_data_.c_str(), img_data_.size());
             if (!cover_image_.isNull())
             {
                SetImage(cover_image_);
             }
             else
             {
                 SetImage(*GetImageSkiaNamed(IDR_LIVEHIME_UPLOAD_BAR));
             }

             if (is_vert)
             {
                 SetScaleType(ST_SHOW_ALL);
             }
             else
             {
                 SetScaleType(ST_FILL);
             }
         }

         void SetShowCoverAdvice(bool bAdvice) {
             show_cover_advice_ = bAdvice;
             SchedulePaint();
         }

    protected:
        void OnMouseEntered(const ui::MouseEvent& event) override
        {
            is_mouse_entered_ = true;
            SchedulePaint();
        }

        bool OnMousePressed(const ui::MouseEvent& event) override {
            if (!event.IsLeftMouseButton()) {
                return false;
            }

            is_mouse_entered_ =  false;
            return true;
        }

        void OnMouseExited(const ui::MouseEvent& event) override
        {
            is_mouse_entered_ = false;
            SchedulePaint();
            //SetImage(*GetImageSkiaNamed(IDR_LIVEHIME_COVER_BAR));
        }

        void OnPaint(gfx::Canvas* canvas) override
        {
            super::OnPaint(canvas);
            auto bounds = GetContentsBounds();

            switch (status_)
            {
                case Status::kNormal:
                {
                    if ( is_mouse_entered_)
                    {
                        canvas->DrawRoundRect(bounds, GetLengthByDPIScale(2), bg_paint_);
                        canvas->DrawImageInt(
                            *upload_img_,
                            bounds.x() + GetLengthByDPIScale(36),
                            bounds.y() + GetLengthByDPIScale(27));
                    }
                    else {
                        if (show_cover_advice_) {
                            gfx::Rect rc;
                            rc.set_width(bounds.width());
                            rc.set_x(bounds.x());
                            rc.set_y(bounds.y() + bounds.height() - GetLengthByDPIScale(20));
                            rc.set_height(GetLengthByDPIScale(20));
                            canvas->DrawRoundRect(rc, GetLengthByDPIScale(2), bg_paint_);
                            canvas->DrawImageInt(*advice_warning_img_, rc.x() + GetLengthByDPIScale(22), rc.y() + GetLengthByDPIScale(6));
                            canvas->DrawImageInt(*advice_tip_img_, rc.x() + GetLengthByDPIScale(36), rc.y() + GetLengthByDPIScale(4));
                        }
                    }
                }
                break;

                case Status::kAudit:
                {
                    canvas->DrawImageInt(*audit_img_, 0, 0, audit_img_->width(), audit_img_->height(), 0, 0, bounds.width(), bounds.height(), true);
                }
                break;

                case Status::kNotPass:
                {
                    canvas->DrawImageInt(*audit_not_pass_img_, 0, 0, audit_img_->width(), audit_img_->height(), 0, 0, bounds.width(), bounds.height(), true);
                }
                break;
            }
        }

    private:
        using super = LivehimeImageView;
        bool is_mouse_entered_ = false;
        gfx::ImageSkia* upload_img_ = nullptr;
        gfx::ImageSkia* audit_img_ = nullptr;
        gfx::ImageSkia* audit_not_pass_img_ = nullptr;
        std::string     img_data_;
        gfx::ImageSkia  cover_image_;
        SkPaint bg_paint_;
        Status status_ = CoverButtonView::Status::kNormal;
        gfx::Rect bg_rect_;
        //CoverAdviceImageView advice_image_;
        bool show_cover_advice_ = false;
        gfx::ImageSkia* advice_warning_img_ = nullptr;
        gfx::ImageSkia* advice_tip_img_ = nullptr;
    };

    class  ToolRandomButtonView : public views::View
    {
        class BiliveRandomButton : public BililiveLabelButton {
        public:
            BiliveRandomButton(views::ButtonListener* listener, const string16& text)
                :BililiveLabelButton(listener, text)
            {
                SetFont(ftTwelve);
                SetTextColor(Button::STATE_NORMAL, GetColor(LabelTitle));
                SetTextColor(Button::STATE_HOVERED, SkColorSetRGB(0x0E, 0xBE, 0xFF));
                SetTextColor(Button::STATE_PRESSED, SkColorSetRGB(0x0E, 0xBE, 0xFF));
                set_round_corner(false);
                SetImage(Button::STATE_NORMAL, *GetImageSkiaNamed(IDR_LIVEHIME_RANDOM_BAR));
                SetImage(Button::STATE_HOVERED, *GetImageSkiaNamed(IDR_LIVEHIME_RANDOM_BAR_HOV));
                SetImage(Button::STATE_HOVERED, *GetImageSkiaNamed(IDR_LIVEHIME_RANDOM_BAR_HOV));
                SetPreferredSize(GetSizeByDPIScale({ 38, 24 }));
            }
        };

     public:
         ToolRandomButtonView(views::ButtonListener* listen, const base::string16& str) {
             random_button_ = new BiliveRandomButton(listen, str);
             SetLayoutManager(new views::FillLayout());
             AddChildView(random_button_);
         }

         void set_id(int id)
         {
             random_button_->set_id(id);
         };

         void OnPaint(gfx::Canvas* canvas) override
         {
             __super::OnPaint(canvas);

             gfx::Rect rect = this->bounds();
             canvas->DrawLine(gfx::Point(rect.width() * 0.001, rect.height() * 0.3),
                 gfx::Point(rect.width() * 0.001, rect.height() * 0.7), SkColorSetRGB(0xCC, 0xD2, 0xD9));
         }

         gfx::Size GetPreferredSize() override
         {
             gfx::Size size = __super::GetPreferredSize();
             size.set_width(GetLengthByDPIScale(54));
             return size;
         }

         views::Button* button() { return random_button_; }

    private:
        BiliveRandomButton* random_button_ = nullptr;
    };

    void OnRoomCoverMainViewClose(bool request_cover_info_by_start_live_flow, int close_type)
    {
    }
}

LivehimeTopToolbarViddupView::LivehimeTopToolbarViddupView()
    : weakptr_factory_(this)
{
}

LivehimeTopToolbarViddupView::~LivehimeTopToolbarViddupView()
{
}

void LivehimeTopToolbarViddupView::ViewHierarchyChanged(const ViewHierarchyChangedDetails &details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            InitViews();

            notifation_register_.Add(this, bililive::NOTIFICATION_LIVEHIME_ROOMINFO_UPDATE_SUCCESS,
                                     base::NotificationService::AllSources());
            notifation_register_.Add(this, bililive::NOTIFICATION_LIVEHIME_CREATE_LIVEROOM_FAILED,
                                     base::NotificationService::AllSources());
            notifation_register_.Add(this, bililive::NOTIFICATION_LIVEHIME_CREATE_LIVEROOM_SUCCESS,
                                     base::NotificationService::AllSources());

            LivehimeLiveRoomController::GetInstance()->AddObserver(this);
        }
        else
        {
            if (preview_button_guide_bubble_ && preview_button_guide_bubble_->GetWidget())
            {
                preview_button_guide_bubble_->GetWidget()->RemoveObserver(this);
            }

            notifation_register_.RemoveAll();
            LivehimeLiveRoomController::GetInstance()->RemoveObserver(this);
        }
    }
}

gfx::Size LivehimeTopToolbarViddupView::GetPreferredSize()
{
    gfx::Size size = __super::GetPreferredSize();
    size.set_height(GetHeightForWidth(size.width()));
    return size;
}

int LivehimeTopToolbarViddupView::GetHeightForWidth(int w)
{
    return GetLengthByDPIScale(55);
}

void LivehimeTopToolbarViddupView::OnPaintBackground(gfx::Canvas* canvas)
{
    __super::OnPaintBackground(canvas);
    
    gfx::Rect rect(GetContentsBounds());
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(GetColor(WindowTitle));
    bililive::FillRoundRect(canvas, rect.x(), rect.y(), rect.width(), rect.height(), GetLengthByDPIScale(8), paint);
}

//void LivehimeTopToolbarViddupView::Layout() {
//    gfx::Rect rect(GetContentsBounds());
//}

void LivehimeTopToolbarViddupView::InitViews()
{
    set_background(views::Background::CreateSolidBackground(GetColor(WindowTitle)));
}

void LivehimeTopToolbarViddupView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
   
}

void LivehimeTopToolbarViddupView::OnWidgetClosing(views::Widget* widget)
{
    preview_button_guide_bubble_ = nullptr;
}

void LivehimeTopToolbarViddupView::ShowTitleEdit(bool show)
{
    title_button_area_view_->SetVisible(!show);
    room_title_edit_->SetVisible(show);
    room_title_edit_->SetEnabled(show);

    if (show)
    {
        room_title_edit_->SetText(room_title_button_->GetText(), BilibiliNativeEditView::EnChangeReason::ECR_DEVICE);
        //room_title_edit_->SelectAll();
        room_title_edit_->RequestFocus();
        room_title_edit_->DoDropDown();
    }
}

void LivehimeTopToolbarViddupView::SetLiveRoomTitle()
{
    auto& anchor_info = GetBililiveProcess()->secret_core()->anchor_info();
    room_title_edit_->ClearItems();
    string16 title = base::UTF8ToUTF16(anchor_info.room_title());
    if (!title.empty())
    {
        room_title_button_->SetText(title);
        room_title_edit_->SetText(title);
        room_title_edit_->AddItem(title, title.length());
    }

    std::vector<string16> title_history = GetTitleHistroy();
    auto result = std::find(title_history.begin(),
                            title_history.end(), title);
    if (result == title_history.end())
    {
        const int kGetListNumb = 4;
        int count = std::min(kGetListNumb, static_cast<int>(title_history.size()));
        for (int i = 0; i < count; ++i)
        {
            string16 history = title_history.at(i);
            room_title_edit_->AddItem(history, history.length());
        }
    }
    else
    {
        for (const auto& it : title_history)
        {
            if (it != title)
            {
                room_title_edit_->AddItem(it, it.length());
            }
        }
    }

    InvalidateLayout();
    Layout();
    SchedulePaint();
}

void LivehimeTopToolbarViddupView::RefreshTitleEdit() {
    SetLiveRoomTitle();
}

// BilibiliSearchListListener
void LivehimeTopToolbarViddupView::OnSearchListResultSelected(BilibiliSearchListView* search_edit, const base::string16 &text, int64 data)
{
    title_num_label_->SetText(base::StringPrintf(L"%d/%d ", text.length(), kLimitTitleNumb));
    title_num_label_->SetTextColor(text.length() == kLimitTitleNumb ? GetColor(TextWarning) : GetColor(TextContent));
}

void LivehimeTopToolbarViddupView::OnSearchEditContentsChanged(BilibiliSearchListView* search_edit, const string16& new_contents)
{
    title_num_label_->SetText(base::StringPrintf(L"%d/%d ", new_contents.length(), kLimitTitleNumb));
    title_num_label_->SetTextColor(new_contents.length() == kLimitTitleNumb ? GetColor(TextWarning) : GetColor(TextContent));
}

void LivehimeTopToolbarViddupView::OnSearchEditFocus(BilibiliSearchListView* search_edit)
{
    ShowTitleEdit(true);
    Layout();
}

void LivehimeTopToolbarViddupView::PreSearchEditBlur(BilibiliSearchListView* search_edit, const gfx::Point& pt_in_screen)
{
    auto title = search_edit->GetText();
    if (!title.empty()) {
        room_title_button_->SetText(title);
        InvalidateLayout();
        Layout();
        SchedulePaint();
        UpdateTitle(title);
    }
}

void LivehimeTopToolbarViddupView::OnSearchEditBlur(BilibiliSearchListView* search_edit, BiliveSearchListBlurChangeReason reason)
{
    if (reason == BiliveSearchListBlurChangeReason::CHANGED_BY_USER_RETURN)
    {
        auto title = search_edit->GetText();
        if (!title.empty()) {
            room_title_button_->SetText(title);
            InvalidateLayout();
            Layout();
            SchedulePaint();
            UpdateTitle(title);
        }
        else
        {
            // The user deleted the copy, but it can't be empty
            BililiveBubbleView* bubble = livehime::ShowBubble(title_button_area_view_, views::BubbleBorder::Arrow::BOTTOM_RIGHT,
                L"Enter the title to attract more fans~");
            bubble->StartFade(false, 2000, 1000);
        }
    }

    ShowTitleEdit(false);
}

void LivehimeTopToolbarViddupView::Observe(int type, const base::NotificationSource& source, const base::NotificationDetails& details)
{
    switch (type)
    {
    case bililive::NOTIFICATION_LIVEHIME_LOAD_ROOMINFO_SUCCESS:
        {
            SetLiveRoomTitle();
            SetAreaName();
            RefreshRoomInfo();
            ShowTalkBubble();
            OnPartionChanged();

            static bool inited = true;
            if (inited)
            {
                inited = false;
                RequestCoverStatus(true);
                SetHotRankInfoVisible();
            }
        }
        break;
    case bililive::NOTIFICATION_LIVEHIME_ROOMINFO_UPDATE_SUCCESS:
        {
            SetAreaName();
            RefreshRoomInfo();
            OnPartionChanged();

        }
        break;
    case bililive::NOTIFICATION_LIVEHIME_CREATE_LIVEROOM_FAILED:
        {
            if (!!details.map_key()){
               auto info = reinterpret_cast<UserInfoService::CreateRoomInfo*>(details.map_key());
               if (info) {
                   base::string16 toast_msg;
                   if (!info->valid_response) {
                       toast_msg = GetLocalizedString(IDS_TOOLBAR_ROOMINFO_LOAD_FAILED);
                   }
                   else {
                       toast_msg = base::UTF8ToUTF16(info->err_msg);
                   }
               }
            }
        }
        break;
    case bililive::NOTIFICATION_LIVEHIME_CREATE_LIVEROOM_SUCCESS:
        {
            int from_type = -1;
            if (!!details.map_key())
            {
                from_type = *reinterpret_cast<int*>(details.map_key());
            }
            if (-1 != from_type)
            {
                switch (from_type)
                {
                case 0: // partition
                    ButtonPressed(partition_button_, ui::MouseEvent(
                        ui::EventType::ET_MOUSE_PRESSED, gfx::Point(), gfx::Point(), ui::EventFlags::EF_LEFT_MOUSE_BUTTON));
                    break;
                case 1: // title
                    ButtonPressed(room_title_button_, ui::MouseEvent(
                        ui::EventType::ET_MOUSE_PRESSED, gfx::Point(), gfx::Point(), ui::EventFlags::EF_LEFT_MOUSE_BUTTON));
                    break;
                case 2:
                    ButtonPressed(change_title_button_->button(), ui::MouseEvent(
                        ui::EventType::ET_MOUSE_PRESSED, gfx::Point(), gfx::Point(), ui::EventFlags::EF_LEFT_MOUSE_BUTTON));
                    break;
                case 3: // cover
                    ButtonPressed(main_button_, ui::MouseEvent(
                        ui::EventType::ET_MOUSE_PRESSED, gfx::Point(), gfx::Point(), ui::EventFlags::EF_LEFT_MOUSE_BUTTON));
                    break;
                default:
                    break;
                }
            }
        }
        break;
    default:
        break;
    }
}

void LivehimeTopToolbarViddupView::UpdateTitle(const base::string16& wtitle)
{

}

void LivehimeTopToolbarViddupView::OnPopDialog()
{
    if (is_show_talk_bubble) {
        PrefService* pref = GetBililiveProcess()->profile()->GetPrefs();
        bool need_show_talk_subject = pref->GetBoolean(prefs::kBililiveTalkSubjectShow);
        if (need_show_talk_subject && talk_subject_button_->visible())
        {
            base::MessageLoop::current()->PostDelayedTask(FROM_HERE,
                base::Bind(&LivehimeTopToolbarViddupView::OnDelayTalkBubbleTask, weakptr_factory_.GetWeakPtr()),
                base::TimeDelta::FromSeconds(1));

            pref->SetBoolean(prefs::kBililiveTalkSubjectShow, false);
            is_show_talk_bubble = false;
        }
    }
}

void LivehimeTopToolbarViddupView::SetLikeInfoVisible(bool visible)
{
    if (like_num_label_count_ && like_num_label_text_) {
        like_num_label_count_->SetVisible(visible);
        like_num_label_text_->SetVisible(visible);
    }
}

void LivehimeTopToolbarViddupView::OnUpdateTitle(
    bool success, int err_code, const std::string& error_msg)
{
    if (!success)
    {
        LOG(WARNING) << "Update title information unsuccess!";
        return;
    }
    if (err_code != 0)
    {
        base::string16 error = base::StringPrintf(L"%d ", err_code);
        error.append(base::UTF8ToUTF16(error_msg));

        if (!error.empty())
        {
            livehime::MessageBoxEndDialogSignalHandler handler;
            handler.closure = base::Bind(&LivehimeTopToolbarViddupView::OnUpdateTitleErrorMsgBoxHandle, weakptr_factory_.GetWeakPtr());
            livehime::ShowMessageBox(
                GetWidget()->GetNativeWindow(),
                GetLocalizedString(IDS_LIVE_ROOM_MSGBOX_TITLE),
                error,
                GetLocalizedString(IDS_LIVE_ROOM_MSGBOX_IKNOW),
                nullptr,
                &handler);
        }

        return;
    }

    std::string new_title = base::UTF16ToUTF8(room_title_button_->GetText());
    std::string old_title = GetBililiveProcess()->secret_core()->anchor_info().room_title();

    GetBililiveProcess()->secret_core()->anchor_info().set_room_title(new_title);
    SaveTitleHistory();
    RefreshRoomInfo();

    if (new_title != old_title)
    {
        livehime::BehaviorEventReportViaServer(secret::LivehimeViaServerBehaviorEvent::ChangeRoomInfo, {});
    }
}

void LivehimeTopToolbarViddupView::RoomAudience(int64_t audience) {
    string16 show_count = L"0";
    if (!GetBililiveProcess()->secret_core()->anchor_info().get_watched_switch()) {
        NUMBER_CONVERT(audience)
        string16 audience_text = L"Room popularity ";
        audience_text.append(show_count);
        audience_num_label_->SetText(audience_text);
        audience_num_label_->InvalidateLayout();
        Layout();
    }
}

void LivehimeTopToolbarViddupView::UpdateWatchedChange(int64_t number)
{
    std::lock_guard<std::mutex> lock_gurd(watched_mtx_);
    string16 show_count = L"0";
    NUMBER_CONVERT(number)
    show_count.append(L"People have seen");
    audience_num_label_->SetText(show_count);
    audience_num_label_->InvalidateLayout();
    Layout();
}

void LivehimeTopToolbarViddupView::UpdateLikeNumberChange(int64_t number)
{
    if (like_count_show_ && number > 0) {
        PrefService* pref = GetBililiveProcess()->profile()->GetPrefs();
        bool room_like_show = pref->GetBoolean(prefs::kBililiveRoomLikeShow);
        if (room_like_show) {
            pref->SetBoolean(prefs::kBililiveRoomLikeShow,false);
        }

        string16 show_count = L"0";
        NUMBER_CONVERT(number)
            like_num_label_count_->SetText(show_count);
        if (!like_num_label_count_->visible()) {
            SetLikeInfoVisible(true);
        }

        like_num_label_count_->InvalidateLayout();
        Layout();
    }
}

void LivehimeTopToolbarViddupView::SaveTitleHistory()
{
    std::vector<string16> title_history;
    int numb = room_title_edit_->GetItemCount();
    base::string16 new_title = room_title_button_->GetText();

    if (!new_title.empty())
    {
        title_history.push_back(new_title);
    }

    for (int i = 0; i < numb; ++i)
    {
        string16 title = room_title_edit_->GetItemText(i);
        title_history.push_back(title);
    }

    PrefService* pref = GetBililiveProcess()->profile()->GetPrefs();
    std::unique_ptr<base::ListValue> list_history_title(new ListValue());

    for (const auto& it : title_history)
    {
        if (list_history_title->Find(StringValue(it)) == list_history_title->end())
        {
            list_history_title->AppendString(it);
            if (list_history_title->GetSize() >= kLimitHistoryTitleNumb)
            {
                break;
            }
        }
    }

    pref->Set(prefs::kHistoryTitleName, *list_history_title.get());

    RefreshTitleEdit();
}

std::vector<string16> LivehimeTopToolbarViddupView::GetTitleHistroy()
{
    std::vector<string16> title_history;
    PrefService* pref = GetBililiveProcess()->profile()->GetPrefs();
    const base::ListValue* list = pref->GetList(prefs::kHistoryTitleName);

    for (const auto& it : *list)
    {
        string16 value;
        it->GetAsString(&value);
        title_history.push_back(value);
    }

    return title_history;
}

void LivehimeTopToolbarViddupView::SetAreaName()
{
    auto area_name = GetBililiveProcess()->secret_core()->anchor_info().current_area_name();
    if (!area_name.empty())
    {
        partition_button_->SetText(base::UTF8ToUTF16(area_name));
    }
    else
    {
        partition_button_->SetText(GetLocalizedString(IDS_TOOLBAR_LIVE_PARTITION));
    }

    InvalidateLayout();
    Layout();
    SchedulePaint();
}

void LivehimeTopToolbarViddupView::OnPartionChanged()
{
    if (GetBililiveProcess()->secret_core()->anchor_info().current_parent_area() == kVtuberAreaID)
    {
        vtuber_partion_ = true;
    }
    else
    {
        vtuber_partion_ = false;
    }
}

void LivehimeTopToolbarViddupView::ShowTalkBubble()
{
    is_show_talk_bubble = true;
    OnPopDialog();
}

void LivehimeTopToolbarViddupView::ShowUpdateView()
{
    if (!LivehimeLiveRoomController::GetInstance()->IsStreaming())
    {
        auto pref = GetBililiveProcess()->global_profile()->GetPrefs();
        bool noraml_build_tip = pref->GetBoolean(prefs::kLivehimeNormalUpdateTipValue);
        int normal_bulid_no = pref->GetInteger(prefs::kLivehimeNormalUpdateVersionTip);
        int update_type = pref->GetInteger(prefs::kLivehimeNormalUpdateType);
        int build_num = BililiveContext::Current()->GetExecutableBuildNumber();
        if (noraml_build_tip && (normal_bulid_no == build_num))
        {
            bililive::ExecuteCommandWithParams(GetBililiveProcess()->bililive_obs(),IDC_LIVEHIME_CHECK_FOR_UPDATE, CommandParams<int>(&update_type));
            pref->SetBoolean(prefs::kLivehimeNormalUpdateTipValue, false);
        }
    }
}

void LivehimeTopToolbarViddupView::RequestCoverStatus(bool inited)
{

}

void LivehimeTopToolbarViddupView::RoomTitleEditFocus()
{
    BililiveBubbleView* bubble_view = livehime::ShowBubble(room_title_edit_, views::BubbleBorder::Arrow::BOTTOM_LEFT, L"Change the livestream title here", true);
    bubble_view->StartFade(false, 3000, 1000);
    if (bubble_view && bubble_view->GetWidget())
    {
        bubble_view->set_text_color(SK_ColorWHITE);
        bubble_view->set_background_color(SkColorSetRGB(0xFB, 0x72, 0x99));
    }

    ShowTitleEdit(true);
    room_title_edit_->SelectAll();
    //Layout();
}

void LivehimeTopToolbarViddupView::OnGetCoverImage(bool is_vert, bool valid, int code, std::string& image_data, int audit_status)
{
    if (valid && code == 0)
    {
        switch (audit_status)
        {
            case 1:
                static_cast<CoverButtonView*>(main_button_)->SetStatus(CoverButtonView::kNormal);
            break;
            case 0:
                static_cast<CoverButtonView*>(main_button_)->SetStatus(CoverButtonView::kAudit);
            break;
            case -1:
                static_cast<CoverButtonView*>(main_button_)->SetStatus(CoverButtonView::kNotPass);
            break;
        }

        bool enable_vert = GetBililiveProcess()->secret_core()->anchor_info().enable_vert_cover();
        base::StringPairs param_pair;
        param_pair.push_back({ "param_pair",enable_vert ? "2" : "1" });
        livehime::PolarisEventReport(
            secret::LivehimePolarisBehaviorEvent::VertCoverShow, param_pair);
        static_cast<CoverButtonView*>(main_button_)->SetImageRaw(image_data, is_vert);
    }
}

void LivehimeTopToolbarViddupView::OnDelayTalkBubbleTask()
{
    BililiveBubbleView* bubble_view = livehime::ShowBubble(talk_subject_button_, views::BubbleBorder::Arrow::BOTTOM_LEFT, L"Bring the topic, more exposure£¡");
    bubble_view->StartFade(false, 3000, 1000);
    if (bubble_view && bubble_view->GetWidget())
    {
        bubble_view->set_text_color(SK_ColorWHITE);
        bubble_view->set_background_color(SkColorSetRGB(0xFB, 0x72, 0x99));
    }
}

void LivehimeTopToolbarViddupView::OnRequestGetCoverRedPointShowRes(bool valid_response, int code, const bool& need_red_point) {
    LOG(INFO) << "GetLowQualityCoverAdvice:" << need_red_point;
    static_cast<CoverButtonView*>(main_button_)->SetShowCoverAdvice(need_red_point);
    GetBililiveProcess()->profile()->GetPrefs()->SetBoolean(prefs::kLivehimeCoverRedPointShow, need_red_point);

    if (cover_show_need_report_) {
        livehime::PolarisEventReport(
            secret::LivehimePolarisBehaviorEvent::LiveRoomCoverShow, "red_point:" + std::to_string((int)need_red_point));
    }
}

void LivehimeTopToolbarViddupView::GetCoverRedPointShow(bool need_report) {
    cover_show_need_report_ = need_report;
    weakptr_factory_.InvalidateWeakPtrs();
}

void LivehimeTopToolbarViddupView::OnCpmOrderCompleted(int order_type)
{
    AddCompletedMarks(order_type);
}

void LivehimeTopToolbarViddupView::AddCompletedMarks(int order_type)
{
    completed_notice_.push_back({ order_type , 3 });

    if (!completed_showing_) {
        base::MessageLoop::current()->PostTask(FROM_HERE,
            base::Bind(&LivehimeTopToolbarViddupView::ShowCompletedMark, weakptr_factory_.GetWeakPtr()));
    }
}

void LivehimeTopToolbarViddupView::ShowCompletedMark()
{
    if (!completed_notice_.empty()) {
        auto notice = completed_notice_.front();
        completed_notice_.pop_front();

        current_order_type_ = notice.order_type;
        completed_mark_ = true;
        completed_showing_ = true;

        InvalidateLayout();
        Layout();
        SchedulePaint();

        completed_task_ = base::Bind(&LivehimeTopToolbarViddupView::HideCompletedMark, weakptr_factory_.GetWeakPtr());
        base::MessageLoop::current()->PostDelayedTask(FROM_HERE,
            completed_task_,
            base::TimeDelta::FromSeconds(notice.show_times_in_sec));
    }
}

void LivehimeTopToolbarViddupView::HideCompletedMark()
{
    completed_mark_ = false;
    InvalidateLayout();
    Layout();
    SchedulePaint();

    if (!completed_notice_.empty())
    {
        base::MessageLoop::current()->PostDelayedTask(FROM_HERE,
            base::Bind(&LivehimeTopToolbarViddupView::ShowCompletedMark, weakptr_factory_.GetWeakPtr()),
            base::TimeDelta::FromSeconds(2));
    }
    else {
        completed_showing_ = false;
    }
}

void LivehimeTopToolbarViddupView::OnUpdateTitleErrorMsgBoxHandle(const base::string16&, void* data)
{
    auto& anchor_info = GetBililiveProcess()->secret_core()->anchor_info();
    std::string title = anchor_info.room_title();
    if (title.empty())
    {
        if (!anchor_info.have_live())
        {
            room_title_button_->SetText(kTitleList.at(0));
            return;
        }
        else
        {
            NOTREACHED();
            room_title_button_->SetText(L"");
        }
    }
    else
    {
        room_title_button_->SetText(base::UTF8ToUTF16(title));
    }
}

void LivehimeTopToolbarViddupView::RefreshRoomInfo()
{
    auto& anchor_info = GetBililiveProcess()->secret_core()->anchor_info();
    string16 title = base::UTF8ToUTF16(anchor_info.room_title());
    auto iter = std::find_if(kTitleList.begin(), kTitleList.end(), [&](const base::string16& text)->bool {
        return title == text;
        });

    if (!title.empty() && iter == kTitleList.end())
    {
        change_title_button_->SetVisible(false);
        PrefService* pref = GetBililiveProcess()->profile()->GetPrefs();
        pref->SetBoolean(prefs::kChangedTitleBefore, true);
    }
    else
    {
        if (!GetBililiveProcess()->profile()->GetPrefs()->GetBoolean(prefs::kChangedTitleBefore))
        {
            change_title_button_->SetVisible(!anchor_info.have_live());
        }
    }
    if (!anchor_info.have_live() && title.empty())
    {
        if (!default_room_title_.empty())
        {
            room_title_button_->SetText(default_room_title_);
            
            if (GetBililiveProcess()->secret_core()->user_info().room_id() > 0)
            {
                UpdateTitle(default_room_title_);
            }
        }
    }

    InvalidateLayout();
    Layout();
}

// LivehimeLiveRoomObserver
void LivehimeTopToolbarViddupView::OnOpenLiveRoomSuccessed(const secret::LiveStreamingService::StartLiveInfo& start_live_info)
{
   
}

void LivehimeTopToolbarViddupView::OnLiveRoomStreamingStarted(const bililive::StartStreamingDetails& details)
{

}

void LivehimeTopToolbarViddupView::OnBookLiveInfoChanged(int type, const base::string16& book_info)
{

}

void LivehimeTopToolbarViddupView::OnAllAppKVCtrlInfoCompleted()
{
    if (AppFunctionController::GetInstance())
    {
        bool talk_show = AppFunctionController::GetInstance()->talk_topic_open();
        talk_subject_button_->SetVisible(talk_show);
        talk_subject_button_->InvalidateLayout();
        button_area_view_->InvalidateLayout();
        bool is_show = AppFunctionController::GetInstance()->fans_rank_open();
        SetFansRankVisible(is_show);
        InvalidateLayout();
        Layout();
    }
}

void LivehimeTopToolbarViddupView::OnLiveRoomClosed(bool is_restreaming, const secret::LiveStreamingService::StartLiveInfo& start_live_info)
{
    
}

void LivehimeTopToolbarViddupView::SetHotRankInfoVisible()
{
    auto  is_exist = GetBililiveProcess()->secret_core()->anchor_info().get_popular_rank_info().exist;
    auto  rank_num = GetBililiveProcess()->secret_core()->anchor_info().get_popular_rank_info().rank;
    if (is_exist && rank_num >= 0) {
		  
        base::StringPairs event_ext;
        auto  ranking_name = UTF8ToUTF16(GetBililiveProcess()->secret_core()->anchor_info().get_popular_rank_info().rank_name);
        event_ext.push_back(std::pair<std::string, std::string>("ranking_name", UTF16ToUTF8(ranking_name)));
        livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::HotRankShow, event_ext);
        hot_rank_broadcast_switch_ = true;
    } else {
  
        hot_rank_broadcast_switch_ = false;
    }
    
    Layout();
    SchedulePaint();
}

void LivehimeTopToolbarViddupView::OnCountDownTimer()
{
    if (countdown_ == 0) {
        hot_rank_timer_.Stop();
        
        Layout();
        SchedulePaint();
        return;
    }
    
    --countdown_;
}

void LivehimeTopToolbarViddupView::SetFansRankVisible(bool visible)
{
    
}