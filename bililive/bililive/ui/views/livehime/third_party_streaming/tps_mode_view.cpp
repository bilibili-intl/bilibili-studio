#include "bililive/bililive/ui/views/livehime/third_party_streaming/tps_mode_view.h"

#include <map>

#include "base/bind.h"
#include "base/notification/notification_observer.h"
#include "base/notification/notification_registrar.h"
#include "base/notification/notification_service.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"

#include "ui/base/clipboard/clipboard.h"
#include "ui/base/clipboard/scoped_clipboard_writer.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/grid_layout.h"

#include "bililive/bililive/livehime/help_center/help_center.h"
#include "bililive/bililive/livehime/qrcode/qrcode_generator.h"
#include "bililive/bililive/ui/bililive_commands.h"
#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/hotkey_notify/bililive_hotkey_notify_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_hover_tip_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_image_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_textfield.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/bililive/ui/views/livehime/third_party_streaming/tps_obs_guide_view.h"
#include "bililive/bililive/utils/bililive_canvas_drawer.h"
#include "bililive/bililive/utils/bililive_image_util.h"
#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "bililive/public/bililive/bililive_process.h"


namespace
{
    const char kRtmpUrlHead[] = "rtmp://";
    const char kRtmpUrlName[] = "/livehime";
    const SkColor kTpsBkColor = SkColorSetRGB(0x1B, 0x1B, 0x1B);
	const int kRowSpacingBig = GetLengthByDPIScale(20);
	const int kRowSpacingSmall = GetLengthByDPIScale(10);

    enum
    {
        Button_Quit = 1,
        Button_Help,
        Button_AddrCopy,
        Button_CodeCopy,
        Button_OBS,
        Button_ChangeQR,
    };

    class OverlapLayout : public views::LayoutManager
    {
    public:
        OverlapLayout() = default;
        void Layout(views::View* host)override
        {
			if (host->child_count() > 1)
			{
                gfx::Size size0 = host->child_at(0)->GetPreferredSize();
                gfx::Size size1 = host->child_at(1)->GetPreferredSize();
                host->child_at(0)->SetBounds(0, 0, size0.width(), size0.height());
                host->child_at(1)->SetBounds(size0.width() - size1.width() - GetLengthByDPIScale(10), size0.height()/2 - size1.height() / 2, 
                    size1.width(), size1.height());
			}
        }
		gfx::Size GetPreferredSize(views::View* host)override
		{
            if (host->child_count() > 1)
            {
                return host->child_at(0)->GetPreferredSize();
            }
            return gfx::Size(10,10);
		}
    };

    class GuideStepView : public views::View
    {
    public:
        explicit GuideStepView(int step)
            : step_(step)
        {
            views::GridLayout *layout = new views::GridLayout(this);
            SetLayoutManager(layout);

            views::ColumnSet *column_set = layout->AddColumnSet(0);
            column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);

            column_set = layout->AddColumnSet(1);
            column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);

            LivehimeImageView* img = new LivehimeImageView();

            layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
            layout->StartRow(0, 0);
            layout->AddView(img);

            base::string16 step_str;
            switch (step)
            {
            case 0:
                step_str = GetLocalizedString(IDS_TPS_STEP_FIRST);// "复制推流地址和推流码";
                img->SetImage(*GetImageSkiaNamed(IDR_LIVEHIME_TPS_STEP_FIRST));
                break;
            case 1:
                step_str = GetLocalizedString(IDS_TPS_STEP_SECOND);// L"填入OBS等第三方推流工具";
                img->SetImage(*GetImageSkiaNamed(IDR_LIVEHIME_TPS_STEP_SECOND));
                break;
            case 2:
                step_str = GetLocalizedString(IDS_TPS_STEP_THIRD);// L"第三方推流软件点击开播";
                img->SetImage(*GetImageSkiaNamed(IDR_LIVEHIME_TPS_STEP_THIRD));
                break;
            case 3:
                step_str = GetLocalizedString(IDS_TPS_STEP_FOURTH);// L"直播姬选择分区自动开播";
                img->SetImage(*GetImageSkiaNamed(IDR_LIVEHIME_TPS_STEP_FOURTH));
                break;
            default:
                break;
            }
            BililiveLabel* label = new LivehimeSmallContentLabel(step_str);
            label->SetMultiLine(true);
            label->SetAllowCharacterBreak(true);
            label->SetTextColor(SK_ColorWHITE);

