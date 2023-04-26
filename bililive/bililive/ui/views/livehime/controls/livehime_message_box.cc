#include "livehime_message_box.h"

#include "bililive/bililive/livehime/notify_ui_control/notify_ui_prop.h"
#include "bililive/bililive/livehime/cef/cef_hybrid_cache.h"
#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/controls/bililive_frame_view/bililive_frame_titlebar_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/public/bililive/bililive_process.h"

#include "ui/views/layout/grid_layout.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/fill_layout.h"

#include "ui/base/resource/resource_bundle.h"
#include "grit/generated_resources.h"
#include "grit/theme_resources.h"

#include "base/base64.h"
#include "base/sha1.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"

namespace
{
    std::string ComputesMessageBoxHashKey(
        const base::string16& title,
        const base::string16& message,
        const base::string16& buttons)
    {
        base::string16 str;
        str.append(title);
        str.append(message);
        str.append(buttons);
        unsigned char hashret[base::kSHA1Length] = { 0 };
        base::SHA1HashBytes((unsigned char*)str.data(), str.length() * sizeof(wchar_t), hashret);
        std::string hashbase64;
        base::Base64Encode(std::string((char*)hashret, base::kSHA1Length), &hashbase64);
        return hashbase64;
    }

    void LivehimeMessageBoxEndDialog(livehime::MessageBoxEndDialogSignalHandler handler,
                                     const base::string16& result)
    {
        if (!handler.closure.is_null())
        {
            handler.closure.Run(result, handler.data);
        }
    }

    void LivehimeGeneralMsgBoxEndDialog(livehime::GeneralMsgBoxEndDialogSignalHandler handler,
        const GeneralMsgBoxRes& result)
    {
        if (!handler.closure.is_null())
        {
            handler.closure.Run(result, handler.data);
        }
    }
}

std::map<std::string, gfx::NativeView> LivehimeMessageBox::open_msgbox_map_;

bool LivehimeMessageBox::FindMessageBoxAndFlash(const std::string &hashkey)
{
    if (open_msgbox_map_.find(hashkey) != open_msgbox_map_.end())
    {
        BililiveWidgetDelegate::FlashModalWidget(open_msgbox_map_[hashkey]);
        return true;
    }
    return false;
}

