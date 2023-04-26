#ifndef NATIVE_EDIT_VIEW_H_
#define NATIVE_EDIT_VIEW_H_

#include <string>

#include "ui/views/view.h"
#include "ui/views/controls/native_control.h"
#include "ui/views/controls/button/button.h"

#include "ui/gfx/text_constants.h"
#include "ui/gfx/font.h"

class BilibiliNativeEditView;
class BilibiliNativeEditController
{
public:
    virtual ~BilibiliNativeEditController() = default;

    virtual void ContentsChanged(BilibiliNativeEditView* sender, const string16& new_contents) {}
    // 处理过了就返回false，否则true让窗口过程继续执行
    virtual bool PreHandleMSG(BilibiliNativeEditView* sender, UINT msg, WPARAM wParam, LPARAM lParam) { return true; }
};

class BilibiliNativeEditView : public views::NativeControl
{
public:
    enum CooperateDirection
    {
        BD_NONE,
        BD_RIGHT,
        BD_LEFT,
        BD_BOTTOM,
        BD_CORNER,
    };

    enum EnChangeReason
    {
        ECR_API,
        ECR_DEVICE,
    };

    enum StyleFlags
    {
        STYLE_DEFAULT = 0,
        STYLE_OBSCURED = 1 << 0,
        //STYLE_LOWERCASE = 1 << 1
    };

    explicit BilibiliNativeEditView(
        views::View *cooperate_view, CooperateDirection dir = BD_RIGHT, StyleFlags style = STYLE_DEFAULT);
    explicit BilibiliNativeEditView(StyleFlags style = STYLE_DEFAULT);
    virtual ~BilibiliNativeEditView();

    void SetBorderColor(SkColor clrBlur, SkColor clrFocus);
    const gfx::Font& font() const { return font_; }
    bool IsFocus() const;
    void SetMultiLine(bool multi_line);
    void SetMultiLine(bool multi_line, bool v_scroll, bool auto_vscroll);
    string16 GetText() const { return text(); }
    bool IsNumberOnly() const { return number_only_; }
    bool IsEmptyText() const { return is_empty_text_; }
    void SetNumberOnly(bool number_only, bool allow_minus = false, bool allow_decimals = false);// 不能控制负数和小数
    void SetSkipDefaultKeyEventProcessing(bool accept_return);
    void set_default_width_in_chars(int default_width, const base::string16 &padding_char);
    void GetHorizontalMargins(int &left, int &right) const;
    void SetHorizontalAlignment(gfx::HorizontalAlignment alignment){ h_align_ = alignment; }
    void SetResetTypeInView(views::View *reset_view);
    void SetPasswordReplacementChar(wchar_t obscured_ochar) { obscured_char_ = obscured_ochar; }

    // START -> 兼容views::Textfield
    void SetController(BilibiliNativeEditController* controller){ controller_ = controller; }
    BilibiliNativeEditController* GetController() const{ return controller_; }
    bool read_only() const;
    void SetReadOnly(bool read_only);
    bool IsObscured() const { return obscured_; }
    void SetObscured(bool obscured);
    string16 text() const;
    void SetText(const string16& text, EnChangeReason reason = EnChangeReason::ECR_API);
    void AppendText(const string16& text);
    void InsertOrReplaceText(const string16& text);
    //string16 GetSelectedText() const;
    void SelectAll(/*bool reversed*/);
    void ClearSelection();
    bool HasSelection() const;
    SkColor GetTextColor() const{ return text_color_; }
    void SetTextColor(SkColor color);
    SkColor GetBackgroundColor() const{ return background_color_; }
    void SetBackgroundColor(SkColor color);
    void SetFont(const gfx::Font& font);
    void SetHorizontalMargins(int left, int right);
    void SetVerticalMargins(int top, int bottom);
    gfx::VerticalAlignment vertical_alignment() const { return vertical_alignment_; }
    void SetVerticalAlignment(gfx::VerticalAlignment alignment);
    void set_default_width_in_chars(int default_width) { default_width_in_chars_ = default_width; }
    bool draw_border() const { return draw_border_; }
    void RemoveBorder();
    void set_placeholder_text(const string16& text) { placeholder_text_ = text; }
    const string16& placeholder_text() const { return placeholder_text_; }
    SkColor placeholder_text_color() const { return placeholder_text_color_; }
    void set_placeholder_text_color(SkColor color) { placeholder_text_color_ = color; }
    void SelectRange(DWORD dwSelection);
    void SetRadius(int radius) { radius_ = radius; };
    void SetExLayeredEnable(bool enable,int opacity);
    void SetFocus();
    void SetDrawBackgroundEnable(bool enable) { is_draw_background_enable_ = enable; };
    // END -> 兼容views::Textfield