            layout->AddPaddingRow(0, kPaddingRowHeightForCtrlTips);
            layout->StartRow(0, 1);
            layout->AddView(label);
        }

    protected:
        // View
        void OnPaint(gfx::Canvas* canvas) override
        {
            __super::OnPaint(canvas);

            /*static int kRadius = GetLengthByDPIScale(14);
            SkPaint paint;
            paint.setColor(SK_ColorWHITE);
            paint.setAntiAlias(true);
            canvas->DrawCircle(gfx::Point(kRadius, kRadius), kRadius, paint);
            canvas->DrawStringInt(std::to_wstring(step_ + 1), ftPrimary, SK_ColorBLACK,
                0, 0, kRadius * 2, kRadius * 2, gfx::Canvas::TEXT_ALIGN_CENTER);*/
        }

    private:
        int step_ = 0;
    };
}


ThirdPartyStreamingView::ThirdPartyStreamingView(bool preset_material)
    : weakptr_factory_(this), preset_material_(preset_material)
{
}

ThirdPartyStreamingView::~ThirdPartyStreamingView()
{
}

void ThirdPartyStreamingView::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails &details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            InitViews();

            LivehimeLiveRoomController::GetInstance()->AddObserver(this);
        }
        else
        {
            LivehimeLiveRoomController::GetInstance()->RemoveObserver(this);
        }
    }
}

