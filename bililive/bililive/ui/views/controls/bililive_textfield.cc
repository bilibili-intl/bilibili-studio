#include "bililive_textfield.h"

#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"

#include "ui/base/win/dpi.h"
#include "ui/views/controls/button/label_button.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/gfx/canvas.h"

#include "third_party/skia/include/core/SkPaint.h"

#include "ui/base/resource/resource_bundle.h"

#include "bililive/bililive/utils/bililive_canvas_drawer.h"


namespace
{
    const int kRadius = 4 * ui::GetDPIScale();
    const int kThickness = 5 * ui::GetDPIScale();
}


BililiveTextfield::BililiveTextfield(views::Textfield::StyleFlags style/* = views::Textfield::STYLE_DEFAULT*/)
    : textfield_style_(style)
    , textfield_(new views::Textfield(style))
    , cooperate_dir_(BD_NONE)
    , cooperate_view_(NULL)
{
    InitParam();
}

BililiveTextfield::BililiveTextfield(views::View *view, CooperateDirection dir/* = BD_RIGHT*/, views::Textfield::StyleFlags style/* = views::Textfield::STYLE_DEFAULT*/)
    : textfield_style_(style)
    , textfield_(new views::Textfield(style))
    , cooperate_dir_(dir)
    , cooperate_view_(view)
{
    InitParam();
}

BililiveTextfield::~BililiveTextfield()
{
}

void BililiveTextfield::InitParam()
{
    has_border_ = true;
    blur_border_color_ = clrTextfieldBorder;
    focus_border_color_ = clrTextfieldFocusBorder;
    add_focus_listener_ = false;
    left_inset_ = right_inset_ = kRadius / 2;
    margins_.Set(left_inset_, left_inset_, right_inset_, right_inset_);

    set_notify_enter_exit_on_child(true);
}

void BililiveTextfield::SetHorizontalMargins(int left, int right)
{
    left_inset_ = left;
    right_inset_ = right;
    if (has_border_)
    {
        margins_.Set(margins_.top(), left, margins_.bottom(), right);
        PreferredSizeChanged();
    }
}

void BililiveTextfield::SetBorder(bool has_border)
{
    has_border_ = has_border;
    if (has_border_)
    {
        int round_corner = kRadius / 2;
        margins_.Set(round_corner, left_inset_, round_corner, right_inset_);
    }
    else
    {
        // 没有border就没必要缩进，不然和其他同列控件看起来不对齐
        margins_.Set(0, 0, 0, 0);
    }
}

void BililiveTextfield::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            if (textfield_)
            {
                textfield_->RemoveBorder();
                //textfield_->SetBackgroundColor(SK_ColorWHITE);
                AddChildView(textfield_);
            }
            if (cooperate_view_)
            {
                AddChildView(cooperate_view_);
            }
            AddFocusChangeListener();
        }
        else
        {
            RemoveFocusChangeListener();
        }
    }
}

void BililiveTextfield::OnPaintBackground(gfx::Canvas* canvas)
{
    if (has_border_)
    {
        gfx::Rect rt = GetLocalBounds();

        SkPaint paint;
        if (background())
        {
            paint.setColor(background()->get_color());
            bililive::FillRoundRect(canvas, rt.x(), rt.y(), rt.width(), rt.height(), kRadius, paint);
        }

        paint.setColor((textfield_->HasFocus() || textfield_->IsMouseHovered() || IsMouseHovered())
                       ? focus_border_color_ : blur_border_color_); //线框颜色
        if (!enabled())
        {
            paint.setColor(SkColorSetA(blur_border_color_, 0.3f * 255));
        }
        bililive::DrawRoundRect(canvas, rt.x(), rt.y(), rt.width(), rt.height(), kRadius, paint);
    }
    else
    {
        __super::OnPaintBackground(canvas);
    }
}

