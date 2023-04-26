#include <olectl.h>

#include "win32_richedit_view.h"
#include "bililive_theme_common.h"

#include "ui/gfx/canvas.h"
#include "ui/gfx/font.h"
#include "ui/views/controls/native_control.h"
#include "ui/views/controls/native/native_view_host.h"

#include "bililive/bililive/utils/bililive_canvas_drawer.h"


namespace bililive
{
	class Win32RichEditViewImpl : public views::NativeControl
	{
	public:
		Win32RichEditViewImpl(Win32RichEditView* container);
		~Win32RichEditViewImpl();

		std::wstring GetText();
		void SetText(const std::wstring& text);

		HWND CreateNativeControl(HWND parent_container) OVERRIDE;
		bool enabled();
		void SetEnabled(bool val);

        LRESULT OnNotify(int w_param, LPNMHDR l_param) OVERRIDE;
        LRESULT OnCommand(UINT code, int id, HWND source) OVERRIDE;

        // View
        virtual gfx::NativeViewAccessible GetNativeViewAccessible() OVERRIDE{ return (gfx::NativeViewAccessible)richedit_hwnd_; }
        virtual bool SkipDefaultKeyEventProcessing(const ui::KeyEvent& event) OVERRIDE{
          return true;
        }

	protected:
		std::wstring before_creation_text_;
		Win32RichEditView* container_;
        HWND richedit_hwnd_;
	};

	Win32RichEditViewImpl::Win32RichEditViewImpl(Win32RichEditView* container)
		: container_(container)
        , richedit_hwnd_(NULL)
	{
	}

	Win32RichEditViewImpl::~Win32RichEditViewImpl()
	{
	}

	HWND Win32RichEditViewImpl::CreateNativeControl(HWND parent_container)
	{
        richedit_hwnd_ = CreateWindowExW(GetAdditionalExStyle(), L"BILIRICHEDIT", L"",
            WS_VISIBLE | WS_VSCROLL | WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL, 0, 0, 0, 0, parent_container, 0, GetModuleHandle(0), 0);
        if (richedit_hwnd_)
        {
            container_->OnCreateNativeControl(richedit_hwnd_);
        }

        return richedit_hwnd_;
	}

	bool Win32RichEditViewImpl::enabled()
	{
		return IsWindowEnabled(GetNativeControlHWND()) != FALSE;
	}

	void Win32RichEditViewImpl::SetEnabled(bool val)
	{
		EnableWindow(GetNativeControlHWND(), val ? TRUE : FALSE);
		NativeControl::SetEnabled(val);
	}

	LRESULT Win32RichEditViewImpl::OnNotify(int w_param, LPNMHDR l_param)
	{
		return container_->OnNotify(w_param, l_param);
	}

	LRESULT Win32RichEditViewImpl::OnCommand(UINT code, int id, HWND source)
	{
		if (source == GetNativeControlHWND())
		{
			return container_->OnCommand(code, id, source);
		}
		else
			return 0;
	}

	std::wstring Win32RichEditViewImpl::GetText()
	{
		HWND hwnd = GetNativeControlHWND();
		if (hwnd)
		{
			std::wstring r;
			int len = (int)SendMessageW(hwnd, WM_GETTEXTLENGTH, 0, 0);
			if (len > 0)
			{
				r.resize(len);
				SendMessageW(hwnd, WM_GETTEXT, (WPARAM)(len + 1), (LPARAM)&r[0]);
			}
			return std::move(r);
		}
		else
			return before_creation_text_;
	}

	void Win32RichEditViewImpl::SetText(const std::wstring& text)
	{
		HWND hwnd = GetNativeControlHWND();
		if (hwnd)
		{
			if (text.size() > 0)
				SendMessageW(hwnd, WM_SETTEXT, 0, (LPARAM)&text[0]);
			else
				SendMessageW(hwnd, WM_SETTEXT, 0, (LPARAM)L"");
		}
		else
			before_creation_text_ = text;
	}



    HMODULE Win32RichEditView::hModule_ = NULL;
    WNDPROC Win32RichEditView::RichEditProc_ = NULL;

    Win32RichEditView::Win32RichEditView(BOOL bReadonly/* = FALSE*/)
        : m_hWnd(NULL)
        , read_only_(bReadonly)
	{
        RegisterBiliRichEditClass();

		impl_ = new Win32RichEditViewImpl(this);

		border_ = new RoundRectView();
		border_->SetBorderColor(clrComboboxBorder);

		AddChildView(border_);
		AddChildView(impl_);
	}

	Win32RichEditView::~Win32RichEditView()
	{
	}

    void Win32RichEditView::OnCreateNativeControl(HWND hWnd)
    {
        m_hWnd = hWnd;
        SetBackgroundColor(RGB(255, 255, 255));
        SetReadOnly(read_only_);
    }