LivehimeMessageBox::LivehimeMessageBox(
    const std::shared_ptr<base::string16>& result,
    const base::string16& title,
    const base::string16& message,
    const gfx::ImageSkia* image_skia,
    const base::string16& buttons,
    const livehime::MessageBoxEndDialogSignalHandler* handler/* = nullptr*/,
    gfx::HorizontalAlignment align/* = gfx::ALIGN_LEFT*/,
    const base::string16& def_button/* = L""*/,
    int button_padding/* = 0*/)
    : BililiveWidgetDelegate(gfx::ImageSkia(), title)
    , msgbox_result_(result)
    , image_view_(nullptr)
    , button_padding_(button_padding)
{
    if (handler)
    {
        msgbox_end_dialog_handler_ = *handler;
    }

    ResourceBundle &rb = ResourceBundle::GetSharedInstance();

    views::GridLayout *layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    views::ColumnSet *column_set = layout->AddColumnSet(0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    if (image_skia)
    {
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
    }
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 1.0f, views::GridLayout::FIXED, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

    // 最少保持足够显示三个button的宽度
    std::vector<base::string16> btns;
    base::SplitStringDontTrim(buttons, ',', &btns);
    int btn_num = std::max(3, static_cast<int>(btns.size()));

    int cx = (LivehimeFunctionLabelButton::GetButtonSize().width() + kPaddingColWidthForActionButton) * btn_num - kPaddingColWidthForActionButton;
    column_set = layout->AddColumnSet(1);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    column_set->AddPaddingColumn(1.0f, 0);
    column_set->AddColumn(views::GridLayout::CENTER, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, cx, cx);
    column_set->AddPaddingColumn(1.0f, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

    label_ = new LivehimeContentLabel(message);
    label_->SetVerticalAlignment(gfx::ALIGN_VCENTER);
    label_->SetHorizontalAlignment(align);
    label_->SetAllowCharacterBreak(true);
    label_->SetMultiLine(true);
    if (image_skia)
    {
        image_view_ = new views::ImageView();
        image_view_->SetImage(image_skia);
        label_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
    }

    layout->AddPaddingRow(0, kPaddingRowHeightEnds);

    layout->StartRow(1.0f, 0);
    if (image_view_)
    {
        layout->AddView(image_view_);
    }
    layout->AddView(label_);

    layout->AddPaddingRow(0, kPaddingRowHeightForDiffGroups);

    buttons_bar_ = new views::View();
    {
        if (button_padding_ == 0) {
            button_padding_ = kPaddingColWidthForActionButton;
        }
        views::BoxLayout *box_layout = new views::BoxLayout(views::BoxLayout::Orientation::kHorizontal, 0, 0, button_padding_);
        buttons_bar_->SetLayoutManager(box_layout);

        // 有指定默认ok按钮的就设置指定按钮，没有的就将第一个设为默认ok按钮
        bool is_set_def = false;

        for (auto var = btns.rbegin(); var != btns.rend(); ++var)
        {
            bool is_def = false;
            if (!def_button.empty())
            {
                if (*var == def_button)
                {
                    if (!is_set_def)
                    {
                        is_set_def = true;
                        is_def = true;
                    }
                    else
                    {
                        NOTREACHED() << "为什么要设置两个一样文案的按钮";
                    }
                }
            }
            else
            {
                if (var == --btns.rend())
                {
                    is_set_def = true;
                    is_def = true;
                }
            }
            LivehimeActionLabelButton* button = new LivehimeActionLabelButton(this, *var, is_def);
            button->SetIsDefault(is_def);
            buttons_bar_->AddChildView(button);
        }

    }
    layout->StartRow(0, 1);
    layout->AddView(buttons_bar_);

    layout->AddPaddingRow(0, kPaddingRowHeightEnds);
}

//标题添加图片功能
LivehimeMessageBox::LivehimeMessageBox(
    const std::shared_ptr<base::string16>& result,
    const base::string16& title,
    const base::string16& message,
    const gfx::ImageSkia* image_skia_title,
    const gfx::ImageSkia* image_skia,
    const base::string16& buttons,
    const livehime::MessageBoxEndDialogSignalHandler* handler/* = nullptr*/,
    gfx::HorizontalAlignment align/* = gfx::ALIGN_LEFT*/,
    const base::string16& def_button/* = L""*/,
    int button_padding/* = 0*/)
    : BililiveWidgetDelegate(*image_skia_title, title)
    , msgbox_result_(result)
    , image_view_(nullptr)
    , button_padding_(button_padding)
{
    if (handler)
    {
        msgbox_end_dialog_handler_ = *handler;
    }

    ResourceBundle& rb = ResourceBundle::GetSharedInstance();

    views::GridLayout* layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    views::ColumnSet* column_set = layout->AddColumnSet(0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    if (image_skia)
    {
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
    }
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 1.0f, views::GridLayout::FIXED, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

    // 最少保持足够显示三个button的宽度
    std::vector<base::string16> btns;
    base::SplitStringDontTrim(buttons, ',', &btns);
    int btn_num = std::max(3, static_cast<int>(btns.size()));

    int cx = (LivehimeFunctionLabelButton::GetButtonSize().width() + kPaddingColWidthForActionButton) * btn_num - kPaddingColWidthForActionButton;
    column_set = layout->AddColumnSet(1);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    column_set->AddPaddingColumn(1.0f, 0);
    column_set->AddColumn(views::GridLayout::CENTER, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, cx, cx);
    column_set->AddPaddingColumn(1.0f, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

    label_ = new LivehimeContentLabel(message);
    label_->SetVerticalAlignment(gfx::ALIGN_VCENTER);
    label_->SetHorizontalAlignment(align);
    label_->SetAllowCharacterBreak(true);
    label_->SetMultiLine(true);
    if (image_skia)
    {
        image_view_ = new views::ImageView();
        image_view_->SetImage(image_skia);
        label_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
    }

    layout->AddPaddingRow(0, kPaddingRowHeightEnds);

    layout->StartRow(1.0f, 0);
    if (image_view_)
    {
        layout->AddView(image_view_);
    }
    layout->AddView(label_);

    layout->AddPaddingRow(0, kPaddingRowHeightForDiffGroups);

    buttons_bar_ = new views::View();
    {
        if (button_padding_ == 0)
        {
            button_padding_ = kPaddingColWidthForActionButton;
        }
        views::BoxLayout* box_layout = new views::BoxLayout(views::BoxLayout::Orientation::kHorizontal, 0, 0, button_padding_);
        buttons_bar_->SetLayoutManager(box_layout);

        // 有指定默认ok按钮的就设置指定按钮，没有的就将第一个设为默认ok按钮
        bool is_set_def = false;

        for (auto var = btns.rbegin(); var != btns.rend(); ++var)
        {
            bool is_def = false;
            if (!def_button.empty())
            {
                if (*var == def_button)
                {
                    if (!is_set_def)
                    {
                        is_set_def = true;
                        is_def = true;
                    }
                    else
                    {
                        NOTREACHED() << "为什么要设置两个一样文案的按钮";
                    }
                }
            }
            else
            {
                if (var == --btns.rend())
                {
                    is_set_def = true;
                    is_def = true;
                }
            }
            LivehimeActionLabelButton* button = new LivehimeActionLabelButton(this, *var, is_def);
            button->SetIsDefault(is_def);
            buttons_bar_->AddChildView(button);
        }

    }
    layout->StartRow(0, 1);
    layout->AddView(buttons_bar_);

    layout->AddPaddingRow(0, kPaddingRowHeightEnds);
}

LivehimeMessageBox::~LivehimeMessageBox()
{
}

void LivehimeMessageBox::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    if (strcmp(sender->GetClassName(), LivehimeActionLabelButton::kViewClassName) == 0)
    {
        SetResultString(((LivehimeActionLabelButton*)sender)->GetText());
    }
    GetWidget()->Close();
}

void LivehimeMessageBox::SetResultString(const base::string16& result_string)
{
    *msgbox_result_ = result_string;
}

base::string16 LivehimeMessageBox::GetResultString()
{
    return *msgbox_result_;
}

void LivehimeMessageBox::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details)
{
    if (details.is_add && details.child == this)
    {
        DCHECK(GetWidget());
        open_msgbox_map_[hashkey_] = GetWidget()->GetNativeView();
    }
}

gfx::Size LivehimeMessageBox::GetPreferredSize()
{
    int img_width = 0;
    gfx::Size img_size;
    if (image_view_)
    {
        img_size = image_view_->GetPreferredSize();
        img_width = img_size.width() + kPaddingColWidthForGroupCtrls;
    }
    int label_width = kMessageBoxWidth - kPaddingColWidthEndsSmall * 2 - img_width;
    int label_height = label_->GetHeightForWidth(label_width);
    int mid_cy = std::max(label_height, img_size.height());

    int cy = kPaddingRowHeightEnds * 2 + kPaddingRowHeightForDiffGroups + mid_cy + buttons_bar_->GetPreferredSize().height();
    return gfx::Size(kMessageBoxWidth, cy);
}

void LivehimeMessageBox::OnWidgetDestroying(views::Widget* widget)
{
    // 需要进行动效渐隐的在渐隐完成后再通知模态结果
    bool invoke_end_handler = true;
    if (animate_ui_type_ != livehime::UIPropType::Unknown)
    {
        views::View* view = (views::View*)livehime::GetUIProp(animate_ui_type_);
        if (view)
        {
            invoke_end_handler = false;

            std::unique_ptr<gfx::Canvas> canvas(
                new gfx::Canvas(size(), ui::GetScaleFactorFromScale(ui::GetDPIScale()), true));
            Paint(canvas.get());
            gfx::ImageSkia img(canvas->ExtractImageRep());
        }
    }

    if (invoke_end_handler)
    {
        // 调用回调通知上层模态操作结果
        if (!msgbox_end_dialog_handler_.closure.is_null())
        {
            base::MessageLoop::current()->PostTask(
                FROM_HERE, base::Bind(LivehimeMessageBoxEndDialog,
                    msgbox_end_dialog_handler_, GetResultString()));
        }
    }

    open_msgbox_map_.erase(hashkey_);
    __super::OnWidgetDestroying(widget);
}


namespace livehime
{
    void ShowMessageBox(
        UniversalMsgboxType type,
        const MessageBoxEndDialogSignalHandler* handler/* = nullptr*/)
    {
        ui::ResourceBundle &rs = ui::ResourceBundle::GetSharedInstance();
        switch (type)
        {
        case UniversalMsgboxType::CannotRenameSceneItem:
            livehime::ShowMessageBox(GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget()->GetNativeView(),
                rs.GetLocalizedString(IDS_TIP_DLG_TIP),
                rs.GetLocalizedString(IDS_TIP_DLG_RENAME_ITEM),
                rs.GetLocalizedString(IDS_SURE),
                handler);
            break;
        default:
            NOTREACHED() << "not impl";
            break;
        }
    }

    void ShowMessageBox(
        gfx::NativeView parent_widget,
        const base::string16& title,
        const base::string16& message,
        const base::string16& buttons,
        const MessageBoxEndDialogSignalHandler* handler/* = nullptr*/,
        MessageBoxType mbt/* = MessageBoxType_NONEICON*/,
        const base::string16& def_button/* = L""*/,
        bool topmost/* = false*/,
        livehime::UIPropType animate_ui_type/* = UIPropType::Unknown*/,
        int button_padding /*= 0*/)
    {
        ResourceBundle &rb = ResourceBundle::GetSharedInstance();
        gfx::ImageSkia* image_skia = nullptr;
        switch (mbt)
        {
        case MessageBoxType_DEFAULT:
            image_skia = rb.GetImageSkiaNamed(IDR_SCENE_NOTICE_IMG);
            break;
        default:
            break;
        }
        ShowMessageBox(parent_widget, title, message, buttons, image_skia,
            handler, gfx::ALIGN_CENTER, def_button, topmost, animate_ui_type, button_padding);
    }

    void ShowMessageBox(
        gfx::NativeView parent_widget,
        const base::string16& title,
        const base::string16& message,
        const base::string16& buttons,
        const gfx::ImageSkia* image_skia,
        const MessageBoxEndDialogSignalHandler* handler/* = nullptr*/,
        gfx::HorizontalAlignment align/* = gfx::ALIGN_LEFT*/,
        const base::string16& def_button/* = L""*/,
        bool topmost/* = false*/,
        livehime::UIPropType animate_ui_type/* = UIPropType::Unknown*/,
        int button_padding /*= 0*/)
    {
        DCHECK(!buttons.empty());

        std::string hashstr = ComputesMessageBoxHashKey(title, message, buttons);
        // 已经弹了同样的messagebox了就不允许再弹了
        if (LivehimeMessageBox::FindMessageBoxAndFlash(hashstr))
        {
            return;
        }

        std::shared_ptr<base::string16> result = std::make_shared<base::string16>(L"");

        LivehimeMessageBox *msgbox = new LivehimeMessageBox(
            result, title, message, image_skia, buttons, handler, align, def_button, button_padding);

        msgbox->set_hashkey(hashstr);
        msgbox->set_animate_ui_type(animate_ui_type);

        views::Widget *widget_ = new views::Widget();
        BililiveNativeWidgetWin *native_widget = new BililiveNativeWidgetWin(widget_);
        native_widget->set_escape_operate(BililiveNativeWidgetWin::WO_CLOSE);
        views::Widget::InitParams params;
        params.native_widget = native_widget;
        params.parent = parent_widget;
        params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
        params.keep_on_top = topmost;
        BililiveWidgetDelegate::DoModalWidget(msgbox, widget_, params);
    }

    void ShowMessageBox(
        gfx::NativeView parent_widget,
        const base::string16& title,
        const base::string16& message,
        const base::string16& buttons,
        const gfx::ImageSkia* image_skia_title,
        const gfx::ImageSkia* image_skia,
        const MessageBoxEndDialogSignalHandler* handler/* = nullptr*/,
        gfx::HorizontalAlignment align/* = gfx::ALIGN_LEFT*/,
        const base::string16& def_button/* = L""*/,
        bool topmost/* = false*/,
        livehime::UIPropType animate_ui_type/* = UIPropType::Unknown*/,
        int button_padding /*= 0*/)
    {
        DCHECK(!buttons.empty());

        std::string hashstr = ComputesMessageBoxHashKey(title, message, buttons);
        // 已经弹了同样的messagebox了就不允许再弹了
        if (LivehimeMessageBox::FindMessageBoxAndFlash(hashstr))
        {
            return;
        }

        std::shared_ptr<base::string16> result = std::make_shared<base::string16>(L"");

        LivehimeMessageBox* msgbox = new LivehimeMessageBox(
            result, title, message, image_skia_title, image_skia, buttons, handler, align, def_button, button_padding);

        msgbox->set_hashkey(hashstr);
        msgbox->set_animate_ui_type(animate_ui_type);

        views::Widget* widget_ = new views::Widget();
        BililiveNativeWidgetWin* native_widget = new BililiveNativeWidgetWin(widget_);
        native_widget->set_escape_operate(BililiveNativeWidgetWin::WO_CLOSE);
        views::Widget::InitParams params;
        params.native_widget = native_widget;
        params.parent = parent_widget;
        params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
        params.keep_on_top = topmost;
        BililiveWidgetDelegate::DoModalWidget(msgbox, widget_, params);
    }


    // 真模态信息弹窗
    base::string16 ShowModalMessageBox(
        gfx::NativeView parent_widget,
        const base::string16& title,
        const base::string16& message,
        const base::string16& buttons,
        MessageBoxType mbt /*= MessageBoxType_NONEICON*/)
    {
        ResourceBundle &rb = ResourceBundle::GetSharedInstance();
        gfx::ImageSkia* image_skia = nullptr;
        switch (mbt)
        {
        case MessageBoxType_DEFAULT:
            image_skia = rb.GetImageSkiaNamed(IDR_SCENE_NOTICE_IMG);
            break;
        default:
            break;
        }
        return ShowModalMessageBox(parent_widget, title, message, buttons, image_skia, gfx::ALIGN_CENTER);
    }

    base::string16 ShowModalMessageBox(
        gfx::NativeView parent_widget,
        const base::string16& title,
        const base::string16& message,
        const base::string16& buttons,
        const gfx::ImageSkia* image_skia,
        gfx::HorizontalAlignment align/* = gfx::ALIGN_LEFT*/)
    {
        DCHECK(!buttons.empty());

        std::shared_ptr<base::string16> result = std::make_shared<base::string16>(L"");

        std::string hashstr = ComputesMessageBoxHashKey(title, message, buttons);
        // 已经弹了同样的messagebox了就不允许再弹了
        if (LivehimeMessageBox::FindMessageBoxAndFlash(hashstr))
        {
            return *result;
        }

        LivehimeMessageBox *msgbox = new LivehimeMessageBox(result, title, message, image_skia, buttons,
                                                            nullptr, align);
        msgbox->set_hashkey(hashstr);

        views::Widget *widget_ = new views::Widget();
        BililiveNativeWidgetWin *native_widget = new BililiveNativeWidgetWin(widget_);
        native_widget->set_escape_operate(BililiveNativeWidgetWin::WO_CLOSE);
        views::Widget::InitParams params;
        params.native_widget = native_widget;
        params.parent = parent_widget;
        params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
        int ret = BililiveWidgetDelegate::RealDoModalWidget(msgbox, widget_, params);
        return *result;
    }

    void ShowGeneralMessageBox(
        gfx::NativeView parent_widget,
        const base::string16& title,
        const base::string16& message,
        const GeneralMsgBoxEndDialogSignalHandler* handler /*= nullptr*/,
        const base::string16& type /*= L"confirm"*/,
        const base::string16& confirm_button /*= L"确定"*/,
        const base::string16& cancel_button /*= L"取消"*/,
        const std::string& noRemindKey /*= L""*/,
        const base::string16& def_button /*= L""*/,
        bool topmost /*= false*/,
        livehime::UIPropType animate_ui_type /*= UIPropType::Unknown*/)
    {
        base::string16 confirm_btn = confirm_button;
        base::string16 cancel_btn = cancel_button;
        base::string16 def_btn = def_button;
        std::string key = noRemindKey;
        if (type == L"alert") {
            if (confirm_button.empty()) {
                if (cancel_button.empty()) {
                    confirm_btn = ResourceBundle::GetSharedInstance().GetLocalizedString(IDS_IKNOW);
                }
                else
                    confirm_btn = cancel_button;
            }
            cancel_btn = L"";
            key = "";
            def_btn = L"";
        }
        else {
            if (cancel_btn.empty())
                cancel_btn = ResourceBundle::GetSharedInstance().GetLocalizedString(IDS_CANCEL);
            if (confirm_btn.empty())
                confirm_btn = ResourceBundle::GetSharedInstance().GetLocalizedString(IDS_SURE);
            DCHECK(cancel_btn != confirm_btn);
        }            
        base::string16 btns = cancel_btn + confirm_btn;
        DCHECK(!btns.empty());
        std::string hashstr = ComputesMessageBoxHashKey(title, message, btns);
        // 已经弹了同样的messagebox了就不允许再弹了
        if (LivehimeMessageBox::FindMessageBoxAndFlash(hashstr))
        {
            return;
        }

        std::shared_ptr<GeneralMsgBoxRes> result = std::make_shared<GeneralMsgBoxRes>( L"" ,L"-1");
        const int max_width = kGeneralMsgBoxWidth - kMsgBoxPadding * 2;
        const int line_max_num = max_width / LivehimePaddingCharWidth(ftFourteen);
        views::Label* label = nullptr;
        LivehimeGeneralMessageBox* msgbox = nullptr;
        const int len = message.length();
        if (len > line_max_num) {
            base::string16 text;
            for (int i = 0, count = 0; i < len; ++i) {
                text.push_back(message.at(i));
                if (++count > line_max_num) {
                    text.push_back('\n');
                    count = 0;
                }
                if (message.at(i) == '\n') {
                    count = 0;
                }
            }
            msgbox = new LivehimeGeneralMessageBox(
                result, title, text, type, confirm_btn, cancel_btn, handler, key, def_btn);
        }
        else {
            msgbox = new LivehimeGeneralMessageBox(
                result, title, message, type, confirm_btn, cancel_btn, handler, key, def_btn);
        }
        msgbox->set_hashkey(hashstr);
        msgbox->set_animate_ui_type(animate_ui_type);

        views::Widget* widget_ = new views::Widget();
        BililiveNativeWidgetWin* native_widget = new BililiveNativeWidgetWin(widget_);
        native_widget->set_escape_operate(BililiveNativeWidgetWin::WO_CLOSE);
        views::Widget::InitParams params;
        params.native_widget = native_widget;
        params.parent = parent_widget;
        params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
        params.keep_on_top = topmost;
        BililiveWidgetDelegate::DoModalWidget(msgbox, widget_, params);
    }

}


std::map<std::string, gfx::NativeView> LivehimeGeneralMessageBox::open_msgbox_map_;

LivehimeGeneralMessageBox::LivehimeGeneralMessageBox(
    const std::shared_ptr<GeneralMsgBoxRes>& result,
    const base::string16& title,
    const base::string16& message,
    const base::string16& type /*= confirm*/,
    const base::string16& confirm_button /*= L"确定"*/,
    const base::string16& cancel_button /*= L"取消"*/,
    const livehime::GeneralMsgBoxEndDialogSignalHandler* handler /*= nullptr*/,
    const std::string& noRemindKey /*= ""*/,
    const base::string16& def_button /*= L""*/)
    : BililiveWidgetDelegate(gfx::ImageSkia(), title),
    msgbox_result_(result),
    title_(title),
    noRemindKey_(noRemindKey),
    has_never_notify_(!noRemindKey.empty()),
    btn_close_show_(type != L"alert")
{
    if (handler)
    {
        msgbox_end_dialog_handler_ = *handler;
    }
    ResourceBundle& rb = ResourceBundle::GetSharedInstance();

    views::GridLayout* layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    views::ColumnSet* column_set = layout->AddColumnSet(0);
    column_set->AddPaddingColumn(0, kMsgBoxPadding);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kMsgBoxPadding);

    label_ = new views::Label(message);
    label_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
    label_->SetAllowCharacterBreak(true);
    label_->SetMultiLine(true);
    label_->SetFont(ftFourteen);
    label_->SetEnabledColor(SkColorSetRGB(0x75, 0x85, 0x92));

    layout->StartRow(0, 0);
    layout->AddView(label_);
    layout->AddPaddingRow(0, kMsgBoxPaddingSmall);
    if (has_never_notify_) {
        column_set = layout->AddColumnSet(1);
        column_set->AddPaddingColumn(1.0f, 0);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddPaddingColumn(1.0f, 0);
        checkbox_ = new LivehimeCheckbox(UIViewsStyle::Style_Model_MsgBox, L"下次不再提醒");
        checkbox_->set_focusable(false);
        layout->StartRow(0, 1);
        layout->AddView(checkbox_);
    }
    layout->AddPaddingRow(0, kMsgBoxPaddingSmall);

    views::View* bottom_view = new views::View(); {
        views::GridLayout* layout = new views::GridLayout(bottom_view);
        bottom_view->SetLayoutManager(layout);
        views::ColumnSet* column_set = layout->AddColumnSet(0);
        column_set->AddPaddingColumn(1.0f, 0);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddPaddingColumn(1.0f, 0);
        buttons_bar_ = new views::View(); {
            views::BoxLayout* box_layout = new views::BoxLayout(views::BoxLayout::Orientation::kHorizontal, 0, 0, kPaddingColWidthForActionButton);
            buttons_bar_->SetLayoutManager(box_layout);
            // 有指定默认ok按钮的就设置指定按钮，没有的就将第二个设为默认ok按钮
            bool is_set_def = false;
            bool is_btn1_def = false;
            if (!def_button.empty())
            {
                if (cancel_button == def_button)
                {
                    is_set_def = true;
                    is_btn1_def = true;
                }
                else if (confirm_button == def_button) {
                    is_set_def = true;
                    is_btn1_def = false;
                }
            }
            if (!is_set_def && confirm_button.empty())
            {
                is_btn1_def = true;
            }
            if (!cancel_button.empty()) {
                LivehimeGeneralMsgBoxButton* cancel_btn = new LivehimeGeneralMsgBoxButton(this, cancel_button, is_btn1_def);
                cancel_btn->SetIsDefault(is_btn1_def);
                buttons_bar_->AddChildView(cancel_btn);
            }
            LivehimeGeneralMsgBoxButton* confirm_btn = new LivehimeGeneralMsgBoxButton(this, confirm_button, !is_btn1_def);
            confirm_btn->SetIsDefault(!is_btn1_def);
            buttons_bar_->AddChildView(confirm_btn);
        }
        layout->StartRowWithPadding(0, 0, 0, kMsgBoxPaddingSmall);
        layout->AddView(buttons_bar_);
    }
    bottom_view->set_border(views::Border::CreateSolidSidedBorder(1, 0, 0, 0, SkColorSetARGB(0.1f * 255, 0, 0, 0)));
    column_set = layout->AddColumnSet(2);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 1.0f, views::GridLayout::USE_PREF, 0, 0);
    layout->StartRow(0, 2);
    layout->AddView(bottom_view);
    layout->AddPaddingRow(0, kMsgBoxPaddingSmall);
}

