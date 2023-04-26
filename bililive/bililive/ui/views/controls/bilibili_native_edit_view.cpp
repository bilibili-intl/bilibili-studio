#include "bilibili_native_edit_view.h"

#include "SkPaint.h"
#include "base/bind.h"
#include "base/strings/utf_string_conversions.h"
#include "base/strings/string_util.h"

#include "ui/base/win/dpi.h"
#include "ui/native_theme/native_theme.h"
#include "ui/views/controls/native/native_view_host.h"
#include "ui/views/widget/widget.h"

#include "bililive/bililive/utils/bililive_canvas_drawer.h"



namespace
{
    const int kRadius = 4 * ui::GetDPIScale();
    const int kThickness = 6 * ui::GetDPIScale();
}

std::map<HWND, BilibiliNativeEditView*> BilibiliNativeEditView::instance_map_;
WNDPROC BilibiliNativeEditView::def_wndproc_ = nullptr;

BilibiliNativeEditView::BilibiliNativeEditView(views::View *cooperate_view, CooperateDirection dir/* = BD_RIGHT*/
    , StyleFlags style)
    : cooperate_view_(cooperate_view)
    , cooperate_dir_(dir)
    , style_(style)
    , radius_(kRadius)
    , weakptr_factory_(this)
{
    InitParam();
}

BilibiliNativeEditView::BilibiliNativeEditView(StyleFlags style)
    : cooperate_view_(nullptr)
    , cooperate_dir_(BD_RIGHT)
    , style_(style)
    , weakptr_factory_(this)
{
    InitParam();
}

BilibiliNativeEditView::~BilibiliNativeEditView()
{
    OnDestroy();//退出时可能没有先调用OnDestroy()，直接delete了，会导致崩溃，原因待排查
}

void BilibiliNativeEditView::InitParam()
{
    set_notify_enter_exit_on_child(true);
    m_hWnd = nullptr;
    brush_ = nullptr;
    edit_enable_ = true;
    init_visible_ = true;
    controller_ = nullptr;
    is_hovered_ = false;
    reset_view_ = nullptr;
    change_reason_ = EnChangeReason::ECR_DEVICE;
    obscured_ = (style_ & STYLE_OBSCURED) ? true : false;

    vertical_alignment_ = gfx::ALIGN_VCENTER;
    default_width_in_chars_ = 0;
    default_width_padding_char_ = L" ";
    draw_border_ = true;
    left_inset_ = right_inset_ = radius_ / 2;
    margins_.Set(left_inset_, left_inset_, right_inset_, right_inset_);
    placeholder_text_color_ = SK_ColorLTGRAY;
    font_ = gfx::Font(gfx::Font().GetFontName(), 13);
    background_color_ = SK_ColorWHITE;
    text_color_ = GetNativeTheme()->GetSystemColor(ui::NativeTheme::kColorId_TextfieldDefaultColor);
    /*text_color_ = GetNativeTheme()->GetSystemColor(read_only() ?
        ui::NativeTheme::kColorId_TextfieldReadOnlyColor :
        ui::NativeTheme::kColorId_TextfieldDefaultColor);*/

    h_align_ = gfx::ALIGN_LEFT;
    limit_number_ = 0;
    want_return_ = false;
    multi_line_ = false;
    v_scroll_ = false;
    auto_v_scroll_ = false;
    number_only_ = allow_minus_ = allow_decimals_ = false;

    border_clrs_[views::Button::STATE_NORMAL] = SK_ColorGRAY;
    border_clrs_[views::Button::STATE_HOVERED] = SK_ColorDKGRAY;
    border_clrs_[views::Button::STATE_PRESSED] = SK_ColorDKGRAY;
    border_clrs_[views::Button::STATE_DISABLED] = SkColorSetA(border_clrs_[views::Button::STATE_NORMAL], 0.3f * 255);
}