	gfx::Size Win32RichEditView::GetPreferredSize()
	{
		gfx::Size r = impl_->GetPreferredSize();
		r.set_width(r.width() + border_->GetCornerRadius() * 2);
		r.set_height(r.height() + border_->GetCornerRadius() * 2);
		return r;
	}

	std::wstring Win32RichEditView::GetText()
	{
		return impl_->GetText();
	}

	void Win32RichEditView::SetText(const std::wstring& text)
	{
		impl_->SetText(text);
	}

	bool Win32RichEditView::enabled()
	{
		return impl_->enabled();
	}

	void Win32RichEditView::SetEnabled(bool val)
	{
		impl_->SetEnabled(val);
	}

	LRESULT Win32RichEditView::OnNotify(int w_param, LPNMHDR l_param)
	{
		return 0;
	}

	LRESULT Win32RichEditView::OnCommand(UINT code, int id, HWND source)
	{
		/*if (listener_)
		{
			if (code == EN_CHANGE)
			{
				listener_->OnEditTextChanged(this, Win32EditListener::EDITEVENT_CHANGED);
			}
			else if (code == EN_KILLFOCUS)
			{
				border_->SetBorderColor(clrTextfieldBorder);
				listener_->OnEditTextChanged(this, Win32EditListener::EDITEVENT_FOCUSLOST);
			}
			else if (code == EN_SETFOCUS)
			{
				border_->SetBorderColor(clrTextfieldFocusBorder);
			}
		}*/
		return 0;
	}

	void Win32RichEditView::OnPaint(gfx::Canvas* canvas)
	{
	}

	void Win32RichEditView::Layout()
	{
		const gfx::Size& thisSize = size();
		border_->SetBounds(0, 0, thisSize.width(), thisSize.height());

		gfx::Rect availRect = border_->GetMaxRect();
		impl_->SetSize(gfx::Size(availRect.width(), availRect.height()));
		impl_->SetPosition(gfx::Point(availRect.x(), availRect.y()));
	}

    HMODULE Win32RichEditView::InitBiliWin32RichEdit20()
    {
        const wchar_t  *pszLibrary = L"MSFTEDIT.DLL";
        //if (!hModule_)
        {
#if (_ATL_NTDDI_MIN > NTDDI_WIN7)
            return(::LoadLibraryExW(pszLibrary, NULL, LOAD_LIBRARY_SEARCH_SYSTEM32));
#else
#ifndef _USING_V110_SDK71_
            // the LOAD_LIBRARY_SEARCH_SYSTEM32 flag for LoadLibraryExW is only supported if the DLL-preload fixes are installed, so
            // use LoadLibraryExW only if SetDefaultDllDirectories is available (only on Win8, or with KB2533623 on Vista and Win7)...
            /*IFDYNAMICGETCACHEDFUNCTION(L"kernel32.dll", SetDefaultDllDirectories, pfSetDefaultDllDirectories)
            {
                return(::LoadLibraryExW(pszLibrary, NULL, LOAD_LIBRARY_SEARCH_SYSTEM32));
            }*/

            // ...otherwise fall back to using LoadLibrary from the SYSTEM32 folder explicitly.
#endif
            wchar_t  wszLoadPath[MAX_PATH + 1];
            UINT rc = ::GetSystemDirectoryW(wszLoadPath, _countof(wszLoadPath));
            if (rc == 0 || rc >= _countof(wszLoadPath))
            {
                return NULL;
            }

            if (wszLoadPath[wcslen(wszLoadPath) - 1] != L'\\')
            {
                if (wcscat_s(wszLoadPath, _countof(wszLoadPath), L"\\") != 0)
                {
                    return NULL;
                }
            }

            if (wcscat_s(wszLoadPath, _countof(wszLoadPath), pszLibrary) != 0)
            {
                return NULL;
            }

            return(::LoadLibraryW(wszLoadPath));
#endif
        }
    }

    bool Win32RichEditView::RegisterBiliRichEditClass()
    {
        DWORD er = 0;
        if (!hModule_)
        {
            hModule_ = InitBiliWin32RichEdit20();
        }
        if (hModule_)
        {
            if (!RichEditProc_)
            {
                WNDCLASSW editClass = { 0 };
                if (GetClassInfoW(hModule_, /*L"RICHEDIT50W"*/MSFTEDIT_CLASS, &editClass))
                {
                    RichEditProc_ = editClass.lpfnWndProc;
                    editClass.lpfnWndProc = Win32RichEditView::BiliRichEditWndProc;
                    editClass.lpszClassName = L"BILIRICHEDIT";
                    RegisterClassW(&editClass);
                }
                else
                {
                    er = GetLastError();
                }
            }
        }
        return RichEditProc_ != NULL;
    }

    LRESULT CALLBACK Win32RichEditView::BiliRichEditWndProc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp)
    {
        LRESULT ret = 0;
        if (RichEditProc_)
        {
            switch (message)
            {
            case WM_USER:
                break;
            default:
                ret = RichEditProc_(hwnd, message, wp, lp);
                break;
            }
        }
        return ret;
    }
};