bool LivehimeGeneralMessageBox::FindMessageBoxAndFlash(const std::string& hashkey)
{
    if (open_msgbox_map_.find(hashkey) != open_msgbox_map_.end())
    {
        BililiveWidgetDelegate::FlashModalWidget(open_msgbox_map_[hashkey]);
        return true;
    }
    return false;
}

void LivehimeGeneralMessageBox::OnCreateNonClientFrameView(views::NonClientFrameView* non_client_frame_view)
{
    BililiveFrameView* frame = reinterpret_cast<BililiveFrameView*>(non_client_frame_view);
    GeneralMsgBoxTitleBarView* title_bar = new GeneralMsgBoxTitleBarView(title_);
    title_bar->SetCloseBtnVisible(btn_close_show_);
    frame->SetTitlebar(title_bar);
    frame->SetBackgroundColor(SkColorSetRGB(255, 255, 255));
}

void LivehimeGeneralMessageBox::OnWidgetDestroying(views::Widget* widget)
{
    // 需要进行动效渐隐的在渐隐完成后再通知模态结果
    bool invoke_end_handler = true;
    if (animate_ui_type_ != livehime::UIPropType::Unknown)
    {
        views::View* view = (views::View*)livehime::GetUIProp(animate_ui_type_);
        if (view)
        {
            invoke_end_handler = false;

            std::unique_ptr<gfx::Canvas> canvas(
                new gfx::Canvas(size(), ui::GetScaleFactorFromScale(ui::GetDPIScale()), true));
            Paint(canvas.get());
            gfx::ImageSkia img(canvas->ExtractImageRep());
            OnCloseBtnClick();
        }
    }

    if (invoke_end_handler)
    {
        // 调用回调通知上层模态操作结果
        if (!msgbox_end_dialog_handler_.closure.is_null())
        {
            OnCloseBtnClick();
            base::MessageLoop::current()->PostTask(
                FROM_HERE, base::Bind(LivehimeGeneralMsgBoxEndDialog,
                    msgbox_end_dialog_handler_, GetResultString()));
        }
    }
    open_msgbox_map_.erase(hashkey_);
    __super::OnWidgetDestroying(widget);
}