void BililiveTextfield::OnEnabledChanged()
{
    if (textfield_)
    {
        textfield_->SetEnabled(enabled());
        textfield_->set_placeholder_text_color(enabled() ? clrLabelTipText : clrLabelDisableText);
    }
    if (cooperate_view_)
    {
        cooperate_view_->SetEnabled(enabled());
    }
}

void BililiveTextfield::SetFont(const gfx::Font &font)
{
    if (textfield_)
    {
        textfield_->SetFont(font);
    }
}

void BililiveTextfield::SetBorderColor(SkColor clrBlur, SkColor clrFocus)
{
    blur_border_color_ = clrBlur;
    focus_border_color_ = clrFocus;
}

void BililiveTextfield::SetBackgroundColor(SkColor color)
{
    if (textfield_)
    {
        textfield_->SetBackgroundColor(color);
    }
}

void BililiveTextfield::SetTextColor(SkColor color)
{
    if (textfield_)
    {
        textfield_->SetTextColor(color);
    }
}

gfx::Size BililiveTextfield::GetPreferredSize()
{
    gfx::Size textfield_size = textfield_->GetPreferredSize();
    if (cooperate_view_)
    {
        gfx::Size cooperate_size = cooperate_view_->GetPreferredSize();
        switch (cooperate_dir_)
        {
        case BD_RIGHT:
        case BD_LEFT:
            textfield_size.Enlarge(cooperate_size.width() + kThickness, 0);
            textfield_size.set_height(std::max(textfield_size.height(), cooperate_size.height()));
            break;
        default:
            break;
        }
    }
    textfield_size.Enlarge(margins_.width(), margins_.height());
    return textfield_size;
}

void BililiveTextfield::Layout()
{
    AddFocusChangeListener();
    gfx::Rect rect = GetLocalBounds();
    rect.Inset(margins_);
    if (cooperate_view_)
    {
        gfx::Size cooperate_size = cooperate_view_->GetPreferredSize();
        switch (cooperate_dir_)
        {
        case BD_RIGHT:
            cooperate_view_->SetBounds(rect.right() - cooperate_size.width(), rect.y() + (rect.height() - cooperate_size.height()) / 2,
                cooperate_size.width(), cooperate_size.height());
            textfield_->SetBounds(rect.x(), rect.y(),
                rect.width() - kThickness - cooperate_size.width(), rect.height());
            break;
        case BD_LEFT:
            cooperate_view_->SetBounds(rect.x(), rect.y() + (height() - cooperate_size.height()) / 2,
                cooperate_size.width(), cooperate_size.height());
            textfield_->SetBounds(rect.x() + kThickness + cooperate_size.width(), rect.y(),
                rect.width() - kThickness - cooperate_size.width(), rect.height());
            break;
        default:
            break;
        }
    }
    else
    {
        textfield_->SetBounds(rect.x(), rect.y(), rect.width(), rect.height());
    }
}

void BililiveTextfield::OnDidChangeFocus(View* focused_before, View* focused_now)
{
    if (focused_before == textfield_ || focused_now == textfield_)
    {
        SchedulePaint();
    }
}

void BililiveTextfield::SetTextfield(views::Textfield* textfield)
{
    if (textfield_)
    {
        RemoveChildView(textfield_);
        delete textfield_;
        textfield_ = nullptr;
    }
    if (textfield)
    {
        textfield_ = textfield;
        textfield_->RemoveBorder();
        textfield_->SetBackgroundColor(SK_ColorWHITE);
        AddChildView(textfield_);
    }
    InvalidateLayout();
}

void BililiveTextfield::SetController(views::TextfieldController* controller)
{
    if (textfield_)
    {
        textfield_->SetController(controller);
    }
}

void BililiveTextfield::set_placeholder_text(const string16& text)
{
    if (textfield_)
    {
        textfield_->set_placeholder_text(text);
    }
}

void BililiveTextfield::set_placeholder_text_color(SkColor color)
{
    if (textfield_)
    {
        textfield_->set_placeholder_text_color(color);
    }
}