void ThirdPartyStreamingView::InitViews()
{
    static int kCloseButtonWidth = GetImageSkiaNamed(IDR_LIVEHIME_V3_TITLEBAR_CLOSE)->width();
    SkColor content_color;
    SkColor edit_color;
    SkColor edit_bg_Color;
    if (preset_material_)
    {
        content_color = GetColor(LabelTitle);
        edit_color = SkColorSetARGB(0x99, 0x53, 0x67, 0x77);
        edit_bg_Color = SK_ColorWHITE;
    }
    else
    {
        content_color = SkColorSetARGB(0x99, 0xff, 0xff, 0xff);
        edit_color = SkColorSetRGB(0xff, 0xff, 0xff);
        edit_bg_Color = kTpsBkColor;
        set_background(views::Background::CreateSolidBackground(kTpsBkColor));
    }

	views::GridLayout* main_layout = new views::GridLayout(this);
	this->SetLayoutManager(main_layout);
    int main_column_set_index = 0;

	if (!preset_material_)
	{
		//标题
		views::ColumnSet* column_set = main_layout->AddColumnSet(main_column_set_index);
		column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls + kCloseButtonWidth * 2 + 5);
		column_set->AddPaddingColumn(1.0f, 0);
		column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
		column_set->AddPaddingColumn(1.0f, 0);
		column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
		column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);

		BililiveLabel* label = new LivehimeTitleLabel(GetLocalizedString(IDS_TPS_TITLE_FULLNAME));
		label->SetTextColor(content_color);
		label->SetFont(ftSixteen);

		BililiveImageButton* quit_button = new BililiveImageButton(this);
		quit_button->SetTooltipText(GetLocalizedString(IDS_EXIT));
		quit_button->SetAllStateImage(GetImageSkiaNamed(IDR_LIVEHIME_V3_TITLEBAR_CLOSE));
		quit_button->set_id(Button_Quit);

        main_layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
        main_layout->StartRow(0, 0);
        main_layout->AddView(label);
        main_layout->AddView(quit_button);
        main_column_set_index++;
	}

    auto content_view = new views::View();
    views::GridLayout *layout = new views::GridLayout(content_view);
    content_view->SetLayoutManager(layout);
    int column_set_index = 0;
    //地址 
    views::ColumnSet* column_set = layout->AddColumnSet(column_set_index);
    column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, GetLengthByDPIScale(10));
    column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);

	BililiveLabel* addr_label = new LivehimeContentLabel(L"推流地址");//L"RTMP地址："
	addr_label->SetTextColor(content_color);

    auto addr_bg_view = new views::View();
    addr_bg_view->SetLayoutManager(new OverlapLayout());
    addr_bg_view->set_background(views::Background::CreateSolidBackground(edit_bg_Color));
	addr_textfield_ = new LivehimeTextfield(nullptr);
	addr_textfield_->text_field()->set_default_width_in_chars(50);
	addr_textfield_->text_field()->SetReadOnly(true);
	addr_textfield_->SetBackgroundColor(edit_bg_Color);
	addr_textfield_->SetTextColor(edit_color);

	bt_copy_addr_ = new CustomStyleButton(this, GetLocalizedString(IDS_TPS_COPY), ftFourteen);
	bt_copy_addr_->SetAllStateTextColor(SkColorSetRGB(0x0e, 0xbe, 0xff));
	bt_copy_addr_->set_id(Button_AddrCopy);

    addr_bg_view->AddChildView(addr_textfield_);
    addr_bg_view->AddChildView(bt_copy_addr_);

    layout->StartRowWithPadding(0, column_set_index, 0, preset_material_ ? GetLengthByDPIScale(40) : GetLengthByDPIScale(60));
    layout->AddView(addr_label);
    layout->AddView(addr_bg_view);

    //推流码
	BililiveLabel* code_label = new LivehimeContentLabel(L"推流码");//L"推流码："
	code_label->SetTextColor(content_color);

	auto code_bg_view = new views::View();
    code_bg_view->SetLayoutManager(new OverlapLayout());
    code_bg_view->set_background(views::Background::CreateSolidBackground(edit_bg_Color));
	code_textfield_ = new LivehimeTextfield(nullptr);
	code_textfield_->text_field()->set_default_width_in_chars(50);
	code_textfield_->text_field()->SetReadOnly(true);
	code_textfield_->SetBackgroundColor(edit_bg_Color);
	code_textfield_->SetTextColor(edit_color);
    
    bt_copy_code_ = new CustomStyleButton(this, GetLocalizedString(IDS_TPS_COPY), ftFourteen);
    bt_copy_code_->SetAllStateTextColor(SkColorSetRGB(0x0e, 0xbe, 0xff));
    bt_copy_code_->set_id(Button_CodeCopy);

    code_bg_view->AddChildView(code_textfield_);
    code_bg_view->AddChildView(bt_copy_code_);

	layout->StartRowWithPadding(0, column_set_index, 0, kRowSpacingSmall);
	layout->AddView(code_label);
	layout->AddView(code_bg_view);

    //推流地址切换按钮
	column_set_index++;
	column_set = layout->AddColumnSet(column_set_index);
    column_set->AddPaddingColumn(1.0f, GetLengthByDPIScale(10));
	column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(1.0f, GetLengthByDPIScale(10));

	change_button_ = new LivehimeLinkButton(this, L"推流失败？换一个");
	change_button_->SetFont(ftTwelve);
    change_button_->SetTextColorEx(views::Button::STATE_NORMAL, preset_material_ ? SkColorSetARGB(0x99, 0x53, 0x67, 0x77) : SkColorSetARGB(0x99, 0xff, 0xff, 0xff));
	change_button_->SetTextColorEx(views::Button::STATE_HOVERED, SkColorSetRGB(0x0e, 0xbe, 0xff));
	change_button_->SetTextColorEx(views::Button::STATE_PRESSED, SkColorSetRGB(0x0e, 0xbe, 0xff));
	change_button_->SetDrawUnderline(true);
    change_button_->set_id(Button_ChangeQR);

	layout->StartRowWithPadding(0, column_set_index, 0, kRowSpacingSmall);
	layout->AddView(change_button_);

    //OBS引导
    //OBS设置
    column_set_index++;
    column_set = layout->AddColumnSet(column_set_index);
	column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::LEADING, 0, views::GridLayout::USE_PREF, 0, 0);
 
	BililiveLabel* obs_label = new LivehimeContentLabel(L"OBS设置");
    obs_label->SetTextColor(content_color);

	layout->StartRowWithPadding(0, column_set_index, 0, kRowSpacingBig);
	layout->AddView(obs_label);

	//图示
	column_set_index++;
	column_set = layout->AddColumnSet(column_set_index);
	column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::LEADING, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, GetLengthByDPIScale(20));
    column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::LEADING, 0, views::GridLayout::USE_PREF, 0, 0);

	BililiveLabel* label1 = new LivehimeContentLabel(L"1.复制上方推流码，黏贴至OBS的设置>推流");
    label1->SetTextColor(content_color);
    label1->SetFont(ftTwelve);

	BililiveLabel* label2 = new LivehimeContentLabel(L"2.完成参数设置，开始推流");
    label2->SetTextColor(content_color);
    label2->SetFont(ftTwelve);

	LivehimeImageView* img1 = new LivehimeImageView();
	img1->SetImage(*GetImageSkiaNamed(IDR_PRESET_MATERIAL_OBS_COPY_CODE));

	LivehimeImageView* img2 = new LivehimeImageView();
	img2->SetImage(*GetImageSkiaNamed(IDR_PRESET_MATERIAL_OBS_START));

	layout->StartRowWithPadding(0, column_set_index, 0, kRowSpacingSmall);
	layout->AddView(label1);
    layout->AddView(label2);

	layout->StartRowWithPadding(0, column_set_index, 0, kRowSpacingSmall);
	layout->AddView(img1);
	layout->AddView(img2);  

    column_set = main_layout->AddColumnSet(main_column_set_index);
	column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
	column_set->AddPaddingColumn(1.0f, 0);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(1.0f, 0);

    main_layout->StartRowWithPadding(1.0f, main_column_set_index, 0, kRowSpacingSmall);
    main_layout->AddView(content_view);
    main_layout->StartRowWithPadding(1.0f, main_column_set_index, 0, kRowSpacingSmall);
}