    // START -> MFC CEdit 用法
    BOOL SetReadOnlyRaw(BOOL bReadOnly = TRUE)
    {
        DCHECK(::IsWindow(m_hWnd));
        return (BOOL)::SendMessage(m_hWnd, EM_SETREADONLY, bReadOnly, 0L);
    }

    void SetSel(DWORD dwSelection, BOOL bNoScroll = FALSE)
    {
        DCHECK(::IsWindow(m_hWnd));
        ::SendMessage(m_hWnd, EM_SETSEL, LOWORD(dwSelection), HIWORD(dwSelection));
        if (!bNoScroll)
            ::SendMessage(m_hWnd, EM_SCROLLCARET, 0, 0L);
    }

    void SetSel(int nStartChar, int nEndChar, BOOL bNoScroll = FALSE)
    {
        DCHECK(::IsWindow(m_hWnd));
        ::SendMessage(m_hWnd, EM_SETSEL, nStartChar, nEndChar);
        if (!bNoScroll)
            ::SendMessage(m_hWnd, EM_SCROLLCARET, 0, 0L);
    }

    void LimitText(int nChars = 0)
    {
        limit_number_ = nChars;
        if (::IsWindow(m_hWnd))
        {
            ::SendMessage(m_hWnd, EM_LIMITTEXT, nChars, 0L);
        }
    }

    void ReplaceSel(LPCTSTR lpszNewText, BOOL bCanUndo = FALSE)
    {
        DCHECK(::IsWindow(m_hWnd));
        ::SendMessage(m_hWnd, EM_REPLACESEL, (WPARAM)bCanUndo, (LPARAM)lpszNewText);
    }

    void InsertText(int nInsertAfterChar, LPCTSTR lpstrText, BOOL bNoScroll = FALSE, BOOL bCanUndo = FALSE)
    {
        SetSel(nInsertAfterChar, nInsertAfterChar, bNoScroll);
        ReplaceSel(lpstrText, bCanUndo);
    }

    void AppendText(LPCTSTR lpstrText, BOOL bNoScroll = FALSE, BOOL bCanUndo = FALSE)
    {
        DCHECK(::IsWindow(m_hWnd));
        InsertText(GetWindowTextLengthW(m_hWnd), lpstrText, bNoScroll, bCanUndo);
    }

    DWORD GetSel() const
    {
        DCHECK(::IsWindow(m_hWnd));
        return (DWORD)::SendMessage(m_hWnd, EM_GETSEL, 0, 0L);
    }

    void GetSel(int& nStartChar, int& nEndChar) const
    {
        DCHECK(::IsWindow(m_hWnd));
        ::SendMessage(m_hWnd, EM_GETSEL, (WPARAM)&nStartChar, (LPARAM)&nEndChar);
    }

    DWORD GetMargins() const
    {
        DCHECK(::IsWindow(m_hWnd));
        return (DWORD)::SendMessage(m_hWnd, EM_GETMARGINS, 0, 0L);
    }

    void SetMargins(UINT nLeft, UINT nRight)
    {
        DCHECK(::IsWindow(m_hWnd));
        ::SendMessage(m_hWnd, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(nLeft, nRight));
    }

    // Clipboard operations
    BOOL Undo()
    {
        DCHECK(::IsWindow(m_hWnd));
        return (BOOL)::SendMessage(m_hWnd, EM_UNDO, 0, 0L);
    }

    void Clear()
    {
        DCHECK(::IsWindow(m_hWnd));
        SetSel(0, -1);
        ::SendMessage(m_hWnd, WM_CLEAR, 0, 0L);

        // 清理文本的时候如果控件已经是非焦点状态则显示占位文本
        if (!IsFocus())
        {
            OnKillFocus();
        }
    }

