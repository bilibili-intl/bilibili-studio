#ifndef LIVEHIME_MESSAGE_BOX_H
#define LIVEHIME_MESSAGE_BOX_H

#include "bililive/bililive/livehime/notify_ui_control/notify_ui_prop.h"
#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"

#include "ui/gfx/text_constants.h"
#include "ui/views/controls/image_view.h"

namespace livehime
{
    // 模态消息框关闭时的执行结果回调，点按的按钮的文本
    typedef base::Callback<void(const base::string16&, void* data)> MessageBoxEndDialogClosure;
    typedef base::Callback<void(const std::pair<base::string16, base::string16>&, void* data)> GeneralMsgBoxEndDialogClosure;
    // 模态弹窗回调参数结构
    struct MessageBoxEndDialogSignalHandler
    {
        MessageBoxEndDialogClosure closure;
        void* data = nullptr;
    };
    // 通用对话框回调参数结构
    struct GeneralMsgBoxEndDialogSignalHandler
    {
        GeneralMsgBoxEndDialogClosure closure;
        void* data = nullptr;
    };

    // 程序通用的信息提示框，提示信息、图标、按钮都固定
    enum class UniversalMsgboxType
    {
        CannotRenameSceneItem,
    };

    void ShowMessageBox(UniversalMsgboxType type,
        const MessageBoxEndDialogSignalHandler* handler = nullptr);

    enum MessageBoxType
    {
        MessageBoxType_NONEICON,
        MessageBoxType_DEFAULT,
    };

    // 通常调的这个
    void ShowMessageBox(
        gfx::NativeView parent_widget,
        const base::string16& title,
        const base::string16& message,
        const base::string16& buttons,
        const MessageBoxEndDialogSignalHandler* handler = nullptr,
        MessageBoxType mbt = MessageBoxType_NONEICON,
        const base::string16& def_button = L"",
        bool topmost = false,
        UIPropType animate_ui_type = UIPropType::Unknown,
        int button_padding = 0);

    void ShowMessageBox(
        gfx::NativeView parent_widget,
        const base::string16& title,
        const base::string16& message,
        const base::string16& buttons,
        const gfx::ImageSkia* image_skia,
        const MessageBoxEndDialogSignalHandler* handler = nullptr,
        gfx::HorizontalAlignment align = gfx::ALIGN_LEFT,
        const base::string16& def_button = L"",
        bool topmost = false,
        UIPropType animate_ui_type = UIPropType::Unknown,
        int button_padding = 0);

    void ShowMessageBox(
        gfx::NativeView parent_widget,
        const base::string16& title,
        const base::string16& message,
        const base::string16& buttons,
        const gfx::ImageSkia* image_skia_title,
        const gfx::ImageSkia* image_skia,
        const MessageBoxEndDialogSignalHandler* handler = nullptr,
        gfx::HorizontalAlignment align = gfx::ALIGN_LEFT,
        const base::string16& def_button = L"",
        bool topmost = false,
        UIPropType animate_ui_type = UIPropType::Unknown,
        int button_padding = 0);


    // 真模态信息框
    base::string16 ShowModalMessageBox(
        gfx::NativeView parent_widget,
        const base::string16& title,
        const base::string16& message,
        const base::string16& buttons,
        MessageBoxType mbt = MessageBoxType_NONEICON);

    base::string16 ShowModalMessageBox(
        gfx::NativeView parent_widget,
        const base::string16& title,
        const base::string16& message,
        const base::string16& buttons,
        const gfx::ImageSkia* image_skia,
        gfx::HorizontalAlignment align = gfx::ALIGN_LEFT);

    void ShowGeneralMessageBox(
        gfx::NativeView parent_widget,
        const base::string16& title,
        const base::string16& message,
        const GeneralMsgBoxEndDialogSignalHandler* handler = nullptr,
        const base::string16& type = L"confirm",  //type类型为"confirm|alert"
        const base::string16& confirm_button = L"",
        const base::string16& cancel_button = L"",
        const std::string& noRemindKey = "",
        const base::string16& def_button = L"",
        bool topmost = false,
        livehime::UIPropType animate_ui_type = UIPropType::Unknown);
}