HWND BilibiliNativeEditView::CreateNativeControl(HWND parent_container)
{
    DWORD dwStyle = WS_VISIBLE | WS_CHILD | ES_NOHIDESEL | WS_TABSTOP;
    if (multi_line_)
    {
        dwStyle |= ES_MULTILINE | ES_WANTRETURN;
        dwStyle |= auto_v_scroll_ ? ES_AUTOVSCROLL : 0;
        dwStyle |= v_scroll_ ? WS_VSCROLL : 0;
    }
    else
    {
        dwStyle |= ES_AUTOHSCROLL;
        dwStyle |= want_return_ ? ES_WANTRETURN : 0;
    }
    switch (h_align_)
    {
    case gfx::ALIGN_LEFT:
        dwStyle |= ES_LEFT;
        break;
    case gfx::ALIGN_CENTER:
        dwStyle |= ES_CENTER;
        break;
    case gfx::ALIGN_RIGHT:
        dwStyle |= ES_RIGHT;
        break;
    }
    //dwStyle |= number_only_ ? ES_NUMBER : 0;  // 不使用控件自带的数值限制功能，我们自己过滤
    dwStyle |= obscured_ ? ES_PASSWORD : 0;
    m_hWnd = CreateWindowExW(GetAdditionalExStyle(), L"EDIT", L"",
        dwStyle, 0, 0, 0, 0, parent_container, 0, GetModuleHandle(0), 0);
    DCHECK(m_hWnd);

    if (def_wndproc_ == nullptr)
    {
        def_wndproc_ = (WNDPROC)::GetWindowLongPtr(m_hWnd, static_cast<int>(GWLP_WNDPROC));
    }

    // 设置窗口过程，为的是拦截windows消息进行通知回调
    instance_map_[m_hWnd] = this;
    LONG ret =  ::SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&BilibiliNativeEditView::BilibiliNativeEditWndProc));
    
    InitView();

    return m_hWnd;
}

void BilibiliNativeEditView::SetExLayeredEnable(bool enable, int opacity)
{
    if (m_hWnd)
    {
        bk_opacity_ = opacity;
        ::LONG ex_style = GetWindowLong(m_hWnd, GWL_EXSTYLE);
        bool layered = ex_style & WS_EX_LAYERED;
        if (enable != layered)
        {
            if (enable)
            {
                ::SetWindowLong(m_hWnd, GWL_EXSTYLE, ex_style | WS_EX_LAYERED);              
            } 
            else
            {
                ::SetWindowLong(m_hWnd, GWL_EXSTYLE, ex_style & ~WS_EX_LAYERED);
            }          
        }
        if (enable)
        {
            ::SetLayeredWindowAttributes(m_hWnd, 0, opacity, LWA_ALPHA);
        }
    }
}

void BilibiliNativeEditView::OnDestroy()
{
    if (brush_) {
        ::DeleteObject(brush_);
        brush_ = nullptr;
    }
    instance_map_.erase(m_hWnd);
}

LRESULT BilibiliNativeEditView::OnNotify(int w_param, LPNMHDR l_param)
{
    return S_OK;
}

LRESULT BilibiliNativeEditView::OnCommand(UINT code, int id, HWND source)
{
    if (source == m_hWnd)
    {
        switch (code)
        {
        case EN_SETFOCUS:
        {
            std::wstring str = GetText();
            if (str == placeholder_text_ && is_empty_text_)
            {
                SetText(L"", ECR_API);
            }

            // 如果当前处于密码状态下，那么处于焦点状态时要设置密码字符
            if (obscured_)
            {
                SetPasswordChar(obscured_char_);
            }

            SchedulePaint();
        }
        break;
        case EN_KILLFOCUS:
            OnKillFocus();
            break;
        case EN_CHANGE:
        {
            base::string16 edit_text = GetText();
            if (!is_setting_ph_text_)
            {
                is_empty_text_ = edit_text.empty();
            }

            if (controller_ && change_reason_ == EnChangeReason::ECR_DEVICE)
            {
                controller_->ContentsChanged(this, edit_text);
            }

            if (!IsFocus())
            {
                // 失去焦点时如果有文本且不是提示文本则不隐藏清除按钮
                if (reset_view_)
                {
                    if (edit_text == placeholder_text() || edit_text.empty())
                    {
                        reset_view_->SetVisible(false);
                    }
                }

                // 丢失焦点的状态下发生了文本变更，若不是提示文本则根据当前密码状态设置密码符
                if (obscured_)
                {
                    if (edit_text == placeholder_text() || edit_text.empty())
                    {
                        SetPasswordChar(0);
                    }
                    else
                    {
                        SetPasswordChar(obscured_char_);
                    }
                }
            }
        }
            break;
        default:
            break;
        }
    }
    return S_OK;
}

LRESULT BilibiliNativeEditView::OnReflectMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LRESULT result = 0;
    bHandled = FALSE;
    switch (uMsg)
    {
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORSTATIC: // win32 edit disable下设置字色是无用的，windows底层自己会调系统色来画字
    {
        bHandled = TRUE;
        SkColor text_color = edit_enable_ ? text_color_ : placeholder_text_color_;
        COLORREF clrText = RGB(SkColorGetR(text_color), SkColorGetG(text_color), SkColorGetB(text_color));
        COLORREF clrBk = RGB(SkColorGetR(background_color_), SkColorGetG(background_color_), SkColorGetB(background_color_));
        if (!IsFocus())
        {
            std::wstring str = GetText();
            if (str == placeholder_text_ && is_empty_text_)
            {
                clrText = RGB(SkColorGetR(placeholder_text_color_), SkColorGetG(placeholder_text_color_),
                    SkColorGetB(placeholder_text_color_));
            }
        }
        ::SetTextColor((HDC)wParam, clrText);
        ::SetBkColor((HDC)wParam, clrBk);

        if (brush_) {
            ::DeleteObject(brush_);
        }
        brush_ = ::CreateSolidBrush(clrBk);
        result = (LRESULT)brush_;
    }
    break;
    default:
        break;
    }
    return result;
}