    void Copy()
    {
        DCHECK(::IsWindow(m_hWnd));
        ::SendMessage(m_hWnd, WM_COPY, 0, 0L);
    }

    void Cut()
    {
        DCHECK(::IsWindow(m_hWnd));
        ::SendMessage(m_hWnd, WM_CUT, 0, 0L);
    }

    void Paste()
    {
        DCHECK(::IsWindow(m_hWnd));
        ::SendMessage(m_hWnd, WM_PASTE, 0, 0L);
    }
    // END -> MFC CEdit 用法

    // View
    virtual void SetVisible(bool is_visible) override;
    virtual void RequestFocus() override;
    virtual void SetEnabled(bool enabled) override;

protected:
    // NativeControl
    virtual HWND CreateNativeControl(HWND parent_container) override;
    virtual void OnDestroy() override;
    virtual LRESULT OnNotify(int w_param, LPNMHDR l_param) override;
    virtual LRESULT OnCommand(UINT code, int id, HWND source) override;
    virtual LRESULT OnReflectMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;

    // View
    virtual gfx::NativeViewAccessible GetNativeViewAccessible() override{ return (gfx::NativeViewAccessible)m_hWnd; }
    virtual bool SkipDefaultKeyEventProcessing(const ui::KeyEvent& event) override;
    virtual void OnEnabledChanged() override;
    virtual void OnFocus() override;
    virtual void VisibilityChanged(View* starting_from, bool is_visible) override;
    virtual void OnMouseEntered(const ui::MouseEvent& event) override;
    virtual void OnMouseExited(const ui::MouseEvent& event) override;
    virtual bool OnMousePressed(const ui::MouseEvent& event) override;
    virtual void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;
    virtual gfx::Size GetPreferredSize() override;
    virtual void OnPaint(gfx::Canvas* canvas) override;
    virtual void Layout() override;

    virtual void OnInvalidInput();

private:
    static WNDPROC def_wndproc_;
    static std::map<HWND, BilibiliNativeEditView*> instance_map_;
    static LRESULT CALLBACK BilibiliNativeEditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // 处理过了就返回true，否则false让窗口过程继续执行
    virtual LRESULT PreHandleMSG(UINT msg, WPARAM wParam, LPARAM lParam, bool &handled);
    void InitParam();
    void InitView();
    void SetNextFocusableViewFocus();
    void SetPasswordChar(TCHAR ch);
    void OnKillFocus();

private:
    BilibiliNativeEditController *controller_;
    HWND m_hWnd;
    HBRUSH brush_;
    bool edit_enable_;
    bool init_visible_;
    EnChangeReason change_reason_;

    gfx::VerticalAlignment vertical_alignment_;
    int default_width_in_chars_;
    base::string16 default_width_padding_char_;//
    bool draw_border_;
    gfx::Insets margins_;
    int left_inset_;
    int right_inset_;
    SkColor placeholder_text_color_;
    base::string16 placeholder_text_;
    gfx::Font font_;
    SkColor background_color_;
    SkColor text_color_;

    views::View *cooperate_view_;// 协作view（一般是搜索按钮）
    CooperateDirection cooperate_dir_;
    views::View *reset_view_;// 重新输入view
    StyleFlags style_;

    gfx::HorizontalAlignment h_align_;
    int limit_number_;
    bool want_return_;
    bool multi_line_;
    bool auto_v_scroll_;
    bool v_scroll_;
    bool number_only_;
    bool allow_minus_;
    bool allow_decimals_;
    bool obscured_;
    wchar_t obscured_char_ = L'●';
    base::string16 before_creation_text_;
    bool read_only_ = false;

    bool is_hovered_;
    bool is_empty_text_ = true;
    bool is_setting_ph_text_ = false;
    bool is_draw_background_enable_ = false;
    int radius_ = 0;
    int bk_opacity_ = 255;
    SkColor border_clrs_[views::Button::STATE_COUNT];

    base::WeakPtrFactory<BilibiliNativeEditView> weakptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(BilibiliNativeEditView);
};

#endif