class LivehimeMessageBox
    : public BililiveWidgetDelegate
    , public views::ButtonListener
{
public:
    LivehimeMessageBox(
        const std::shared_ptr<base::string16>& result,
        const base::string16& title,
        const base::string16& message,
        const gfx::ImageSkia* image_skia,
        const base::string16& buttons,
        const livehime::MessageBoxEndDialogSignalHandler* handler = nullptr,
        gfx::HorizontalAlignment align = gfx::ALIGN_LEFT,
        const base::string16& def_button = L"",
        int button_padding = 0);

    LivehimeMessageBox(
        const std::shared_ptr<base::string16>& result,
        const base::string16& title,
        const base::string16& message,
        const gfx::ImageSkia* image_skia_title,
        const gfx::ImageSkia* image_skia,
        const base::string16& buttons,
        const livehime::MessageBoxEndDialogSignalHandler* handler = nullptr,
        gfx::HorizontalAlignment align = gfx::ALIGN_LEFT,
        const base::string16& def_button = L"",
        int button_padding = 0);

    virtual ~LivehimeMessageBox();

    void set_hashkey(const std::string &hashkey) { hashkey_ = hashkey; }
    static bool FindMessageBoxAndFlash(const std::string &hashkey);

    void set_animate_ui_type(livehime::UIPropType animate_ui_type) { animate_ui_type_ = animate_ui_type; }

protected:
    // WidgetDelegate
    ui::ModalType GetModalType() const override{ return ui::MODAL_TYPE_WINDOW; }
    views::View* GetContentsView() override { return this; }

    // WidgetObserver
    void OnWidgetDestroying(views::Widget* widget) override;

    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;
    gfx::Size GetPreferredSize() override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    base::string16 GetResultString();
    void SetResultString(const base::string16& result_string);

private:
    int button_padding_ = 0;
    std::shared_ptr<base::string16> msgbox_result_;
    views::ImageView *image_view_;
    LivehimeContentLabel *label_;
    views::View *buttons_bar_;

    std::string hashkey_;
    static std::map<std::string, gfx::NativeView> open_msgbox_map_;

    livehime::UIPropType animate_ui_type_ = livehime::UIPropType::Unknown;

    livehime::MessageBoxEndDialogSignalHandler msgbox_end_dialog_handler_;

    DISALLOW_COPY_AND_ASSIGN(LivehimeMessageBox);
};

namespace {
    int kMsgBoxPadding = GetLengthByDPIScale(16);
    int kMsgBoxPaddingSmall = GetLengthByDPIScale(8);
    int kGeneralMsgBoxWidth = GetLengthByDPIScale(400);
}

typedef std::pair<base::string16, base::string16> GeneralMsgBoxRes;
class LivehimeGeneralMessageBox
    : public BililiveWidgetDelegate
    , public views::ButtonListener
{
public:
    LivehimeGeneralMessageBox(
        const std::shared_ptr<GeneralMsgBoxRes>& result,
        const base::string16& title,
        const base::string16& message,
        const base::string16& type = L"confirm",
        const base::string16& confirm_button = L"",
        const base::string16& cancel_button = L"",
        const livehime::GeneralMsgBoxEndDialogSignalHandler* handler = nullptr,
        const std::string & noRemindKey = "",
        const base::string16& def_button = L"");
    virtual ~LivehimeGeneralMessageBox() {};

    void set_hashkey(const std::string& hashkey) { hashkey_ = hashkey; }
    static bool FindMessageBoxAndFlash(const std::string& hashkey);
    void set_animate_ui_type(livehime::UIPropType animate_ui_type) { animate_ui_type_ = animate_ui_type; }

protected:
    // WidgetDelegate
    ui::ModalType GetModalType() const override { return ui::MODAL_TYPE_WINDOW; }
    views::View* GetContentsView() override { return this; }
    void OnCreateNonClientFrameView(views::NonClientFrameView* non_client_frame_view) override;
    // WidgetObserver
    void OnWidgetDestroying(views::Widget* widget) override;

    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;
    gfx::Size GetPreferredSize() override;
    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    GeneralMsgBoxRes GetResultString();
    void SetResultString(bool first, const base::string16& result_string);
    void OnCloseBtnClick();
private:
    std::shared_ptr<GeneralMsgBoxRes> msgbox_result_;
    views::Label* label_;
    views::View* buttons_bar_;
    LivehimeCheckbox* checkbox_ = nullptr;
    base::string16 title_;
    std::string hashkey_;
    bool btn_close_show_ = true;
    bool has_never_notify_;
    std::string noRemindKey_;
    static std::map<std::string, gfx::NativeView> open_msgbox_map_;
    livehime::UIPropType animate_ui_type_ = livehime::UIPropType::Unknown;

    livehime::GeneralMsgBoxEndDialogSignalHandler msgbox_end_dialog_handler_;

    DISALLOW_COPY_AND_ASSIGN(LivehimeGeneralMessageBox);
};
#endif