LRESULT CALLBACK BilibiliNativeEditView::BilibiliNativeEditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    BilibiliNativeEditView *instance = instance_map_[hwnd];
    if (instance)
    {
        bool handled = false;
        LRESULT lret = instance->PreHandleMSG(msg, wParam, lParam, handled);
        if (handled)
        {
            return lret;
        }

        bool excute_cmd = true;
        if (instance->controller_)
        {
            excute_cmd = instance->controller_->PreHandleMSG(instance, msg, wParam, lParam);
        }
        if (excute_cmd)
        {
            return ::CallWindowProc((WNDPROC)def_wndproc_, hwnd, msg, wParam, lParam);
        }
        else
        {
            return S_OK;
        }
    }
    return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT BilibiliNativeEditView::PreHandleMSG(UINT msg, WPARAM wParam, LPARAM lParam, bool &handled)
{
    handled = false;
    switch (msg)
    {
    case WM_ERASEBKGND:
    {
        handled = true;
        RECT rect = { 0 };
        ::GetClientRect(m_hWnd, &rect);

        HBRUSH brush = ::CreateSolidBrush(
            RGB(SkColorGetR(background_color_),
            SkColorGetG(background_color_),
            SkColorGetB(background_color_)));

        ::FillRect((HDC)wParam, &rect, brush);

        ::DeleteObject(brush);
        return 1;
    }
        break;
    case WM_KEYDOWN:
    {
        if (wParam == 'a' || wParam == 'A')
        {
            if (GetKeyState(VK_CONTROL) & 0x8000)
            {
                SetSel(0, -1);
            }
        }
    }
    break;
    case WM_MOUSEMOVE:
        OnMouseEntered(ui::MouseEvent(ui::ET_MOUSE_ENTERED, gfx::Point(), gfx::Point(), ui::EF_NONE));
        break;
    case WM_MOUSELEAVE:
        OnMouseExited(ui::MouseEvent(ui::ET_MOUSE_EXITED, gfx::Point(), gfx::Point(), ui::EF_NONE));
        break;
    case WM_PASTE:
    {
        if (number_only_)
        {
            if (::OpenClipboard(NULL))
            {
                //获得剪贴板数据
                base::string16 paste_text;
                HGLOBAL hUnicodeMem = ::GetClipboardData(CF_UNICODETEXT);
                if (NULL != hUnicodeMem)
                {
                    wchar_t* lpStr = (wchar_t*)::GlobalLock(hUnicodeMem);
                    if (NULL != lpStr)
                    {
                        paste_text = lpStr;
                        ::GlobalUnlock(hUnicodeMem);
                    }
                }
                if (!hUnicodeMem)
                {
                    HGLOBAL hAsciiMem = ::GetClipboardData(CF_TEXT);
                    if (NULL != hAsciiMem)
                    {
                        char* lpStr = (char*)::GlobalLock(hAsciiMem);
                        if (NULL != lpStr)
                        {
                            paste_text = base::ASCIIToUTF16(lpStr);
                            ::GlobalUnlock(hAsciiMem);
                        }
                    }
                }
                ::CloseClipboard();

                if (!paste_text.empty())
                {
                    base::string16 edit_text = text();
                    int nStartChar = 0, nEndChar = 0;
                    GetSel(nStartChar, nEndChar);
                    // 直接拼接出粘贴后的字符串，再对整个字符串进行判断就行
                    edit_text.replace(nStartChar, nEndChar - nStartChar, paste_text.c_str());
                    bool valid = true;
                    // 负号，只允许有一个，且不需在最前
                    {
                        base::string16::size_type pos = edit_text.find('-');
                        if (pos != base::string16::npos)
                        {
                            if (allow_minus_)
                            {
                                int letter_count = 1;
                                while (pos = edit_text.find('-', pos + 1), pos != base::string16::npos)
                                {
                                    ++letter_count;
                                }
                                if (letter_count > 1)
                                {
                                    valid = false;
                                }
                                else
                                {
                                    if (edit_text.at(0) != '-')
                                    {
                                        valid = false;
                                    }
                                }
                            }
                            else
                            {
                                valid = false;
                            }
                        }
                    }
                    // 小数点，只允许有一个，且只能跟在数字之后
                    if (valid)
                    {
                        base::string16::size_type pos = edit_text.find('.');
                        if (pos != base::string16::npos)
                        {
                            if (allow_decimals_)
                            {
                                base::string16::size_type tmp_pos = pos;

                                int letter_count = 1;
                                while (pos = edit_text.find('.', pos + 1), pos != base::string16::npos)
                                {
                                    ++letter_count;
                                }
                                if (letter_count > 1)
                                {
                                    valid = false;
                                }
                                else
                                {
                                    if (tmp_pos > 0)
                                    {
                                        if (0 == iswdigit((int)edit_text.at(tmp_pos - 1)))
                                        {
                                            valid = false;
                                        }
                                    }
                                    else
                                    {
                                        valid = false;
                                    }
                                }
                            }
                            else
                            {
                                valid = false;
                            }
                        }
                    }
                    // 负号和小数点都通过了就逐一检查是否存在非数字字符
                    if (valid)
                    {
                        ReplaceSubstringsAfterOffset(&edit_text, 0, L"-", L"");
                        ReplaceSubstringsAfterOffset(&edit_text, 0, L".", L"");
                        for (base::string16::size_type i = 0; i < edit_text.length(); i++)
                        {
                            if (0 == iswdigit((int)edit_text.at(i)))
                            {
                                valid = false;
                                break;
                            }
                        }
                    }
                    if (!valid)
                    {
                        OnInvalidInput();
                        handled = true;
                        return S_OK;
                    }
                }
            }
        }
    }
    break;
    case WM_CHAR:
    {
        if (number_only_)
        {
            static std::vector<UINT> filter_keys;
            if (filter_keys.size() == 0)
            {
                /*filter_keys.push_back(VK_RETURN);
                filter_keys.push_back(VK_DELETE);*/
                filter_keys.push_back(0);   // Paste粘贴时触发
                filter_keys.push_back(0x16);   // ctrl+v粘贴时触发
                filter_keys.push_back(VK_ESCAPE);
                filter_keys.push_back(VK_BACK);
                filter_keys.push_back(VK_CANCEL);// 鼠标框选会触发
                filter_keys.push_back(VK_LBUTTON);// ctrl+a会触发
            }
            if (std::find(filter_keys.begin(), filter_keys.end(), wParam) != filter_keys.end())
            {
                // 执行默认过程
                break;
            }

            bool valid = false;
            wchar_t letter = (wchar_t)wParam;
            if (0 != iswdigit((int)letter))
            {
                // 允许继续执行
                valid = true;
            }
            else
            {
                // 中断默认执行过程并提示
                if (letter == '-' && allow_minus_)
                {
                    // 负号必须在最前，且只能有一个
                    base::string16 edit_text = text();
                    if (edit_text.find('-') == base::string16::npos)
                    {
                        int nStartChar = 0, nEndChar = 0;
                        GetSel(nStartChar, nEndChar);
                        if (nStartChar == 0)
                        {
                            valid = true;
                        }
                    }
                }
                if (letter == '.' && allow_decimals_)
                {
                    // 小数点必须跟在数字后面
                    base::string16 edit_text = text();
                    if (edit_text.find('.') == base::string16::npos)
                    {
                        int nStartChar = 0, nEndChar = 0;
                        GetSel(nStartChar, nEndChar);
                        if (nStartChar != 0)
                        {
                            if (iswdigit(edit_text.at(nStartChar - 1)) != 0)
                            {
                                valid = true;
                            }
                        }
                    }
                }
            }
            if (!valid)
            {
                OnInvalidInput();
                handled = true;
                return S_OK;
            }
        }
    }
    break;
    case WM_SETFOCUS:
    {
        if (!edit_enable_)
        {
            ::SetFocus(nullptr);

            SetNextFocusableViewFocus();

            handled = true;
            return S_OK;
        }
        else
        {
            if (reset_view_)
            {
                reset_view_->SetVisible(true);
            }
        }
    }
    break;
    case WM_NCHITTEST:
    {
        if (!edit_enable_)
        {
            handled = true;
            return HTTRANSPARENT;
        }
    }
    break;
    default:
        break;
    }
    return S_OK;
}

