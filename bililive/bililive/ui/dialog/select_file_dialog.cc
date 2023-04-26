#include "shell_dialogs.h"

#include <Windows.h>
#include <commdlg.h>
#include <Shlwapi.h>
#include <ShlObj.h>
#include <sstream>
#include <string>
#include <vector>
#include <cderr.h>

#include "base/strings/stringprintf.h"

#include "ui/views/widget/widget.h"
#include "ui/base/resource/resource_bundle.h"
#include "grit/generated_resources.h"

namespace bililive
{
    std::wstring GetOpenFileNameErrorMessage(DWORD dwErr)
    {
        if (0 == dwErr)
        {
            return L"";
        }

        std::wstring msg;
        switch (dwErr)
        {
        case FNERR_BUFFERTOOSMALL:
            // msg = L"所选择的文件路径过长！";
            msg = ResourceBundle::GetSharedInstance().GetLocalizedString(IDS_SELECT_FILE_DIALOG_PATH_TOO_LONG);
            break;
        case FNERR_INVALIDFILENAME:
            //msg = L"无效的文件路径！";
            msg = msg = ResourceBundle::GetSharedInstance().GetLocalizedString(IDS_SELECT_FILE_DIALOG_PATH_INVALID);
            break;
        default:
            // msg = base::StringPrintf(L"选择文件时发生错误 %d", dwErr);
            base::string16 fmt = ResourceBundle::GetSharedInstance().GetLocalizedString(IDS_SELECT_FILE_DIALOG_PATH_ERROR_FORMAT);
            msg = base::StringPrintf(fmt.c_str(), dwErr);
            break;
        }
        return msg;
    }

    std::wstring AppendExtListToDesc(const std::wstring& desc, const wchar_t* exts[])
    {
        const wchar_t** ext = exts;
        bool firstIter = true;
        std::wstringstream buf;
        buf << desc << L" (";
        while (*ext)
        {
            if (!firstIter)
                buf << ",";
            buf << L"*." << *ext;

            firstIter = false;
            ++ext;
        }
        buf << L")";

        return buf.str();
    }

    class SelectFileDialog : public ISelectFileDialog
    {
    private:
        views::Widget* parent_;

        std::wstringstream strFiltersBuf;

        std::wstring defaultPath_;
        std::vector<std::wstring> selectedFiles_;

        std::wstring error_message_;

    public:
        explicit SelectFileDialog(views::Widget* parent)
            : parent_(parent)
        {
        }

        ~SelectFileDialog()
        {
        }

        void AddFilter(const std::wstring& desc, const wchar_t* exts[]) override
        {
            auto _desc = AppendExtListToDesc(desc, exts);

            strFiltersBuf << _desc << L'\0';
            bool isFirst = true;
            while (*exts != 0)
            {
                if (isFirst == false)
                {
                    strFiltersBuf << L";";
                }
                strFiltersBuf << L"*." << *exts;
                isFirst = false;
                ++exts;
            }
            strFiltersBuf << L'\0';
        }

        void ClearFilter() override
        {
            strFiltersBuf = std::wstringstream();
        }

        void SetDefaultPath(const std::wstring& defaultPath) override
        {
            defaultPath_ = defaultPath;
        }

