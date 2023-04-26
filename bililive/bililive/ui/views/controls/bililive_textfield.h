
#ifndef BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_TEXTFIELD_H
#define BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_TEXTFIELD_H

#include "base/basictypes.h"

#include "bililive/bililive/ui/views/controls/bililive_theme_common.h"

#include "ui/base/events/event.h"
#include "ui/views/focus/focus_manager.h"
#include "ui/views/controls/textfield/textfield.h"
#include "ui/views/controls/textfield/textfield_controller.h"
#include "ui/views/controls/button/image_button.h"


class BililiveTextfield
    : public views::View
    , public views::FocusChangeListener
{
public:
    enum CooperateDirection
    {
        BD_NONE,
        BD_RIGHT,
        BD_LEFT,
    };

    explicit BililiveTextfield(views::Textfield::StyleFlags style = views::Textfield::STYLE_DEFAULT);
    explicit BililiveTextfield(views::View *view, CooperateDirection dir = BD_RIGHT, views::Textfield::StyleFlags style = views::Textfield::STYLE_DEFAULT);
    virtual ~BililiveTextfield();

    void SetHorizontalMargins(int left, int right);
    void SetBorder(bool has_border);
    void SetBorderColor(SkColor clrBlur, SkColor clrFocus);
    void SetBackgroundColor(SkColor color);

    views::Textfield* text_field() const { return textfield_; }

    // Textfield
    void SetFont(const gfx::Font &font);
    void SetTextColor(SkColor color);
    void SetController(views::TextfieldController* controller);
    void set_placeholder_text(const string16& text);
    void set_placeholder_text_color(SkColor color);

    // View
    void Layout() override;
    gfx::Size GetPreferredSize() override;
protected:
    // View
    void OnPaintBackground(gfx::Canvas* canvas) override;
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;
    void OnEnabledChanged() override;
    void OnMouseEntered(const ui::MouseEvent& event) override;
    void OnMouseExited(const ui::MouseEvent& event) override;

    // FocusChangeListener
    void OnWillChangeFocus(View* focused_before, View* focused_now) override{}
    void OnDidChangeFocus(View* focused_before, View* focused_now) override;

    void SetTextfield(views::Textfield* textfield);

private:
    void InitParam();
    void AddFocusChangeListener();
    void RemoveFocusChangeListener();

protected:
    views::Textfield *textfield_;
    views::View *cooperate_view_;
    views::Textfield::StyleFlags textfield_style_;
    CooperateDirection cooperate_dir_;

private:
    bool add_focus_listener_;
    bool has_border_;
    SkColor focus_border_color_;
    SkColor blur_border_color_;
    gfx::Insets margins_;
    int left_inset_;
    int right_inset_;

    DISALLOW_COPY_AND_ASSIGN(BililiveTextfield);
};

// 快捷键设置完成后回调
class AcceleratorsTextfield;
class AcceleratorsTextfieldDelegate
{
public:
    virtual void OnHotkeyChange(AcceleratorsTextfield* sender, UINT &nModify, UINT &nVk) = 0;
};

// 快捷键编辑框
// 字母、数字、Insert、Home等必须使用组合键，F1~F12功能键可以单设
class AcceleratorsTextfield : public views::Textfield
{
public:
    AcceleratorsTextfield();
    explicit AcceleratorsTextfield(views::Textfield::StyleFlags style);
    virtual ~AcceleratorsTextfield();

    bool GetHotkey(UINT &nModify, UINT &nVk);
    void SetHotkey(UINT nModify, UINT nVk);
    void DelHotKey();

    // 检查键值是否为有效的字母键
    static bool CheckLetterKey(unsigned char kc, base::string16 *pstr = NULL);
    // 检查键值是否为F1~F12功能键
    static bool CheckFunctionKey(unsigned char kc, base::string16 *pstr = NULL);
    // 获取快捷键字串
    static base::string16 GetAcceleratorsString(bool ctrl, bool shift, bool alt, unsigned char vk);

    void SetAcceleratorsTextfieldDelegate(AcceleratorsTextfieldDelegate *delegate){ delegate_ = delegate; }

protected:
    // Overridden from EventHandler:
    virtual void OnKeyEvent(ui::KeyEvent* event) override;

    void initMember();
    void ResetSettingKeys();

private:
    AcceleratorsTextfieldDelegate *delegate_;

    // 设置中的组合键
    bool m_bCtrl;
    bool m_bShift;
    bool m_bAlt;
    unsigned char m_cChar;

    // 设置完毕的组合键（以字母键、功能键的释放作为设置完毕的时机）
    bool m_bValidSetup;	// 设置是否有效
    bool m_bRealCtrl;
    bool m_bRealShift;
    bool m_bRealAlt;
    unsigned char m_cRealChar;

    DISALLOW_COPY_AND_ASSIGN(AcceleratorsTextfield);
};


#endif  // BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_COMBOX_BILILIVE_COMBOBOX_H