void BilibiliNativeEditView::InitView()
{
    SetVisible(init_visible_);
    SetFont(font_);
    LimitText(limit_number_);
    SetMargins(0, 0);
    SetReadOnly(read_only_);

    if (!before_creation_text_.empty())
    {
        SetText(before_creation_text_);
        SetSel(0, 0);
    }
    else if (!placeholder_text_.empty())
    {
        is_setting_ph_text_ = true;
        SetText(placeholder_text_, ECR_API);
        is_setting_ph_text_ = false;
        SetSel(0, 0);
    }
}

void BilibiliNativeEditView::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details)
{
    __super::ViewHierarchyChanged(details);

    if (details.is_add && details.child == this)
    {
        if (cooperate_view_)
        {
            AddChildView(cooperate_view_);
        }

        // 如果被添加到视图层级时顶级view尚未关联实际窗口则抛任务重布局，底层的NativeControl::Layout()中会创建控件
        if (!GetWidget())
        {
            base::MessageLoop::current()->PostTask(FROM_HERE,
                base::Bind(&BilibiliNativeEditView::Layout, weakptr_factory_.GetWeakPtr()));
        }
    }
}

void BilibiliNativeEditView::SetEnabled(bool enabled)
{
    if (edit_enable_ != enabled)
    {
        edit_enable_ = enabled;
        if (hwnd_view_)
        {
            ::InvalidateRect(m_hWnd, &hwnd_view_->GetLocalBounds().ToRECT(), FALSE);
        }
        SchedulePaint();
    }
}