void BililiveTextfield::AddFocusChangeListener()
{
    if (!add_focus_listener_ && GetFocusManager())
    {
        add_focus_listener_ = true;
        GetFocusManager()->AddFocusChangeListener(this);
    }
}

void BililiveTextfield::RemoveFocusChangeListener()
{
    if (add_focus_listener_ && GetFocusManager())
    {
        GetFocusManager()->RemoveFocusChangeListener(this);
    }
}

void BililiveTextfield::OnMouseEntered(const ui::MouseEvent& event)
{
    SchedulePaint();
}

void BililiveTextfield::OnMouseExited(const ui::MouseEvent& event)
{
    SchedulePaint();
}


// AcceleratorsTextfield
AcceleratorsTextfield::AcceleratorsTextfield()
    : delegate_(NULL)
{
    SetReadOnly(true);
    initMember();
}

AcceleratorsTextfield::AcceleratorsTextfield(views::Textfield::StyleFlags style)
    : views::Textfield(style)
    , delegate_(NULL)
{
    SetReadOnly(true);
    initMember();
}

AcceleratorsTextfield::~AcceleratorsTextfield()
{}

void AcceleratorsTextfield::initMember()
{
    m_bValidSetup = false;
    m_bCtrl = m_bShift = m_bAlt = false;
    m_cChar = 0;
    m_bRealCtrl = m_bRealShift = m_bRealAlt = false;
    m_cRealChar = 0;
}

void AcceleratorsTextfield::ResetSettingKeys()
{
    m_bCtrl = m_bShift = m_bAlt = false;
    m_cChar = 0;
}

bool AcceleratorsTextfield::GetHotkey(UINT &nModify, UINT &nVk)
{
    nModify = nVk = 0;
    if (m_bValidSetup)
    {
        if (m_bRealCtrl)
        {
            nModify |= MOD_CONTROL;
        }
        if (m_bRealShift)
        {
            nModify |= MOD_SHIFT;
        }
        if (m_bRealAlt)
        {
            nModify |= MOD_ALT;
        }
        nVk = m_cRealChar;
    }
    return m_bValidSetup;
}

void AcceleratorsTextfield::SetHotkey(UINT nModify, UINT nVk)
{
    m_bRealCtrl = ((nModify & MOD_CONTROL) != 0);
    m_bRealShift = ((nModify & MOD_SHIFT) != 0);
    m_bRealAlt = ((nModify & MOD_ALT) != 0);
    m_cRealChar = static_cast<unsigned char>(nVk);
    m_bValidSetup = true;

    base::string16 cstr = GetAcceleratorsString(m_bRealCtrl, m_bRealShift, m_bRealAlt, m_cRealChar);

    SetText(cstr);
}

void AcceleratorsTextfield::DelHotKey()
{
    initMember();
    SetText(L"");
}

base::string16 AcceleratorsTextfield::GetAcceleratorsString(bool ctrl, bool shift, bool alt, unsigned char vk)
{
    base::string16 temp, vkey;
    if (ctrl)
    {
        temp += L"Ctrl + ";
    }
    if (shift)
    {
        temp += L"Shift + ";
    }
    if (alt)
    {
        temp += L"Alt + ";
    }
    if (CheckLetterKey(vk, &vkey))
    {
        temp += vkey;
    }
    return temp;
}