void ThirdPartyStreamingView::ChangeQRImage()
{
    if (index_ < 0 || index_ >= (int)ip_list_.size())
    {
        NOTREACHED();
        return;
    }

    std::string url = kRtmpUrlHead;
    url += ip_list_[index_];
    url += ":";
    url += std::to_string(port_);
    url += kRtmpUrlName;

    std::string rtmp = base::StringPrintf("rtmp://%s:%d", ip_list_[index_].c_str(), port_);
    addr_textfield_->text_field()->SetText(base::UTF8ToUTF16(rtmp));
    code_textfield_->text_field()->SetText(L"livehime");

    //gfx::ImageSkia qr_image = bililive::GenerateURLQRCode(url, 3);
    //qr_image_view_->SetImage(qr_image);
}

void ThirdPartyStreamingView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    switch (sender->id())
    {
    case Button_Quit:
        {
            LOG(INFO) << "click quit tps mode button.";
            if (LivehimeLiveRoomController::GetInstance()->IsLiveReady())
            {
                LOG(INFO) << "quit tps mode.";
                SetEnabled(false);
                bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_LEAVE_THIRD_PARTY_STREAM_MODE);
            }
            else
            {
                LOG(INFO) << "quit tps mode ask.";
                // 提示，退出第三方模式将停止直播，是否确认退出
                livehime::MessageBoxEndDialogSignalHandler handler;
                handler.closure = base::Bind(&ThirdPartyStreamingView::QuitEndDialog, weakptr_factory_.GetWeakPtr());
                livehime::ShowMessageBox(GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget()->GetNativeWindow(),
                    GetLocalizedString(IDS_LIVE_ROOM_MSGBOX_TITLE),
                    L"退出第三方推流模式将停止直播，是否确认退出？",
                    GetLocalizedString(IDS_COLIVE_MSGBOX_QUIT) + L"," + GetLocalizedString(IDS_COLIVE_MSGBOX_CANCEL),
                    &handler,
                    livehime::MessageBoxType_NONEICON,
                    GetLocalizedString(IDS_COLIVE_MSGBOX_QUIT));
            }
        }
        break;
    case Button_Help:
        livehime::ShowHelp(livehime::HelpType::ThirdPartyStreaming);
        break;
    case Button_AddrCopy:
    {
        ui::ScopedClipboardWriter clip(ui::Clipboard::GetForCurrentThread(), ui::Clipboard::BUFFER_STANDARD);
        clip.WriteText(addr_textfield_->text_field()->text());
        HotkeyNotifyView::ShowForm(L"推流地址已复制",
            GetBililiveProcess()->bililive_obs()->obs_view()->get_preview_rect(),
            nullptr, &ftPrimary);
    }
        break;
    case Button_CodeCopy:
    {
        ui::ScopedClipboardWriter clip(ui::Clipboard::GetForCurrentThread(), ui::Clipboard::BUFFER_STANDARD);
        clip.WriteText(code_textfield_->text_field()->text());
        HotkeyNotifyView::ShowForm(L"推流码已复制",
            GetBililiveProcess()->bililive_obs()->obs_view()->get_preview_rect(),
            nullptr, &ftPrimary);
    }
        break;
    case Button_OBS:
        ThirdPartyStreamingOBSGuideView::ShowForm(
            GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget()->GetNativeView());
        break;
    case Button_ChangeQR:
    {
        if (++index_ >= (int)ip_list_.size())
        {
            index_ = 0;
        }
        ChangeQRImage();
    }
        break;
    default:
        break;
    }
}

void ThirdPartyStreamingView::QuitEndDialog(const base::string16& btn, void* data)
{
    if (btn == GetLocalizedString(IDS_COLIVE_MSGBOX_QUIT))
    {
        LOG(INFO) << "quit tps mode ask choose quit.";
        SetEnabled(false);
        bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(), 
            IDC_LIVEHIME_LEAVE_THIRD_PARTY_STREAM_MODE);
    }
}

void ThirdPartyStreamingView::OnThirdPartyStreamingListened(const std::vector<std::string>& ip_list, int port)
{
    index_ = 0;
    ip_list_ = ip_list;
    port_ = port;
    if (ip_list.size() > 1)
    {
        change_button_->SetVisible(true);
    }
    else
    {
        change_button_->SetVisible(false);
    }
    if (!ip_list.empty())
    {
        ChangeQRImage();
    }
    else
    {
        LOG(INFO) << "tps can't enum valid network addrs.";
    }
}
