#include <olectl.h>
#include <commctrl.h>

#include "win32_datetimepicker_view.h"
#include "bililive_theme_common.h"

#include "ui/gfx/canvas.h"
#include "ui/gfx/font.h"
#include "ui/views/controls/native_control.h"
#include "ui/views/controls/native/native_view_host.h"

#include "bililive/bililive/utils/gdiplus/bililive_gdiplus_utils.h"
#include "bililive/bililive/utils/bililive_canvas_drawer.h"
#include "bililive/public/common/bililive_paths.h"
#include "bililive/bililive/ui/views/uploader/bililive_uploader_type.h"

#include "base/path_service.h"
#include "base/files/file_stream_win.h"
#include "base/threading/thread_restrictions.h"
#include "base/strings/utf_string_conversions.h"
#include "base/guid.h"



std::map<HWND, Win32DateTimePickerView*> Win32DateTimePickerView::instance_map_;
WNDPROC Win32DateTimePickerView::def_wndproc_ = nullptr;

Win32DateTimePickerView::Win32DateTimePickerView(Win32DateTimePickerDelegate *delegate)
    : delegate_(delegate)
    , m_hWnd(nullptr)
    , hfont_(nullptr)
    , border_normal_color_(bililive_uploader::clrGrayBorder)
    , border_hover_color_(bililive_uploader::clrPinkBorder)
    , style_(0)
{
    //LONG fRegisteredClasses = 0;
    INITCOMMONCONTROLSEX init;
    init.dwSize = sizeof(init);
    init.dwICC = ICC_DATE_CLASSES;
    InitCommonControlsEx(&init);

    set_background(views::Background::CreateSolidBackground(SK_ColorWHITE));
}

Win32DateTimePickerView::~Win32DateTimePickerView()
{
}

HWND Win32DateTimePickerView::CreateNativeControl(HWND parent_container)
{
    DWORD dwStyle = WS_VISIBLE | WS_CHILD/* | MCS_NOTODAY*/;
    m_hWnd = CreateWindowExW(GetAdditionalExStyle(), L"SysDateTimePick32", L"SysDateTimePick32",
        dwStyle, 0, 0, 0, 0, parent_container, 0, GetModuleHandle(NULL), 0);

    if (m_hWnd)
    {
        if (def_wndproc_ == nullptr)
        {
            def_wndproc_ = (WNDPROC)::GetWindowLong(m_hWnd, GWL_WNDPROC);
        }

        instance_map_[m_hWnd] = this;
        ::SetWindowLong(m_hWnd, GWL_WNDPROC, (LONG)Win32DateTimePickerView::Win32DateTimePickerViewWndProc);

        SetFont(gfx::Font().GetNativeFont());

        InitView();
    }

    return m_hWnd;
}

void Win32DateTimePickerView::OnDestroy()
{
    instance_map_.erase(m_hWnd);
}

LRESULT Win32DateTimePickerView::OnNotify(int w_param, LPNMHDR l_param)
{
    switch (l_param->code)
    {
    case DTN_DROPDOWN:
        if (delegate_)
        {
            delegate_->OnDtpDropDown(this);
        }
        break;
    case DTN_DATETIMECHANGE:
        if (delegate_)
        {
            delegate_->OnDtpDateTimeChange(this);
        }
        break;
    case DTN_CLOSEUP:
        if (delegate_)
        {
            delegate_->OnDtpCloseup(this);
        }
        break;
    default:
        break;
    }
    return S_OK;
}

LRESULT Win32DateTimePickerView::OnReflectMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LRESULT result = 0;
    bHandled = FALSE;
    switch (uMsg)
    {
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORMSGBOX:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG:
    {
        bHandled = TRUE;
        result = (LRESULT)::CreateSolidBrush(RGB(255, 255, 255));
    }
    break;
    default:
        break;
    }
    return result;
}

void Win32DateTimePickerView::InitView()
{
    if (hfont_)
    {
        SetFont(hfont_);
    }
    if (init_systime_.had_set_value)
    {
        SetTime(&init_systime_.systime);
    }
    if (min_systime_.had_set_value && max_systime_.had_set_value)
    {
        SetRange(&min_systime_.systime, &max_systime_.systime);
    }
    if (!format_.empty())
    {
        SetFormat(format_.c_str());
    }
    if (style_)
    {
        SetMonthCalStyle(style_);
    }
}

gfx::Size Win32DateTimePickerView::GetPreferredSize()
{
    static gfx::Size pref_size;
    if (pref_size.IsEmpty())
    {
        gfx::Font ft = GetFont();
        pref_size = gfx::Size(ft.GetStringWidth(L"--9999-99-99") + 40, ft.GetHeight() + 10);
    }
    return pref_size;
}

