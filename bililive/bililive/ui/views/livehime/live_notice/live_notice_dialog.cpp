#include "bililive/bililive/ui/views/livehime/live_notice/live_notice_dialog.h"

#include "base/ext/callable_callback.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"

#include "bililive/bililive/bililive_process_impl.h"
#include "bililive/bililive/ui/views/controls/util/bililive_util_views.h"
#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/hotkey_notify/bililive_toast_notify_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_image_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_sub_edit.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_tabbed.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/bililive/ui/views/controls/bililive_labelbutton.h"
#include "bililive/bililive/utils/convert_util.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/grid_layout.h"

namespace {

    LiveNoticeDialog* g_single_instance = nullptr;

    const unsigned int kLimitTitleNumb = 60;

    enum class ImgType {
        NewRoom,
        OldRoom,
        Net,
        Watch,
    };

    class NoticeImageView : public views::View
        , public views::ButtonListener
    {
    public:
        NoticeImageView(ImgType type,const base::string16& img_text)
        {
            views::GridLayout* layout = new views::GridLayout(this);
            SetLayoutManager(layout);
            views::ColumnSet* columnset = layout->AddColumnSet(0);
            columnset->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);

            columnset = layout->AddColumnSet(1);
            columnset->AddColumn(views::GridLayout::CENTER, views::GridLayout::CENTER, 1.0f, views::GridLayout::USE_PREF, 0, 0);

            const gfx::Size kImageSize(GetLengthByDPIScale(184), GetLengthByDPIScale(140));
            auto icon_image = new LivehimeImageView(this);
            icon_image->SetPreferredSize(kImageSize);
            icon_image->SetEnabled(false);

            switch (type)
            {
            case ImgType::NewRoom:
                icon_image->SetImage(*GetImageSkiaNamed(IDR_LIVEHIME_V3_NOTICE_NEW_ROOM));
                break;
            case ImgType::OldRoom:
                icon_image->SetImage(*GetImageSkiaNamed(IDR_LIVEHIME_V3_NOTICE_OLD_ROOM));
                break;
            case ImgType::Net:
                icon_image->SetImage(*GetImageSkiaNamed(IDR_LIVEHIME_V3_NOTICE_NET));
                break;
            case ImgType::Watch:
                icon_image->SetImage(*GetImageSkiaNamed(IDR_LIVEHIME_V3_NOTICE_WATCH));
                break;
            default:
                break;
            }

            layout->StartRow(0, 0);
            layout->AddView(icon_image);

            auto label = new LivehimeContentLabel(img_text);
            label->SetTextColor(SkColorSetRGB(0x9E, 0xAB, 0xB5));
            label->SetFont(ftTwelve);
            layout->StartRowWithPadding(1.0f, 1,0, kPaddingRowHeightForCtrlTips);
            layout->AddView(label);
        }

    protected:
        // ButtonListener
        void ButtonPressed(views::Button* sender, const ui::Event& event) override
        {
        }
    };

}

LiveNoticeDialog::LiveNoticeDialog()
    : BililiveWidgetDelegate(gfx::ImageSkia(), GetLocalizedString(IDS_NOTICE_WINDOW_TITLE)),
    weakptr_factory_(this),
    old_content_(L"")
{
}

LiveNoticeDialog::~LiveNoticeDialog()
{
    g_single_instance = nullptr;
}

void LiveNoticeDialog::ShowWindow(views::Widget * parent)
{
    if (!g_single_instance)
    {
        views::Widget *widget = new views::Widget();

        views::Widget::InitParams params;
        params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
        params.native_widget = new BililiveNativeWidgetWin(widget);
        params.parent = parent->GetNativeView();

        g_single_instance = new LiveNoticeDialog();
        DoModalWidget(g_single_instance, widget, params);
    }
    else
    {
        g_single_instance->GetWidget()->Activate();
    }
}

void LiveNoticeDialog::Close()
{
    if (g_single_instance)
    {
        g_single_instance->GetWidget()->Close();
    }
}

void LiveNoticeDialog::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails & details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            InitViews();
        }
    }
}

