#ifndef WIN32_DATETIMEPICKER_VIEW_H_
#define WIN32_DATETIMEPICKER_VIEW_H_

#include <windows.h>

#include "ui/gfx/font.h"

#include "ui/views/View.h"
#include "ui/views/controls/native_control.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/gfx/image/image.h"

#include "bililive/bililive/utils/gdiplus/bililive_gdiplus_image.h"

class Win32DateTimePickerView;
class Win32DateTimePickerDelegate
{
public:
    virtual void OnDtpDropDown(Win32DateTimePickerView *dtp){}
    virtual void OnDtpDateTimeChange(Win32DateTimePickerView *dtp) = 0;
    virtual void OnDtpCloseup(Win32DateTimePickerView *dtp){}
};

class Win32DateTimePickerView : public views::NativeControl
{
    struct DateTimePickerTime
    {
        DateTimePickerTime()
            : had_set_value(false)
        {
            memset(&systime, 0, sizeof(SYSTEMTIME));
        }

        void SetTime(_In_ LPSYSTEMTIME pTimeNew)
        {
            had_set_value = true;
            if (pTimeNew)
            {
                memcpy_s(&systime, sizeof(SYSTEMTIME), pTimeNew, sizeof(SYSTEMTIME));
            }
            else
            {
                memset(&systime, 0, sizeof(SYSTEMTIME));
            }
        }

        bool had_set_value;
        SYSTEMTIME systime;
    };

public:
    explicit Win32DateTimePickerView(Win32DateTimePickerDelegate *delegate);
    ~Win32DateTimePickerView();

    void SetDropButtonImage(const gfx::Image *normal, const gfx::Image *hovered);
    void SetBorderColor(SkColor clrBlur, SkColor clrFocus);
    void SetFont(const gfx::Font *font, bool bRedraw = false);
    gfx::Font GetFont();

    void SetFont(HFONT hFont, BOOL bRedraw = FALSE);

    // Sets the time in the datetime picker control.
    BOOL SetTime(_In_ LPSYSTEMTIME pTimeNew);

    // Retrieves the currently selected time from the datetime picker control.
    DWORD GetTime(_Out_ LPSYSTEMTIME pTimeDest) const;

    // Sets the minimum and maximum allowable times for the datetime picker control.
    BOOL SetRange(_In_ const LPSYSTEMTIME pMinRange, _In_ const LPSYSTEMTIME pMaxRange);

    // Retrieves the current minimum and maximum allowable times for the datetime picker control.
    DWORD GetRange(_Out_ LPSYSTEMTIME pMinRange, _Out_ LPSYSTEMTIME pMaxRange) const;

    // Sets the display of the datetime picker control based on the specified format string.
    BOOL SetFormat(_In_z_ LPCTSTR pstrFormat);

#if (NTDDI_VERSION >= NTDDI_VISTA) && defined(UNICODE)
    // REVIEW: Sets the style of the datetime picker control's child calendar control.
    DWORD SetMonthCalStyle(_In_ DWORD dwStyle);

    // REVIEW: Retrieves the style of the datetime picker control's child calendar control.
    DWORD GetMonthCalStyle() const;

    //// Retrieves information from the datetime picker control.
    //BOOL GetDateTimePickerInfo(_Out_ LPDATETIMEPICKERINFO pDateTimePickerInfo) const;

    //// Retrieves the ideal size for the control (so that all the text fits).
    //BOOL GetIdealSize(_Out_ LPSIZE pSize) const;
#endif // (NTDDI_VERSION >= NTDDI_VISTA) && defined(UNICODE)

protected:
    // NativeControl
    HWND CreateNativeControl(HWND parent_container) override;
    void OnDestroy() override;
    LRESULT OnNotify(int w_param, LPNMHDR l_param) override;
    LRESULT OnReflectMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;

    // View
    gfx::NativeViewAccessible GetNativeViewAccessible() override{ return (gfx::NativeViewAccessible)m_hWnd; }
    gfx::Size GetPreferredSize() override;
    void OnEnabledChanged() override;
    void OnFocus() override;

private:
    void InitView();

private:
    static WNDPROC def_wndproc_;
    static std::map<HWND, Win32DateTimePickerView*> instance_map_;
    static LRESULT CALLBACK Win32DateTimePickerViewWndProc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp);

    void OnPaint();

private:
    Win32DateTimePickerDelegate *delegate_;
    HWND m_hWnd;
    HFONT hfont_;
    std::wstring format_;
    DWORD style_;
    DateTimePickerTime init_systime_;
    DateTimePickerTime min_systime_;
    DateTimePickerTime max_systime_;
    SkColor border_normal_color_;
    SkColor border_hover_color_;
    scoped_ptr<GdiplusImage> drop_image_;
    scoped_ptr<GdiplusImage> drop_hovered_image_;

    DISALLOW_COPY_AND_ASSIGN(Win32DateTimePickerView);
};

#endif
