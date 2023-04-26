#include "shell_dialogs.h"

#include <ShlObj.h>

#include "base/win/scoped_comptr.h"

#include "ui/views/widget/widget.h"


namespace bililive {

    class SelectFileDialogEx : public ISelectFileDialog {
    public:
        explicit SelectFileDialogEx(views::Widget* parent)
            : parent_(parent) {}

        void AddFilter(const std::wstring& desc, const wchar_t* exts[]) override {
            std::wstring exts_str;
            bool is_first = true;
            while (*exts) {
                if (!is_first) {
                    exts_str.append(L";");
                }
                exts_str.append(L"*.").append(*exts);
                is_first = false;
                ++exts;
            }
            exts_.push_back({ desc, std::move(exts_str) });
        }

        void ClearFilter() override {
            exts_.clear();
        }

        void SetDefaultPath(const std::wstring& defaultPath) override {
            default_path_ = defaultPath;
        }

        bool DoModel(ui::SelectFileDialog::Type type) override {
            error_message_.clear();
            selected_files_.clear();

            switch (type) {
            case ui::SelectFileDialog::SELECT_OPEN_FILE:
            case ui::SelectFileDialog::SELECT_OPEN_MULTI_FILE:
            case ui::SelectFileDialog::SELECT_FOLDER:
            {
                return DoModalOpen(type);
            }

            case ui::SelectFileDialog::SELECT_SAVEAS_FILE:
            {
                return DoModalSave(type);
            }

            default:
                DCHECK(false);
                return false;
            }
        }

        std::wstring GetSelectedFileName() override {
            if (!selected_files_.empty()) {
                return *selected_files_.begin();
            }
            return {};
        }

        std::vector<std::wstring> GetSelectedFileNames() override {
            return selected_files_;
        }

        std::wstring GetErrorMessage() override {
            return error_message_;
        }

    private:
        bool DoModalOpen(ui::SelectFileDialog::Type type) {
            base::win::ScopedComPtr<IFileOpenDialog> dialog;
            HRESULT hr = ::CoCreateInstance(
                CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(dialog.Receive()));
            if (FAILED(hr)) {
                LOG(WARNING) << "Failed to create the instance of IFileOpenDialog: " << hr;
                return false;
            }

            FILEOPENDIALOGOPTIONS options = FOS_FORCEFILESYSTEM;
            if (type == ui::SelectFileDialog::SELECT_FOLDER) {
                options |= FOS_PICKFOLDERS;
            } else if (type == ui::SelectFileDialog::SELECT_OPEN_MULTI_FILE) {
                options |= FOS_ALLOWMULTISELECT;
            }

            dialog->SetOptions(options);

            if (type != ui::SelectFileDialog::SELECT_FOLDER) {
                std::vector<COMDLG_FILTERSPEC> exts_raw;
                for (const auto& ext : exts_) {
                    exts_raw.push_back({ ext.first.c_str(), ext.second.c_str() });
                }
                dialog->SetFileTypes(UINT(exts_raw.size()), exts_raw.data());
            }

            if (!default_path_.empty()) {
                base::win::ScopedComPtr<IShellItem> item;
                hr = ::SHCreateItemFromParsingName(
                    default_path_.c_str(), nullptr, IID_PPV_ARGS(item.Receive()));
                if (SUCCEEDED(hr)) {
                    dialog->SetFolder(item.get());
                }
            }

            HWND handle = parent_ ? parent_->GetNativeView() : nullptr;
            if (SUCCEEDED(dialog->Show(handle))) {
                base::win::ScopedComPtr<IShellItemArray> items;
                if (SUCCEEDED(dialog->GetResults(items.Receive()))) {
                    DWORD count;
                    if (SUCCEEDED(items->GetCount(&count))) {
                        bool have_succeeded = false;
                        for (DWORD i = 0; i < count; ++i) {
                            base::win::ScopedComPtr<IShellItem> item;
                            if (SUCCEEDED(items->GetItemAt(i, item.Receive()))) {
                                LPWSTR path;
                                if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
                                    std::wstring path_str(path);
                                    selected_files_.push_back(path_str);
                                    ::CoTaskMemFree(path);
                                    have_succeeded = true;
                                }
                            }
                        }
                        return have_succeeded;
                    }
                }
            }

            return false;
        }

        bool DoModalSave(ui::SelectFileDialog::Type type) {
            base::win::ScopedComPtr<IFileSaveDialog> dialog;
            HRESULT hr = ::CoCreateInstance(
                CLSID_FileSaveDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(dialog.Receive()));
            if (FAILED(hr)) {
                LOG(WARNING) << "Failed to create the instance of IFileSaveDialog: " << hr;
                return false;
            }

            dialog->SetOptions(FOS_FORCEFILESYSTEM);
            {
                std::vector<COMDLG_FILTERSPEC> exts_raw;
                for (const auto& ext : exts_) {
                    exts_raw.push_back({ ext.first.c_str(), ext.second.c_str() });
                }
                dialog->SetFileTypes(UINT(exts_raw.size()), exts_raw.data());
            }

            if (!default_path_.empty()) {
                base::win::ScopedComPtr<IShellItem> item;
                hr = ::SHCreateItemFromParsingName(
                    default_path_.c_str(), nullptr, IID_PPV_ARGS(item.Receive()));
                if (SUCCEEDED(hr)) {
                    dialog->SetFolder(item.get());
                }
            }

            HWND handle = parent_ ? parent_->GetNativeView() : nullptr;
            if (SUCCEEDED(dialog->Show(handle))) {
                base::win::ScopedComPtr<IShellItem> item;
                if (SUCCEEDED(dialog->GetResult(item.Receive()))) {
                    LPWSTR path;
                    if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
                        std::wstring path_str(path);
                        selected_files_.push_back(path_str);
                        ::CoTaskMemFree(path);
                        return true;
                    }
                }
            }

            return false;
        }

        views::Widget* parent_;
        std::vector<std::pair<std::wstring, std::wstring>> exts_;

        std::wstring default_path_;
        std::vector<std::wstring> selected_files_;
        std::wstring error_message_;
    };


    ISelectFileDialog* CreateSelectFileDialog(views::Widget* parent)
    {
        return new SelectFileDialogEx(parent);
    }

}