void BilibiliNativeEditView::OnEnabledChanged()
{
    edit_enable_ = enabled();
    /*if (::IsWindow(m_hWnd))
    {
        ::EnableWindow(m_hWnd, enabled());
    }*/
}

bool BilibiliNativeEditView::SkipDefaultKeyEventProcessing(const ui::KeyEvent& event)
{
    if (multi_line_)
    {
        return true;
    }
    return want_return_;
}

void BilibiliNativeEditView::SetFocus()
{
    OnFocus();
}

void BilibiliNativeEditView::OnFocus()
{
    if (::IsWindow(m_hWnd))
    {
        ::SetFocus(m_hWnd);
    }
}

void BilibiliNativeEditView::VisibilityChanged(View* starting_from, bool is_visible)
{
    if (::IsWindow(m_hWnd))
    {
        ::ShowWindow(m_hWnd, is_visible ? SW_SHOW : SW_HIDE);
    }
}

void BilibiliNativeEditView::SetVisible(bool is_visible)
{
    // 当实际编辑框尚未创建就被隐藏的话后续是不会得到创建机会的,所以在此先记录下窗口未创建前的显示状态
    // 等到窗口实际创建完成之后再置回来
    if (!m_hWnd)
    {
        init_visible_ = is_visible;
        return;
    }

    if (is_visible != visible())
    {
        if (m_hWnd)
        {
            View::SetVisible(is_visible);
        }
        if (!is_visible/* && container_*/)
        {
            //::DestroyWindow(*container_);
            //::ShowWindow(*container_, SW_HIDE);
        }
        else if (is_visible/* && !container_*/)
        {
            //ValidateNativeControl();
            if (hwnd_view_ == NULL) {
                hwnd_view_ = new views::NativeViewHost;
                AddChildView(hwnd_view_);
            }
        }
    }
}

void BilibiliNativeEditView::OnMouseEntered(const ui::MouseEvent& event)
{
    if (edit_enable_)
    {
        if (draw_border_)
        {
            if (!is_hovered_)
            {
                is_hovered_ = true;
                SchedulePaint();
            }
        }
    }
}

void BilibiliNativeEditView::OnMouseExited(const ui::MouseEvent& event)
{
    if (edit_enable_)
    {
        if (draw_border_)
        {
            if (is_hovered_)
            {
                is_hovered_ = false;
                SchedulePaint();
            }
        }
    }
}

bool BilibiliNativeEditView::OnMousePressed(const ui::MouseEvent& event)
{
    if (edit_enable_)
    {
        RequestFocus();
    }
    return false;
}

void BilibiliNativeEditView::RequestFocus()
{
    if (edit_enable_)
    {
        if (::IsWindow(m_hWnd) && ::IsWindowEnabled(m_hWnd))
        {
            if (!IsFocus())
            {
                ::SetFocus(m_hWnd);
            }
        }
    }
    else
    {
        SetNextFocusableViewFocus();
    }
    //__super::RequestFocus();
}

void BilibiliNativeEditView::SetNextFocusableViewFocus()
{
    views::View *next_view = GetNextFocusableView();
    while (next_view)
    {
        if (next_view->focusable())
        {
            next_view->RequestFocus();
            break;
        }
        next_view = next_view->GetNextFocusableView();
    }
}

gfx::Size BilibiliNativeEditView::GetPreferredSize()
{
    int text_cx = std::max(font().GetStringWidth(default_width_padding_char_) * default_width_in_chars_,
        font().GetStringWidth(text()));
    gfx::Size pref_size(std::max(font().GetHeight(), text_cx + 10),
        font().GetHeight() + 2);
    if (cooperate_view_)
    {
        gfx::Size coop_pref_size = cooperate_view_->GetPreferredSize();
        switch (cooperate_dir_)
        {
        case BD_RIGHT:
        case BD_LEFT:
        {
            pref_size.Enlarge(coop_pref_size.width(), std::max(0, coop_pref_size.height() - font().GetHeight()));
        }
        break;
        case BD_BOTTOM:
        case BD_CORNER:
        {
            pref_size.Enlarge(std::max(0, coop_pref_size.width() - font().GetHeight()), coop_pref_size.height());
        }
        break;
        default:    // BD_NONE
            DCHECK(false);
            break;
        }
    }
    pref_size.Enlarge(margins_.width(), margins_.height());
    return pref_size;
}

