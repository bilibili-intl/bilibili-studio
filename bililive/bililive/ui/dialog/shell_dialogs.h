#ifndef BILILIVE_SHELL_DIALOG_H_
#define BILILIVE_SHELL_DIALOG_H_

#include <string>

#include "third_party/skia/include/core/SkColor.h"
#include "ui/shell_dialogs/select_file_dialog.h"


namespace ui
{
    class ShellFileDialog;
}

namespace views
{
    class Widget;
}

namespace bililive
{
    class ISelectFileDialog
    {
    public:
        virtual ~ISelectFileDialog() {}

        // exts 是以空指针元素结尾的数组
        virtual void AddFilter(const std::wstring& desc, const wchar_t* exts[]) = 0;
        virtual void ClearFilter() = 0;
        virtual void SetDefaultPath(const std::wstring& defaultPath) = 0;
        virtual std::wstring GetSelectedFileName() = 0;
        virtual std::vector<std::wstring> GetSelectedFileNames() = 0;
        virtual bool DoModel(ui::SelectFileDialog::Type type = ui::SelectFileDialog::Type::SELECT_OPEN_FILE) = 0;
        virtual std::wstring GetErrorMessage() = 0;
    };

    ISelectFileDialog* CreateSelectFileDialog(views::Widget* parent = nullptr);


    class SelectColorDialog
    {
    public:
        enum Result
        {
            RESULT_OK,
            RESULT_CANCEL
        };

        SelectColorDialog();
        ~SelectColorDialog();
        void SetCurrentColor(SkColor color);
        SkColor GetCurrentColor();
        Result DoModel(views::Widget* parent = nullptr);

    private:
        SkColor color_;
    };


    class SelectFontDialog
    {
    public:
        enum Result
        {
            RESULT_OK,
            RESULT_CANCEL
        };

        SelectFontDialog();
        ~SelectFontDialog();
        void SetCurrentFont(const std::wstring& fontName, int height, bool isBold, bool isItalic);

        template<class T1, class T2, class T3, class T4>
        void GetCurrentFont(T1& fontName, T2& height, T3& isBold, T4& isItalic)
        {
            fontName = fontName_;
            height = height_;
            isBold = isBold_;
            isItalic = isItalic_;
        }

        Result DoModel(views::Widget* parent = nullptr);

    private:
        std::wstring fontName_;
        int height_;
        bool isBold_;
        bool isItalic_;
    };

}

#endif