void LivehimeGeneralMessageBox::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details)
{
    if (details.is_add && details.child == this)
    {
        DCHECK(GetWidget());
        open_msgbox_map_[hashkey_] = GetWidget()->GetNativeView();
    }
}

gfx::Size LivehimeGeneralMessageBox::GetPreferredSize()
{
    int label_height = label_->GetPreferredSize().height();
    int checkbox_height = checkbox_ ? checkbox_->GetPreferredSize().height() : 0;
    int cy = kMsgBoxPadding * 2 + label_height + checkbox_height + buttons_bar_->GetPreferredSize().height();
    return gfx::Size(kGeneralMsgBoxWidth, cy);
}

void LivehimeGeneralMessageBox::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    if (strcmp(sender->GetClassName(), LivehimeGeneralMsgBoxButton::kViewClassName) == 0)
    {
        SetResultString(true, ((LivehimeGeneralMsgBoxButton*)sender)->GetText());
        if (has_never_notify_)
        {
            if (checkbox_->checked()) {
                base::string16 val = L"1";
                SetResultString(false, val);
                CefHybridCache::GetInstance()->AddHybridCacheByKey(noRemindKey_, val);
            }
            else {
                base::string16 val = L"0";
                SetResultString(false, val);
            }
        }
    }
    GetWidget()->Close();
}

GeneralMsgBoxRes LivehimeGeneralMessageBox::GetResultString()
{
    return *msgbox_result_;
}

void LivehimeGeneralMessageBox::SetResultString(bool first, const base::string16& result_string)
{
    if (first)
        (*msgbox_result_).first = result_string;
    else
        (*msgbox_result_).second = result_string;
}

void LivehimeGeneralMessageBox::OnCloseBtnClick()
{
    if (GetResultString().first.empty()) {
        base::string16 result = L"关闭";
        SetResultString(true, result);
        if (has_never_notify_)
        {
            if (checkbox_->checked()) {
                base::string16 val = L"1";
                SetResultString(false, val);
                CefHybridCache::GetInstance()->AddHybridCacheByKey(noRemindKey_, val);
            }
            else {
                base::string16 val = L"0";
                SetResultString(false, val);
            }
        }
    }
}