void BilibiliNativeEditView::OnPaint(gfx::Canvas* canvas)
{
    if (is_draw_background_enable_)
    {
		//非输入框区域填充背景色，就是输入框区域透明不处理
		gfx::Rect hwnd_view_content = hwnd_view_->bounds();
		gfx::Rect content = GetContentsBounds();
		SkPaint bk_paint;
		SkColor bk_color = SkColorSetA(background_color_, bk_opacity_);
		bk_paint.setColor(bk_color);

		int border = 2;//这里的2应该是下面int line_height = font_.GetHeight() + 2;里的
		if (hwnd_view_content.x() > 0)//左
		{
			bililive::FillRoundRect(canvas, 0, 0, hwnd_view_content.x() + border, content.height(), 0, bk_paint);
		}
		if (hwnd_view_content.y() > 0)//上
		{
			bililive::FillRoundRect(canvas, hwnd_view_content.x(), 0, hwnd_view_content.width(), hwnd_view_content.y() + border, 0, bk_paint);
		}
		int right = hwnd_view_content.x() + hwnd_view_content.width();
		if (right < content.width())//右
		{
			bililive::FillRoundRect(canvas, right - border, 0, content.width() - right, content.height(), 0, bk_paint);
		}
		int bottom = hwnd_view_content.y() + hwnd_view_content.height();
		if (bottom < content.height())//下
		{
			bililive::FillRoundRect(canvas, hwnd_view_content.x(), bottom - border, hwnd_view_content.width(), content.height() - bottom + border, 0, bk_paint);
		}
    }

    if (draw_border_)
    {
        SkPaint paint;
        paint.setAntiAlias(true);
        if (/*edit_enable_*/enabled())
        {
            if (IsFocus())
            {
                paint.setColor(border_clrs_[views::Button::STATE_HOVERED]);
            }
            else
            {
                paint.setColor(is_hovered_ ? border_clrs_[views::Button::STATE_HOVERED] : border_clrs_[views::Button::STATE_NORMAL]);
            }
        }
        else
        {
            paint.setColor(border_clrs_[views::Button::STATE_DISABLED]);
        }
        bililive::DrawRoundRect(canvas, 0, 0, width(), height(), radius_, paint);
    }
}

void BilibiliNativeEditView::Layout()
{
    NativeControl::Layout();

    gfx::Rect rect_edit = GetLocalBounds();    // 供编辑框使用的区域
    rect_edit.Inset(margins_);
    if (cooperate_view_ && !rect_edit.IsEmpty())
    {
        gfx::Size coop_pref_size = cooperate_view_->GetPreferredSize();
        switch (cooperate_dir_)
        {
        case BD_RIGHT:
        {
            cooperate_view_->SetBounds(rect_edit.right() - coop_pref_size.width(),
                rect_edit.y() + (rect_edit.height() - coop_pref_size.height()) / 2,
                coop_pref_size.width(), coop_pref_size.height());

            rect_edit.Inset(0, 0, coop_pref_size.width() + kThickness, 0);
        }
        break;
        case BD_LEFT:
        {
            cooperate_view_->SetBounds(rect_edit.x(),
                rect_edit.y() + (rect_edit.height() - coop_pref_size.height()) / 2,
                coop_pref_size.width(), coop_pref_size.height());

            rect_edit.Inset(coop_pref_size.width() + kThickness, 0, 0, 0);
        }
        break;
        case BD_BOTTOM:
        {
            cooperate_view_->SetBounds(rect_edit.x(), rect_edit.bottom() - coop_pref_size.height(),
                rect_edit.width(), coop_pref_size.height());

            rect_edit.Inset(0, 0, 0, coop_pref_size.height());
        }
        break;
        case BD_CORNER:
        {
            cooperate_view_->SetBounds(rect_edit.right() - coop_pref_size.width(), rect_edit.bottom() - coop_pref_size.height(),
                coop_pref_size.width(), coop_pref_size.height());

            rect_edit.Inset(0, 0, 0, coop_pref_size.height());
        }
        break;
        default:    // BD_NONE
            DCHECK(false);
        break;
        }
    }
    if (reset_view_)
    {
        // 当前重置输入view只支持单行、协作view居左对齐的风格，其他风格有点另类
        if (edit_enable_ && !multi_line_ && cooperate_dir_ == BD_LEFT)
        {
            gfx::Size reset_pref_size = reset_view_->GetPreferredSize();

            reset_view_->SetBounds(rect_edit.right() - reset_pref_size.width(),
                rect_edit.y() + (rect_edit.height() - reset_pref_size.height()) / 2,
                reset_pref_size.width(), reset_pref_size.height());

            rect_edit.Inset(0, 0, reset_pref_size.width() + kThickness, 0);
        }
        else
        {
            NOTREACHED() << "no support edit style";
        }
    }
    if (hwnd_view_ && !rect_edit.IsEmpty())
    {
        if (!hwnd_view_->notify_enter_exit_on_child())
        {
            hwnd_view_->set_notify_enter_exit_on_child(true);
            hwnd_view_->set_border(views::Border::CreateSolidBorder(1, SK_ColorBLUE));
        }

        int line_height = font_.GetHeight() + 2;// 多个2是为了使编辑框的闪烁光标能全部显示
        if (multi_line_)
        {
            line_height = rect_edit.height();
        }
        switch (vertical_alignment_)
        {
        case gfx::ALIGN_TOP:
            hwnd_view_->SetBounds(rect_edit.x(), rect_edit.y(), rect_edit.width(), line_height);
            break;
        case gfx::ALIGN_VCENTER:
            hwnd_view_->SetBounds(rect_edit.x(), rect_edit.y() + (rect_edit.height() - line_height) / 2, rect_edit.width(), line_height);
            break;
        case gfx::ALIGN_BOTTOM:
            hwnd_view_->SetBounds(rect_edit.x(), rect_edit.bottom() - line_height, rect_edit.width(), line_height);
            break;
        default:
            hwnd_view_->SetBounds(rect_edit.x(), rect_edit.y(), rect_edit.width(), rect_edit.height());
            break;
        }
    }
}

