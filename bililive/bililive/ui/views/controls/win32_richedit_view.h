#ifndef WIN32_RICHEDIT_VIEW_H_
#define WIN32_RICHEDIT_VIEW_H_

#include "ui/views/View.h"
#include <string>
#include <Richedit.h>
#include "bililive_roundrect_view.h"

namespace bililive
{
	//class Win32RichEditListener;
	class Win32RichEditViewImpl;
	//class RoundRectView;

	class Win32RichEditView : public views::View
	{
	public:
        explicit Win32RichEditView(BOOL bReadonly = FALSE);
        virtual ~Win32RichEditView();

        // VIEW
        gfx::Size GetPreferredSize() OVERRIDE;
        void OnPaint(gfx::Canvas* canvas) OVERRIDE;
        void Layout() OVERRIDE;
        virtual gfx::NativeViewAccessible GetNativeViewAccessible() OVERRIDE{ return (gfx::NativeViewAccessible)m_hWnd; }

		std::wstring GetText();
		void SetText(const std::wstring& text);

        HWND GetSafeHwnd() const { return m_hWnd; }
        void SetBorderColor(SkColor color) { border_->SetBorderColor(color); }

		bool enabled();
		void SetEnabled(bool val);

		//下层的Native Control传上来的，处理原生EDIT控件用
		LRESULT OnNotify(int w_param, LPNMHDR l_param);
		LRESULT OnCommand(UINT code, int id, HWND source);
        
        // CRichEdit
        void GetSel(LONG& nStartChar, LONG& nEndChar) const
        {
            DCHECK(::IsWindow(m_hWnd));
            CHARRANGE cr = { 0, 0 };
            ::SendMessage(m_hWnd, EM_EXGETSEL, 0, (LPARAM)&cr);
            nStartChar = cr.cpMin;
            nEndChar = cr.cpMax;
        }

        void GetSel(CHARRANGE &cr) const
        {
            DCHECK(::IsWindow(m_hWnd));
            ::SendMessage(m_hWnd, EM_EXGETSEL, 0, (LPARAM)&cr);
        }

        int SetSel(LONG nStartChar, LONG nEndChar)
        {
            DCHECK(::IsWindow(m_hWnd));
            CHARRANGE cr = { nStartChar, nEndChar };
            return (int)::SendMessage(m_hWnd, EM_EXSETSEL, 0, (LPARAM)&cr);
        }

        int SetSel(CHARRANGE &cr)
        {
            DCHECK(::IsWindow(m_hWnd));
            return (int)::SendMessage(m_hWnd, EM_EXSETSEL, 0, (LPARAM)&cr);
        }

        int SetSelAll()
        {
            return SetSel(0, -1);
        }

        int SetSelNone()
        {
            return SetSel(-1, 0);
        }

        BOOL SetCharFormat(CHARFORMAT& cf, WORD wFlags)
        {
            DCHECK(::IsWindow(m_hWnd));
            cf.cbSize = sizeof(CHARFORMAT);
            return (BOOL)::SendMessage(m_hWnd, EM_SETCHARFORMAT, (WPARAM)wFlags, (LPARAM)&cf);
        }

        BOOL SetDefaultCharFormat(CHARFORMAT& cf)
        {
            DCHECK(::IsWindow(m_hWnd));
            cf.cbSize = sizeof(CHARFORMAT);
            return (BOOL)::SendMessage(m_hWnd, EM_SETCHARFORMAT, 0, (LPARAM)&cf);
        }