bool AcceleratorsTextfield::CheckLetterKey(unsigned char kc, base::string16 *pstr/* = NULL*/)
{
    bool bValid = false;
    base::string16 temp;
    if ((kc >= ui::KeyboardCode::VKEY_0) && (kc <= ui::KeyboardCode::VKEY_Z))
    {
        char ch = static_cast<char>(kc);
        base::UTF8ToUTF16(&ch, 1, &temp);
        bValid = true;
    }
    else
    {
        bValid = true;
        switch (kc)
        {
        case VK_INSERT:
            temp = L"Insert";
            break;
        case VK_HOME:
            temp = L"Home";
            break;
        case VK_END:
            temp = L"End";
            break;
        case VK_PRIOR:
            temp = L"Page Up";
            break;
        case VK_NEXT:
            temp = L"Page Down";
            break;
        case VK_SCROLL:
            temp = L"Scroll Lock";
            break;
        case VK_UP:
            temp = L"Up";
            break;
        case VK_LEFT:
            temp = L"Left";
            break;
        case VK_DOWN:
            temp = L"Down";
            break;
        case VK_RIGHT:
            temp = L"Right";
            break;
        case VK_NUMLOCK:
            temp = L"Num Lock";
            break;
        case VK_DIVIDE:
            temp = L"Num /";
            break;
        case VK_MULTIPLY:
            temp = L"Num *";
            break;
        case VK_SUBTRACT:
            temp = L"Num -";
            break;
        case VK_ADD:
            temp = L"Num +";
            break;
        case VK_NUMPAD0:
            temp = L"Num 0";
            break;
        case VK_NUMPAD1:
            temp = L"Num 1";
            break;
        case VK_NUMPAD2:
            temp = L"Num 2";
            break;
        case VK_NUMPAD3:
            temp = L"Num 3";
            break;
        case VK_NUMPAD4:
            temp = L"Num 4";
            break;
        case VK_NUMPAD5:
            temp = L"Num 5";
            break;
        case VK_NUMPAD6:
            temp = L"Num 6";
            break;
        case VK_NUMPAD7:
            temp = L"Num 7";
            break;
        case VK_NUMPAD8:
            temp = L"Num 8";
            break;
        case VK_NUMPAD9:
            temp = L"Num 9";
            break;
        case VK_DECIMAL:
            temp = L"Num Del";
            break;
        case VK_CAPITAL:
            temp = L"Caps Lock";
            break;
        case VK_OEM_MINUS:
            temp = L"-";
            break;
        case VK_OEM_PLUS:
            temp = L"+";
            break;
        case VK_OEM_1:
            temp = L";";
            break;
        case VK_OEM_2:
            temp = L"/";
            break;
        case VK_OEM_3:
            temp = L"`";
            break;
        case VK_OEM_4:
            temp = L"[";
            break;
        case VK_OEM_5:
            temp = L"\\";
            break;
        case VK_OEM_6:
            temp = L"]";
            break;
        case VK_OEM_7:
            temp = L"'";
            break;
        case VK_OEM_COMMA:
            temp = L",";
            break;
        case VK_OEM_PERIOD:
            temp = L".";
            break;
        case VK_F1:
            temp = L"F1";
            break;
        case VK_F2:
            temp = L"F2";
            break;
        case VK_F3:
            temp = L"F3";
            break;
        case VK_F4:
            temp = L"F4";
            break;
        case VK_F5:
            temp = L"F5";
            break;
        case VK_F6:
            temp = L"F6";
            break;
        case VK_F7:
            temp = L"F7";
            break;
        case VK_F8:
            temp = L"F8";
            break;
        case VK_F9:
            temp = L"F9";
            break;
        case VK_F10:
            temp = L"F10";
            break;
        case VK_F11:
            temp = L"F11";
            break;
        case VK_F12:
            temp = L"F12";
            break;
        default:
            bValid = false;
            break;
        }
    }
    if (bValid && pstr)
    {
        *pstr = temp;
    }
    return bValid;
}