        bool DoModel(ui::SelectFileDialog::Type type = ui::SelectFileDialog::Type::SELECT_OPEN_FILE) override
        {
            if (type == ui::SelectFileDialog::Type::SELECT_OPEN_FILE ||
                type == ui::SelectFileDialog::Type::SELECT_OPEN_MULTI_FILE ||
                type == ui::SelectFileDialog::Type::SELECT_SAVEAS_FILE)
            {
                //std::vector<wchar_t> fileBuf(MAX_PATH * 2);
                strFiltersBuf << L'\0';
                std::wstring filter = strFiltersBuf.str();
                std::vector<wchar_t> filename(MAX_PATH * 1024);
                filename[0] = 0;

                OPENFILENAMEW ofn;
                // We must do this otherwise the ofn's FlagsEx may be initialized to random
                // junk in release builds which can cause the Places Bar not to show up!
                ZeroMemory(&ofn, sizeof(OPENFILENAMEW));
                ofn.lStructSize = sizeof(OPENFILENAMEW);
                if (parent_)
                {
                    ofn.hwndOwner = parent_->GetNativeWindow();
                }

                if (!filter.empty())
                {
                    ofn.lpstrFilter = filter.c_str();
                    ofn.nFilterIndex = 1;
                }

                if (!defaultPath_.empty())
                {
                    ofn.lpstrInitialDir = defaultPath_.c_str();
                }

                ofn.lpstrFile = filename.data();
                ofn.nMaxFile = filename.size();

                if (type == ui::SelectFileDialog::Type::SELECT_OPEN_FILE)
                {
                    ofn.Flags = OFN_NOCHANGEDIR| OFN_EXPLORER | OFN_FILEMUSTEXIST;
                }
                else if (type == ui::SelectFileDialog::Type::SELECT_OPEN_MULTI_FILE)
                {
                    ofn.Flags = OFN_NOCHANGEDIR | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT;
                }
                else if (type == ui::SelectFileDialog::Type::SELECT_SAVEAS_FILE)
                {
                    ofn.Flags = OFN_NOCHANGEDIR | OFN_EXPLORER | OFN_OVERWRITEPROMPT;
                }

                BOOL ret;
                if (type == ui::SelectFileDialog::Type::SELECT_OPEN_FILE || type == ui::SelectFileDialog::Type::SELECT_OPEN_MULTI_FILE)
                {
                    ret = ::GetOpenFileNameW(&ofn);
                }
                else if (type == ui::SelectFileDialog::Type::SELECT_SAVEAS_FILE)
                {
                    ret = ::GetSaveFileNameW(&ofn);
                }

                if (ret == TRUE)
                {
                    if (type == ui::SelectFileDialog::Type::SELECT_OPEN_FILE)
                    {
                        selectedFiles_.clear();
                        selectedFiles_.push_back(filename.data());
                    }
                    else if (type == ui::SelectFileDialog::Type::SELECT_OPEN_MULTI_FILE)
                    {
                        selectedFiles_.clear();
                        size_t startIndex = 0;
                        std::wstring folder;

                        while (startIndex < filename.size())
                        {
                            std::wstring tmp = filename.data() + startIndex;
                            if (tmp.empty())
                                break;
                            startIndex += tmp.size() + 1;
                            if (folder.empty())
                                folder = std::move(tmp);
                            else
                                selectedFiles_.push_back(folder + L'\\' + tmp);
                        }

                        if (selectedFiles_.empty())
                            selectedFiles_.push_back(folder);
                    }
                }
                else
                {
                    DWORD dwErr = ::CommDlgExtendedError();
                    if (type == ui::SelectFileDialog::Type::SELECT_OPEN_FILE)
                    {
                        error_message_ = GetOpenFileNameErrorMessage(dwErr);
                    }
                    else if (type == ui::SelectFileDialog::Type::SELECT_SAVEAS_FILE)
                    {
                    }
                }

                return (ret == TRUE);
            }
            else if (type == ui::SelectFileDialog::Type::SELECT_FOLDER)
            {
                std::vector<wchar_t> dirName(MAX_PATH * 2);

                BROWSEINFOW bi = { 0 };
                bi.hwndOwner = parent_->GetNativeWindow();
                bi.pszDisplayName = dirName.data();
                bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_EDITBOX;
                bi.lpfn = [](HWND hwnd, UINT msg, LPARAM lp, LPARAM data)->int
                {
                    if (msg == BFFM_INITIALIZED)
                    {
                        SendMessageW(hwnd, BFFM_SETSELECTION, (WPARAM)TRUE, (LPARAM)data);
                    }
                    return 0;
                };
                bi.lParam = (LPARAM)defaultPath_.data();

                PIDLIST_ABSOLUTE pa = SHBrowseForFolderW(&bi);
                if (pa != NULL && SHGetPathFromIDListW(pa, dirName.data()) == TRUE)
                {
                    selectedFiles_.clear();
                    selectedFiles_.push_back(dirName.data());
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                DCHECK(0);
                return false;
            }
        }

        std::wstring GetSelectedFileName() override
        {
            if (!selectedFiles_.empty())
                return *selectedFiles_.begin();
            else
                return{};
        }

        std::vector<std::wstring> GetSelectedFileNames() override
        {
            return selectedFiles_;
        }

        std::wstring GetErrorMessage() override
        {
            return error_message_;
        }
    };

    /*ISelectFileDialog* CreateSelectFileDialog(views::Widget* parent)
    {
        return new SelectFileDialog(parent);
    }*/
};