        BOOL SetSelectionCharFormat(CHARFORMAT& cf)
        {
            DCHECK(::IsWindow(m_hWnd));
            cf.cbSize = sizeof(CHARFORMAT);
            return (BOOL)::SendMessage(m_hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
        }

        BOOL SetWordCharFormat(CHARFORMAT& cf)
        {
            DCHECK(::IsWindow(m_hWnd));
            cf.cbSize = sizeof(CHARFORMAT);
            return (BOOL)::SendMessage(m_hWnd, EM_SETCHARFORMAT, SCF_SELECTION | SCF_WORD, (LPARAM)&cf);
        }

        int GetTextLength() const
        {
            DCHECK(::IsWindow(m_hWnd));
            return (int)::SendMessage(m_hWnd, WM_GETTEXTLENGTH, 0, 0L);
        }

        BOOL SetReadOnly(BOOL bReadOnly = TRUE)
        {
            DCHECK(::IsWindow(m_hWnd));
            read_only_ = bReadOnly;
            return (BOOL)::SendMessage(m_hWnd, EM_SETREADONLY, bReadOnly, 0L);
        }

        int GetFirstVisibleLine() const
        {
            DCHECK(::IsWindow(m_hWnd));
            return (int)::SendMessage(m_hWnd, EM_GETFIRSTVISIBLELINE, 0, 0L);
        }

        int GetLineCount() const
        {
            DCHECK(::IsWindow(m_hWnd));
            return (int)::SendMessage(m_hWnd, EM_GETLINECOUNT, 0, 0L);
        }

        DWORD GetSelectionCharFormat(CHARFORMAT2& cf) const
        {
            DCHECK(::IsWindow(m_hWnd));
            cf.cbSize = sizeof(CHARFORMAT2);
            return (DWORD)::SendMessage(m_hWnd, EM_GETCHARFORMAT, 1, (LPARAM)&cf);
        }

        BOOL SetSelectionCharFormat(CHARFORMAT2& cf)
        {
            DCHECK(::IsWindow(m_hWnd));
            cf.cbSize = sizeof(CHARFORMAT2);
            return (BOOL)::SendMessage(m_hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
        }

        DWORD GetParaFormat(PARAFORMAT2& pf) const
        {
            DCHECK(::IsWindow(m_hWnd));
            pf.cbSize = sizeof(PARAFORMAT2);
            return (DWORD)::SendMessage(m_hWnd, EM_GETPARAFORMAT, 0, (LPARAM)&pf);
        }

        BOOL SetParaFormat(PARAFORMAT2& pf)
        {
            DCHECK(::IsWindow(m_hWnd));
            pf.cbSize = sizeof(PARAFORMAT2);
            return (BOOL)::SendMessage(m_hWnd, EM_SETPARAFORMAT, 0, (LPARAM)&pf);
        }

        BOOL SetAutoURLDetect(BOOL bAutoDetect = TRUE)
        {
            DCHECK(::IsWindow(m_hWnd));
            return !(BOOL)::SendMessage(m_hWnd, EM_AUTOURLDETECT, bAutoDetect, 0L);
        }

        // this method is deprecated, please use SetAutoURLDetect
        BOOL EnableAutoURLDetect(BOOL bEnable = TRUE) { return SetAutoURLDetect(bEnable); }

        void ReplaceSel(LPCTSTR lpszNewText, BOOL bCanUndo = FALSE)
        {
            DCHECK(::IsWindow(m_hWnd));
            ::SendMessage(m_hWnd, EM_REPLACESEL, (WPARAM)bCanUndo, (LPARAM)lpszNewText);
        }

        LONG StreamIn(UINT uFormat, EDITSTREAM& es)
        {
            DCHECK(::IsWindow(m_hWnd));
            return (LONG)::SendMessage(m_hWnd, EM_STREAMIN, uFormat, (LPARAM)&es);
        }

        LONG StreamOut(UINT uFormat, EDITSTREAM& es)
        {
            DCHECK(::IsWindow(m_hWnd));
            return (LONG)::SendMessage(m_hWnd, EM_STREAMOUT, uFormat, (LPARAM)&es);
        }

        void ShowScrollBar(int nBarType, BOOL bVisible = TRUE)
        {
            DCHECK(::IsWindow(m_hWnd));
            ::SendMessage(m_hWnd, EM_SHOWSCROLLBAR, nBarType, bVisible);
        }

        int AppendText(LPCTSTR lpstrText, BOOL bCanUndo = FALSE)
        {
            return InsertText(GetWindowTextLength(m_hWnd), lpstrText, bCanUndo);
        }

        int InsertText(long nInsertAfterChar, LPCTSTR lpstrText, BOOL bCanUndo = FALSE)
        {
            int nRet = SetSel(nInsertAfterChar, nInsertAfterChar);
            ReplaceSel(lpstrText, bCanUndo);
            return nRet;
        }

        COLORREF SetBackgroundColor(COLORREF cr)
        {
            DCHECK(::IsWindow(m_hWnd));
            return (COLORREF)::SendMessage(m_hWnd, EM_SETBKGNDCOLOR, 0, cr);
        }

        void Clear()
        {
            DCHECK(::IsWindow(m_hWnd));
            if (!read_only_)
            {
                ::SendMessage(m_hWnd, WM_CLEAR, 0, 0L);
            }
            else
            {
                SetSel(0, -1);
                ReplaceSel(L"");
            }
        }

        int LineIndex(int nLine = -1) const
        {
            DCHECK(::IsWindow(m_hWnd));
            return (int)::SendMessage(m_hWnd, EM_LINEINDEX, nLine, 0L);
        }

        int LineLength(int nLine = -1) const
        {
            DCHECK(::IsWindow(m_hWnd));
            return (int)::SendMessage(m_hWnd, EM_LINELENGTH, nLine, 0L);
        }

        BOOL LineScroll(int nLines, int nChars = 0)
        {
            DCHECK(::IsWindow(m_hWnd));
            return (BOOL)::SendMessage(m_hWnd, EM_LINESCROLL, nChars, nLines);
        }

        void ScrollToBottom()
        {
            DCHECK(::IsWindow(m_hWnd));
            ::SendMessage(m_hWnd, WM_VSCROLL, SB_BOTTOM, 0);
        }

        void ScrollToTop()
        {
            DCHECK(::IsWindow(m_hWnd));
            ::SendMessage(m_hWnd, WM_VSCROLL, SB_TOP, 0);
        }

        // Additional operations
        void ScrollCaret()
        {
            DCHECK(::IsWindow(m_hWnd));
            ::SendMessage(m_hWnd, EM_SCROLLCARET, 0, 0L);
        }


    private:
        static bool RegisterBiliRichEditClass();
        static HMODULE InitBiliWin32RichEdit20();
        static LRESULT CALLBACK BiliRichEditWndProc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp);
        static HMODULE hModule_;
        static WNDPROC RichEditProc_;

        void OnCreateNativeControl(HWND hWnd);

	protected:
        friend class Win32RichEditViewImpl;

		RoundRectView* border_;
        Win32RichEditViewImpl* impl_;
        HWND m_hWnd;
        BOOL read_only_;

    private:
        DISALLOW_COPY_AND_ASSIGN(Win32RichEditView);
	};
};

#endif