void BilibiliNativeEditView::SetBorderColor(SkColor clrBlur, SkColor clrFocus)
{
    draw_border_ = true;
    if (margins_.empty())
    {
        margins_.Set(radius_ / 2, left_inset_, radius_ / 2, right_inset_);
    }
    border_clrs_[views::Button::STATE_NORMAL] = clrBlur;
    border_clrs_[views::Button::STATE_HOVERED] = clrFocus;
    border_clrs_[views::Button::STATE_PRESSED] = clrFocus;
    border_clrs_[views::Button::STATE_DISABLED] = SkColorSetA(border_clrs_[views::Button::STATE_NORMAL], 0.3f * 255);
    SchedulePaint();
}

bool BilibiliNativeEditView::IsFocus() const
{
    if (::IsWindow(m_hWnd))
    {
        return (::GetFocus() == m_hWnd);
    }
    return false;
}

void BilibiliNativeEditView::SetMultiLine(bool multi_line)
{
    SetMultiLine(multi_line, false, multi_line);
}

void BilibiliNativeEditView::SetMultiLine(bool multi_line, bool v_scroll, bool auto_vscroll)
{
    multi_line_ = multi_line;
    if (multi_line_)
    {
        v_scroll_ = v_scroll;
        if (v_scroll_)
        {
            auto_v_scroll_ = false;
        }
        else
        {
            auto_v_scroll_ = auto_vscroll;
        }
    }
    else
    {
        v_scroll = auto_v_scroll_ = false;
    }
}

void BilibiliNativeEditView::SetSkipDefaultKeyEventProcessing(bool want_return)
{
    want_return_ = want_return;
}

void BilibiliNativeEditView::set_default_width_in_chars(int default_width, const base::string16 &padding_char)
{
    default_width_in_chars_ = default_width;
    default_width_padding_char_ = padding_char;
    InvalidateLayout();
}

void BilibiliNativeEditView::GetHorizontalMargins(int &left, int &right) const
{
    left = margins_.left();
    right = margins_.right();
}

void BilibiliNativeEditView::SetResetTypeInView(views::View * reset_view)
{
    if (reset_view_)
    {
        RemoveChildView(reset_view_);
        delete reset_view_;
    }
    reset_view_ = reset_view;
    if(reset_view_)
    {
        AddChildView(reset_view_);
        reset_view_->SetVisible(IsFocus());
    }
    if (hwnd_view_)
    {
        Layout();
    }
}

void BilibiliNativeEditView::SetNumberOnly(bool number_only, bool allow_minus /*= false*/, bool allow_decimals /*= false*/)
{
    number_only_ = number_only;
    allow_minus_ = allow_minus;
    allow_decimals_ = allow_decimals;
}

void BilibiliNativeEditView::OnInvalidInput()
{
}