void LiveNoticeDialog::InitViews()
{
    views::GridLayout* layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    views::ColumnSet* column_set = layout->AddColumnSet(0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

    column_set = layout->AddColumnSet(1);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

    column_set = layout->AddColumnSet(2);
    column_set->AddPaddingColumn(0, GetLengthByDPIScale(40));
    column_set->AddColumn(views::GridLayout::CENTER, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, GetLengthByDPIScale(20));
    column_set->AddColumn(views::GridLayout::CENTER, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);

    column_set = layout->AddColumnSet(3);
    column_set->AddPaddingColumn(1.0f, 0);
    column_set->AddColumn(views::GridLayout::CENTER, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForActionButton);
    column_set->AddColumn(views::GridLayout::CENTER, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(1.0f, 0);

    auto title_label = new LivehimeTitleLabel(GetLocalizedString(IDS_NOTICE_TITLE));
    title_label->SetFont(ftFourteen);
    title_label->SetTextColor(GetColor(LabelTitle));
    layout->AddPaddingRow(0, GetLengthByDPIScale(16));
    layout->StartRow(0, 1);
    layout->AddView(title_label);

    title_num_label_ = new LivehimeSmallContentLabel(base::StringPrintf(L"0/%2d ", kLimitTitleNumb));
    title_num_label_->SetPreferredSize(gfx::Size(ftTwelve.GetStringWidth(L" 99/99"), ftTwelve.GetHeight()));
    text_area_ = new LivehimeSubEditView(title_num_label_);
    text_area_->set_placeholder_text(GetLocalizedString(IDS_NOTICE_New_ROOM_EDITTEXT));
    text_area_->SetController(this);
    //最长允许输入字数限制的2倍长度的字符串 因为英文和数字只算半个字符
    text_area_->LimitText(kLimitTitleNumb*2);

    layout->AddPaddingRow(0, GetLengthByDPIScale(12));
    layout->StartRow(0, 0);
    layout->AddView(text_area_, 1, 1, views::GridLayout::Alignment::FILL, views::GridLayout::Alignment::FILL, 0, GetLengthByDPIScale(70));

    auto label = new LivehimeTitleLabel(GetLocalizedString(IDS_NOTICE_DESCRIPTION_TITLE));
    label->SetFont(ftFourteen);
    label->SetTextColor(SkColorSetRGB(0x53,0x67,0x77));
    layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForDiffGroups);
    layout->AddView(label);

    layout->AddPaddingRow(0, kPaddingRowHeightForGroupTips);
    auto label1  = new LivehimeContentLabel(GetLocalizedString(IDS_NOTICE_DESCRIPTION));
    label1->SetTextColor(SkColorSetRGB(0x86,0x96,0xA3));
    label1->SetFont(ftTwelve);
    layout->StartRow(0, 1);
    layout->AddView(label1);

    layout->AddPaddingRow(0, kPaddingRowHeightForGroupTips);
    label1 = new LivehimeContentLabel(GetLocalizedString(IDS_NOTICE_DESCRIPTION_PART));
    label1->SetFont(ftTwelve);
    layout->StartRow(0, 1);
    layout->AddView(label1);

    NoticeImageView* img_view = new NoticeImageView(ImgType::NewRoom, GetLocalizedString(IDS_NOTICE_LIVEROOM_NEW));
    layout->StartRowWithPadding(0, 2, 0, GetLengthByDPIScale(16));
    layout->AddView(img_view);

    img_view = new NoticeImageView(ImgType::OldRoom, GetLocalizedString(IDS_NOTICE_LIVEROOM_OLD));
    layout->AddView(img_view);

    layout->StartRowWithPadding(0, 2, 0, GetLengthByDPIScale(16));
    img_view = new NoticeImageView(ImgType::Net, GetLocalizedString(IDS_NOTICE_LIVEROOM_WEB));
    layout->AddView(img_view);

    img_view = new NoticeImageView(ImgType::Watch, GetLocalizedString(IDS_NOTICE_PAGE_FLOOW));
    layout->AddView(img_view);

    ok_button_ = new LivehimeActionLabelButton(this, GetLocalizedString(IDS_SRCPROP_COMMON_OK), true);
    ok_button_->SetIsDefault(true);
    cancel_button_ = new LivehimeActionLabelButton(this, GetLocalizedString(IDS_SRCPROP_COMMON_CANCEL), false);

    layout->StartRowWithPadding(1.0f, 3, 0, kPaddingColWidthEndsSmall);
    layout->AddView(ok_button_);
    layout->AddView(cancel_button_);
    layout->AddPaddingRow(0, kPaddingColWidthEndsSmall);

}

gfx::Size LiveNoticeDialog::GetPreferredSize()
{
    gfx::Size size = __super::GetPreferredSize();
    size.SetSize(GetLengthByDPIScale(460), GetLengthByDPIScale(650));
    return size;
}

void LiveNoticeDialog::OnNewRoomSetting(bool success, int code, const std::string& message)
{
    if (!success)
    {
        ShowToastMsg(GetLocalizedString(IDS_NOTICE_SERVER_CONNNECT_FAIL));
    }
    if (success && code!=0 )
    {
        if (code == 6)
        {
            ShowToastMsg(GetLocalizedString(IDS_NOTICE_SENSITIVE_INFORMATION));
        }
        else
        {
            ShowToastMsg(base::UTF8ToUTF16(message));
        }
    }
    else
    {
        ShowToastMsg(GetLocalizedString(IDS_NOTICE_NOTICE_SET_SUCCESS));
    }
    GetWidget()->Close();
}

void LiveNoticeDialog::RequestUpdateNotice(const std::string& content)
{
 
}

void LiveNoticeDialog::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    if (sender == ok_button_)
    {
        base::string16 content = text_area_->GetText();
        if (content.empty() || content == GetLocalizedString(IDS_NOTICE_New_ROOM_EDITTEXT))
        {
            ShowToastMsg(GetLocalizedString(IDS_NOTICE_EMPTY));

            GetWidget()->Close();
            return;
        }
        else
        {
            RequestUpdateNotice(base::UTF16ToUTF8(content));
        }

        livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::LiveNewRoomSubClick, "");
    }
    else
    {
        GetWidget()->Close();
    }
}

void LiveNoticeDialog::ShowToastMsg(const base::string16& box_text)
{
    gfx::Rect hnv_s = GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget()->GetWindowBoundsInScreen();
    ToastNotifyView::ShowForm(box_text,
        hnv_s, nullptr, &ftPrimary);
}

void LiveNoticeDialog::ContentsChanged(BilibiliNativeEditView* sender, const string16& new_contents)
{
    int first_pos = new_contents.find_first_of(L"\r\n");
    if (first_pos != new_contents.npos)
    {
        text_area_->SetText(old_content_);
        return;
    }

    int count = bililive::CountWords(new_contents);
    if (count <= kLimitTitleNumb)
    {
        old_len_ = count;
        old_content_ = new_contents;
        if (!sender)
        {
            text_area_->SetText(old_content_);
        }
    }
    else
    {
        text_area_->SetText(old_content_);
    }

    title_num_label_->SetText(base::StringPrintf(L"%d/%d ", old_len_, kLimitTitleNumb));
}