void Win32DateTimePickerView::OnEnabledChanged()
{
    if (::IsWindow(m_hWnd))
    {
        ::EnableWindow(m_hWnd, enabled());
    }
}

void Win32DateTimePickerView::OnFocus()
{
    if (::IsWindow(m_hWnd))
    {
        ::SetFocus(m_hWnd);
    }
}

// Sets the time in the datetime picker control.
BOOL Win32DateTimePickerView::SetTime(_In_ LPSYSTEMTIME pTimeNew)
{
    init_systime_.SetTime(pTimeNew);

    if (::IsWindow(m_hWnd))
    {
        WPARAM wParam = (pTimeNew == NULL) ? GDT_NONE : GDT_VALID;
        return (BOOL) ::SendMessage(m_hWnd, DTM_SETSYSTEMTIME, wParam, (LPARAM)pTimeNew);
    }
    return FALSE;
}

// Retrieves the currently selected time from the datetime picker control.
DWORD Win32DateTimePickerView::GetTime(_Out_ LPSYSTEMTIME pTimeDest) const
{
    DCHECK(pTimeDest != NULL);
    DCHECK(::IsWindow(m_hWnd));
    return (DWORD) ::SendMessage(m_hWnd, DTM_GETSYSTEMTIME, 0, (LPARAM)pTimeDest);
}

// Sets the minimum and maximum allowable times for the datetime picker control.
BOOL Win32DateTimePickerView::SetRange(_In_ const LPSYSTEMTIME pMinRange, _In_ const LPSYSTEMTIME pMaxRange)
{
    SYSTEMTIME sysTimes[2] = { 0 };

    WPARAM wFlags = 0;
    if (pMinRange != NULL)
    {
        min_systime_.SetTime(pMinRange);
        wFlags |= GDTR_MIN;
        memcpy_s(&sysTimes[0], sizeof(SYSTEMTIME), pMinRange, sizeof(SYSTEMTIME));
    }

    if (pMaxRange != NULL)
    {
        max_systime_.SetTime(pMaxRange);
        wFlags |= GDTR_MAX;
        memcpy_s(&sysTimes[1], sizeof(SYSTEMTIME), pMaxRange, sizeof(SYSTEMTIME));
    }

    if (::IsWindow(m_hWnd) && pMinRange && pMaxRange)
    {
        return (BOOL) ::SendMessage(m_hWnd, DTM_SETRANGE, wFlags, (LPARAM)sysTimes);
    }
    return FALSE;
}

// Retrieves the current minimum and maximum allowable times for the datetime picker control.
DWORD Win32DateTimePickerView::GetRange(_Out_ LPSYSTEMTIME pMinRange, _Out_ LPSYSTEMTIME pMaxRange) const
{
    DCHECK(::IsWindow(m_hWnd));
    SYSTEMTIME sysTimes[2];
    memset(sysTimes, 0, sizeof(sysTimes));

    DWORD dwResult = DWORD(::SendMessage(m_hWnd, DTM_GETRANGE, 0, (LPARAM)sysTimes));

    if (pMinRange != NULL)
    {
        if (dwResult & GDTR_MIN)
        {
            memcpy_s(pMinRange, sizeof(SYSTEMTIME), &sysTimes[0], sizeof(SYSTEMTIME));
        }
    }

    if (pMaxRange != NULL)
    {
        if (dwResult & GDTR_MAX)
        {
            memcpy_s(pMaxRange, sizeof(SYSTEMTIME), &sysTimes[1], sizeof(SYSTEMTIME));
        }
    }

    return dwResult;
}

void Win32DateTimePickerView::SetBorderColor(SkColor clrBlur, SkColor clrFocus)
{
    border_normal_color_ = clrBlur;
    border_hover_color_ = clrFocus;
    if (::IsWindow(m_hWnd))
    {
        SchedulePaint();
    }
}

void Win32DateTimePickerView::SetFont(HFONT hFont, BOOL bRedraw/* = FALSE*/)
{
    hfont_ = hFont;

    if (::IsWindow(m_hWnd))
    {
        ::SendMessage(m_hWnd, WM_SETFONT, (WPARAM)hFont, bRedraw);
    }
}

void Win32DateTimePickerView::SetFont(const gfx::Font *font, bool bRedraw/* = false*/)
{
    hfont_ = font->GetNativeFont();
    SetFont(hfont_, bRedraw ? TRUE : FALSE);
}