void BilibiliNativeEditView::SetPasswordChar(TCHAR ch)
{
    DCHECK(::IsWindow(m_hWnd));

    if (m_hWnd) {
        ::SendMessage(m_hWnd, EM_SETPASSWORDCHAR, ch, 0L);
        ::InvalidateRect(m_hWnd, nullptr, FALSE);
    }
}

void BilibiliNativeEditView::OnKillFocus()
{
    std::wstring str = GetText();
    if (str.empty())
    {
        is_setting_ph_text_ = true;
        SetText(placeholder_text_, ECR_API);
        is_setting_ph_text_ = false;

        // 如果当前处于密码状态下，那么占位提示文本要原文显示出来，不然会变成密码字符
        if (obscured_)
        {
            SetPasswordChar(0);
        }
    }
    SchedulePaint();
}

// START -> 兼容views::Textfield
void BilibiliNativeEditView::SetReadOnly(bool read_only)
{
    read_only_ = read_only;
    if (m_hWnd)
    {
        SetReadOnlyRaw(read_only_);
    }
}

bool BilibiliNativeEditView::read_only() const
{
    return read_only_;
}

string16 BilibiliNativeEditView::text() const
{
    if (m_hWnd)
    {
        std::wstring r;
        int len = (int)SendMessageW(m_hWnd, WM_GETTEXTLENGTH, 0, 0);
        if (len > 0)
        {
            r.resize(len);
            SendMessageW(m_hWnd, WM_GETTEXT, (WPARAM)(len + 1), (LPARAM)&r[0]);
        }
        return std::move(r);
    }
    return before_creation_text_;
}

void BilibiliNativeEditView::SetText(const string16& text, EnChangeReason reason)
{
    if (!is_setting_ph_text_) {
        is_empty_text_ = text.empty();
    }

    if (m_hWnd)
    {
        change_reason_ = reason;

        if (!text.empty())
        {
            SendMessageW(m_hWnd, WM_SETTEXT, 0, (LPARAM)&text[0]);
        }
        else
        {
            if (IsFocus())
            {
                SendMessageW(m_hWnd, WM_SETTEXT, 0, 0);
            }
            else
            {
                is_setting_ph_text_ = true;
                SendMessageW(m_hWnd, WM_SETTEXT, 0, (LPARAM)&placeholder_text_[0]);
                is_setting_ph_text_ = false;
            }
        }

        change_reason_ = EnChangeReason::ECR_DEVICE;
    }
    else
    {
        before_creation_text_ = text;
    }
}

void BilibiliNativeEditView::AppendText(const string16& text)
{
    AppendText(text.c_str());
}

void BilibiliNativeEditView::InsertOrReplaceText(const string16& text)
{
    ReplaceSel(text.c_str());
}

void BilibiliNativeEditView::SelectAll(/*bool reversed*/)
{
    SetSel(0, -1);
}

void BilibiliNativeEditView::ClearSelection()
{
    SelectAll();
    InsertOrReplaceText(L"");
}

bool BilibiliNativeEditView::HasSelection() const
{
    return GetSel() > 0;
}

void BilibiliNativeEditView::SetFont(const gfx::Font& font)
{
    font_ = font;
    if (m_hWnd)
    {
        SendMessageW(m_hWnd, WM_SETFONT, (WPARAM)font_.GetNativeFont(), MAKELPARAM(TRUE, 0));
    }
}

void BilibiliNativeEditView::SetTextColor(SkColor color)
{
    text_color_ = color;
    SchedulePaint();
}

void BilibiliNativeEditView::SetBackgroundColor(SkColor color)
{
    background_color_ = color;
    SchedulePaint();
}

void BilibiliNativeEditView::SetHorizontalMargins(int left, int right)
{
    left_inset_ = left;
    right_inset_ = right;

    margins_.Set(margins_.top(), left, margins_.bottom(), right);
    PreferredSizeChanged();
}

void BilibiliNativeEditView::SetVerticalMargins(int top, int bottom)
{
    margins_.Set(top, margins_.left(), bottom, margins_.right());
    PreferredSizeChanged();
}

void BilibiliNativeEditView::SetVerticalAlignment(gfx::VerticalAlignment alignment)
{
    vertical_alignment_ = alignment;
    PreferredSizeChanged();
}

void BilibiliNativeEditView::RemoveBorder()
{
    draw_border_ = false;
    // 没有border就没必要缩进，不然和其他同列控件看起来不对齐
    margins_.Set(0, 0, 0, 0);
}

void BilibiliNativeEditView::SelectRange(DWORD dwSelection)
{
    SetSel(dwSelection);
}

void BilibiliNativeEditView::SetObscured(bool obscured)
{
    if(obscured_ != obscured)
    {
        obscured_ = obscured;
        TCHAR ch = obscured_ ? obscured_char_ : 0;
        SetPasswordChar(ch);
    }
}

// END -> 兼容views::Textfield