bool AcceleratorsTextfield::CheckFunctionKey(unsigned char kc, base::string16 *pstr /*= NULL*/)
{
    bool bValid = true;
    base::string16 temp;

    switch (kc)
    {
    case VK_F1:
        temp = L"F1";
        break;
    case VK_F2:
        temp = L"F2";
        break;
    case VK_F3:
        temp = L"F3";
        break;
    case VK_F4:
        temp = L"F4";
        break;
    case VK_F5:
        temp = L"F5";
        break;
    case VK_F6:
        temp = L"F6";
        break;
    case VK_F7:
        temp = L"F7";
        break;
    case VK_F8:
        temp = L"F8";
        break;
    case VK_F9:
        temp = L"F9";
        break;
    case VK_F10:
        temp = L"F10";
        break;
    case VK_F11:
        temp = L"F11";
        break;
    case VK_F12:
        temp = L"F12";
        break;
    default:
        bValid = false;
        break;
    }

    if (bValid && pstr)
    {
        *pstr = temp;
    }
    return bValid;
}

void AcceleratorsTextfield::OnKeyEvent(ui::KeyEvent* event)
{
    ui::KeyboardCode kc = event->key_code();
    if (event->type() == ui::EventType::ET_KEY_PRESSED || event->type() == ui::EventType::ET_KEY_RELEASED)
    {
        // 无视某几个键位
        if ((kc == VK_PRINT) || (kc == VK_RETURN))
        {
            return;
        }
        // 清除键位
        if ((kc == VK_ESCAPE) || (kc == VK_DELETE) || (kc == VK_BACK) || (kc == VK_PAUSE)
            || (kc == VK_TAB) || (kc == VK_LWIN) || (kc == VK_RWIN) || (kc == VK_SPACE))
        {
            ResetSettingKeys();
            m_bValidSetup = false;
        }
        else
        {
            if (kc == ui::KeyboardCode::VKEY_CONTROL)
            {
                m_bCtrl = (event->type() == ui::EventType::ET_KEY_PRESSED);
            }
            if (kc == ui::KeyboardCode::VKEY_SHIFT)
            {
                m_bShift = (event->type() == ui::EventType::ET_KEY_PRESSED);
            }
            if (kc == ui::KeyboardCode::VKEY_MENU)
            {
                m_bAlt = (event->type() == ui::EventType::ET_KEY_PRESSED);
            }
            if (event->type() == ui::EventType::ET_KEY_PRESSED)
            {
                // 若先前已经完成过设定则重置字母键记录
                if (m_bValidSetup)
                {
                    m_cChar = 0;
                }
                // 按键新按视为还在设定中
                m_bValidSetup = false;
                if (CheckLetterKey(kc))
                {
                    m_cChar = kc;
                }
                // F1~F12功能键可以单设，其他的必须使用组合，没有组合的默认以CTRL+ALT进行组合
                if (!CheckFunctionKey(m_cChar))
                {
                    if (!m_bCtrl && !m_bShift && !m_bAlt)
                    {
                        m_bCtrl = m_bAlt = true;
                    }
                }
            }
            else
            {
                if (CheckLetterKey(kc))
                {
                    // 字母键释放时视为设定结束
                    m_bValidSetup = true;
                    m_bRealCtrl = m_bCtrl;
                    m_bRealShift = m_bShift;
                    m_bRealAlt = m_bAlt;
                    m_cRealChar = m_cChar;
                    // 设定结束清空缓存组合键
                    ResetSettingKeys();
                }
            }
        }

        // 根据当前设定是否已然结束来显示组合
        bool bCtrl = false, bShift = false, bAlt = false;
        unsigned char ch = 0;
        if (!m_bValidSetup)
        {
            bCtrl = m_bCtrl;
            bShift = m_bShift;
            bAlt = m_bAlt;
            ch = m_cChar;
        }
        else
        {
            bCtrl = m_bRealCtrl;
            bShift = m_bRealShift;
            bAlt = m_bRealAlt;
            ch = m_cRealChar;
        }

        // 快捷键组合转字符串
        base::string16 cstr = GetAcceleratorsString(bCtrl, bShift, bAlt, ch);

        SetText(cstr);

        if (m_bValidSetup)
        {
            if (delegate_)
            {
                UINT nModify, nVk;
                GetHotkey(nModify, nVk);
                delegate_->OnHotkeyChange(this, nModify, nVk);
            }
        }
    }
}