gfx::Font Win32DateTimePickerView::GetFont()
{
    if (::IsWindow(m_hWnd))
    {
        return gfx::Font((HFONT)::SendMessage(m_hWnd, WM_GETFONT, 0, 0));
    }
    if (hfont_)
    {
        return gfx::Font(hfont_);
    }
    return gfx::Font();
}

BOOL Win32DateTimePickerView::SetFormat(_In_z_ LPCTSTR pstrFormat)
{
    format_ = pstrFormat;

    if (::IsWindow(m_hWnd))
    {
        return (BOOL) ::SendMessage(m_hWnd, DTM_SETFORMAT, 0, (LPARAM)pstrFormat);
    }
    return FALSE;
}

#if (NTDDI_VERSION >= NTDDI_VISTA) && defined(UNICODE)
DWORD Win32DateTimePickerView::SetMonthCalStyle(_In_ DWORD dwStyle)
{
    style_ = dwStyle;

    if (::IsWindow(m_hWnd))
    {
        return (DWORD)DateTime_SetMonthCalStyle(m_hWnd, style_);
    }
    return 0;
} // DTM_SETMCSTYLE

DWORD Win32DateTimePickerView::GetMonthCalStyle() const
{
    if (::IsWindow(m_hWnd))
    {
        return (DWORD)DateTime_GetMonthCalStyle(m_hWnd);
    }
    return 0;
} // DTM_GETMCSTYLE
#endif // (NTDDI_VERSION >= NTDDI_VISTA) && defined(UNICODE)

void Win32DateTimePickerView::OnPaint()
{
    if (::IsWindow(m_hWnd))
    {
        GdiplusImage *image = NULL;
        SkColor clr = SK_ColorWHITE;
        if (m_hWnd == ::GetFocus())
        {
            clr = border_hover_color_;
        }
        else
        {
            clr = border_normal_color_;
        }
        if (IsMouseHovered())
        {
            image = drop_hovered_image_.get();
        }
        else
        {
            image = drop_image_.get();
        }

        HDC dc = ::GetDC(m_hWnd);

        RECT rect = { 0 };
        GetClientRect(m_hWnd, &rect);
        
        // ÕÚ¸ÇÍâ²ã±ß¿ò
        ::FrameRect(dc, &rect, (HBRUSH)::GetStockObject(WHITE_BRUSH));
        // ÕÚ¸ÇÄÚ²ã±ß¿ò
        ::InflateRect(&rect, -1, -1);
        ::FrameRect(dc, &rect, (HBRUSH)::GetStockObject(WHITE_BRUSH));

        GdiplusCanvas canvas(dc);
        if (image && image->IsValid())
        {
            // ÕÚ¸ÇÏÂÀ­°´Å¥
            static int btn_cx = 33;
            RECT thumb_rect = { rect.right - btn_cx, rect.top, rect.right, rect.bottom };
            ::FillRect(dc, &thumb_rect, (HBRUSH)::GetStockObject(WHITE_BRUSH));

            // »æÖÆ°´Å¥£¬ÔÙ»æÖÆ±ß¿ò
            canvas.DrawImage(image, thumb_rect.left, thumb_rect.top, btn_cx, thumb_rect.bottom - thumb_rect.top);
        }
        canvas.DrawRoundRectange(clr, 0, 0, width() - 1, height() - 1, 4);

        ::ReleaseDC(m_hWnd, dc);
    }
}

LRESULT CALLBACK Win32DateTimePickerView::Win32DateTimePickerViewWndProc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp)
{
    LRESULT ret = ::CallWindowProc(def_wndproc_, hwnd, message, wp, lp);
    if (instance_map_.find(hwnd) != instance_map_.end())
    {
        Win32DateTimePickerView *view = instance_map_[hwnd];
        if (view)
        {
            switch (message)
            {
            case WM_PAINT:
                view->OnPaint();
                break;
            default:
                break;
            }
        }
    }
    return ret;
}

void Win32DateTimePickerView::SetDropButtonImage(const gfx::Image *normal, const gfx::Image *hovered)
{
    if (normal)
    {
        scoped_refptr<base::RefCountedMemory> mem = normal->As1xPNGBytes();
        if (mem)
        {
            drop_image_.reset(new GdiplusImage());
            drop_image_->LoadFromBuffer((LPVOID)mem->front(), mem->size());
        }
    }
    if (hovered)
    {
        scoped_refptr<base::RefCountedMemory> mem = hovered->As1xPNGBytes();
        if (mem)
        {
            drop_hovered_image_.reset(new GdiplusImage());
            drop_hovered_image_->LoadFromBuffer((LPVOID)mem->front(), mem->size());
        }
    }
    SchedulePaint();
}