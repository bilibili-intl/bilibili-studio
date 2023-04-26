#include "bililive/bililive/ui/views/livehime/third_party_streaming/tps_obs_guide_view.h"

#include "base/file_util.h"
#include "base/path_service.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/thread_restrictions.h"
#include "base/win/registry.h"

#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/grid_layout.h"

#include "bililive/bililive/livehime/obs/obs_inject_util.h"
#include "bililive/bililive/ui/bililive_commands.h"
#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/bililive/ui/dialog/shell_dialogs.h"
#include "bililive/bililive/ui/views/controls/bililive_floating_scroll_view.h"
#include "bililive/bililive/ui/views/hotkey_notify/bililive_hotkey_notify_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_image_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_native_edit.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/utils/bililive_canvas_drawer.h"
#include "bililive/common/bililive_context.h"
#include "bililive/public/common/bililive_constants.h"
#include "bililive/public/common/bililive_paths.h"

namespace
{
    enum
    {
        Button_Folder = 1,
        Button_Setup,
        Button_Remove,
    };

    ThirdPartyStreamingOBSGuideView* g_single_instance = nullptr;

}


void ThirdPartyStreamingOBSGuideView::ShowForm(gfx::NativeView parent)
{
    if (!g_single_instance)
    {
        views::Widget* widget = new views::Widget();

        views::Widget::InitParams params;
        params.parent = parent;

        g_single_instance = new ThirdPartyStreamingOBSGuideView();
        DoModalWidget(g_single_instance, widget, params);
    }
}

ThirdPartyStreamingOBSGuideView::ThirdPartyStreamingOBSGuideView()
    : BililiveWidgetDelegate(gfx::ImageSkia(), L"第三方推流模式OBS用户使用引导")
    , weakptr_factory_(this)
{
}

ThirdPartyStreamingOBSGuideView::~ThirdPartyStreamingOBSGuideView()
{
    g_single_instance = nullptr;
}

void ThirdPartyStreamingOBSGuideView::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails &details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            InitViews();
            InitData();
        }
    }
}

gfx::Size ThirdPartyStreamingOBSGuideView::GetPreferredSize()
{
    return content_view_->GetPreferredSize();
}

void ThirdPartyStreamingOBSGuideView::InitViews()
{
    content_view_ = new BililiveViewWithFloatingScrollbar();
    SetLayoutManager(new views::FillLayout());
    AddChildView(content_view_->Container());

    views::GridLayout *layout = new views::GridLayout(content_view_);
    content_view_->SetLayoutManager(layout);

    views::ColumnSet *column_set = layout->AddColumnSet(0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

    column_set = layout->AddColumnSet(1);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForCtrlTips);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

    column_set = layout->AddColumnSet(2);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    column_set->AddPaddingColumn(1.0f, 0);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForActionButton);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(1.0f, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

    BililiveLabel* label = new LivehimeTitleLabel(L"情形一：直播姬OBS插件已成功安装到OBS目录中");
    label->SetHorizontalAlignment(gfx::ALIGN_LEFT);
    label->SetFont(ftPrimaryBold);

    BililiveLabel* tip_label = new LivehimeContentLabel(L"即直播姬OBS插件已由直播姬安装程序自动安装到OBS目录中，\n"
        "可从OBS主界面顶部的菜单栏中的“工具 -> 哔哩哔哩直播”直接启动/激活直播姬并开始直播。");
    tip_label->SetHorizontalAlignment(gfx::ALIGN_LEFT);
    tip_label->SetMultiLine(true);
    tip_label->SetAllowCharacterBreak(true);

    LivehimeImageView* guide_img = new LivehimeImageView();
    guide_img->SetImage(*GetImageSkiaNamed(IDR_LIVEHIME_TPS_OBS_GUIDE_ENTRY));

    layout->AddPaddingRow(0, kPaddingRowHeightEnds);
    layout->StartRow(0, 0);
    layout->AddView(label);

    layout->AddPaddingRow(0, kPaddingRowHeightForCtrlTips);
    layout->StartRow(0, 0);
    layout->AddView(tip_label);

    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, 0);
    layout->AddView(guide_img);

    label = new LivehimeTitleLabel(L"情形二：直播姬OBS插件尚未安装到OBS目录中");
    label->SetHorizontalAlignment(gfx::ALIGN_LEFT);
    label->SetFont(ftPrimaryBold);

    tip_label = new LivehimeContentLabel(L"即直播姬安装程序未能在本机检测到OBS安装目录，从而无法自动安装插件，\n"
        "可通过手动指定OBS所的目录将直播姬插件安装到OBS中，然后重启OBS即可使插件生效。");
    tip_label->SetHorizontalAlignment(gfx::ALIGN_LEFT);
    tip_label->SetMultiLine(true);
    tip_label->SetAllowCharacterBreak(true);

    guide_img = new LivehimeImageView();
    guide_img->SetImage(*GetImageSkiaNamed(IDR_LIVEHIME_TPS_OBS_GUIDE_DIR));

    layout->AddPaddingRow(0, kPaddingRowHeightEnds);
    layout->StartRow(0, 0);
    layout->AddView(label);

    layout->AddPaddingRow(0, kPaddingRowHeightForCtrlTips);
    layout->StartRow(0, 0);
    layout->AddView(tip_label);

    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, 0);
    layout->AddView(guide_img);

    label = new LivehimeTitleLabel(L"OBS安装目录：");
    label->SetHorizontalAlignment(gfx::ALIGN_LEFT);

    folder_edit_ = new LivehimeNativeEditView();

    folder_button_ = new LivehimeFunctionLabelButton(this, L"选择目录");
    folder_button_->set_id(Button_Folder);

    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, 1);
    layout->AddView(label);
    layout->AddView(folder_edit_);
    layout->AddView(folder_button_);

    obs_folder_tip_ = new LivehimeTipLabel();
    obs_folder_tip_->SetMultiLine(true);

    layout->AddPaddingRow(0, kPaddingRowHeightForCtrlTips);
    layout->StartRow(0, 0);
    layout->AddView(obs_folder_tip_);

    LivehimeFunctionLabelButton* setup_button = new LivehimeFunctionLabelButton(this, L"安装插件");
    setup_button->SetStyle(LivehimeButtonStyle_ActionButtonPositive);
    setup_button->set_id(Button_Setup);

    LivehimeFunctionLabelButton* remove_button = new LivehimeFunctionLabelButton(this, L"移除插件");
    remove_button->SetStyle(LivehimeButtonStyle_ActionButtonNegative);
    remove_button->set_id(Button_Remove);

    layout->AddPaddingRow(0, kPaddingRowHeightForCtrlTips);
    layout->StartRow(0, 2);
    layout->AddView(setup_button);
    layout->AddView(remove_button);

    layout->AddPaddingRow(0, kPaddingRowHeightEnds);
}

void ThirdPartyStreamingOBSGuideView::InitData()
{
    // 32位和64位OBS的共同安装路径都写在'SOFTWARE\WOW6432Node\OBS Studio';
    const wchar_t kOBSRegInstallPath[] = L"SOFTWARE\\OBS Studio";
    base::win::RegKey obs_key;
    auto result = obs_key.Open(HKEY_LOCAL_MACHINE, kOBSRegInstallPath, KEY_READ | KEY_WOW64_32KEY);
    if (result != ERROR_SUCCESS)
    {
        result = obs_key.Open(HKEY_LOCAL_MACHINE, kOBSRegInstallPath, KEY_READ | KEY_WOW64_64KEY);
        if (result != ERROR_SUCCESS)
        {
            return;
        }
    }

    std::wstring path;
    result = obs_key.ReadValue(nullptr, &path);
    if (result == ERROR_SUCCESS && !path.empty())
    {
        folder_edit_->SetText(path);

        obs_folder_tip_->SetText(L"已检测到本机OBS安装目录：“" + path + L"”。");
    }
}

void ThirdPartyStreamingOBSGuideView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    switch (sender->id())
    {
    case Button_Folder:
    {
        scoped_ptr<bililive::ISelectFileDialog> dlg(bililive::CreateSelectFileDialog(sender->GetWidget()));
        if (dlg)
        {
            dlg->SetDefaultPath(folder_edit_->text());
            bool selected = dlg->DoModel(ui::SelectFileDialog::Type::SELECT_FOLDER);
            if (selected)
            {
                std::wstring wfolder = dlg->GetSelectedFileName();
                folder_edit_->SetText(wfolder);
            }
        }
    }
    break;
    case Button_Setup:
    {
        base::FilePath path(folder_edit_->text());
        if (!path.empty())
        {
            livehime::CreateOrRemoveObsInjectFlag(true);

            base::FilePath bit32 = path.Append(L"obs-plugins\\32bit");
            base::FilePath bit64 = path.Append(L"obs-plugins\\64bit");

            base::FilePath cur_dir = BililiveContext::Current()->GetMainDirectory();
            base::FilePath plugin32 = cur_dir.Append(L"plugins").Append(bililive::kBililiveOBSPluginDll);
            base::FilePath plugin64 = cur_dir.Append(L"x64\\plugins").Append(bililive::kBililiveOBSPluginDll);

            base::string16 success_str;
            base::string16 failed_str;
            auto copy_func = [&success_str, &failed_str](const base::FilePath& src_file, const base::FilePath& dst_dir)
            {
                base::ThreadRestrictions::ScopedAllowIO allow;
                if (base::DirectoryExists(dst_dir))
                {
                    auto dst_bit = dst_dir.Append(bililive::kBililiveOBSPluginDll);
                    if (base::CopyFile(src_file, dst_bit))
                    {
                        success_str += L"插件已成功安装到：“" + dst_dir.value() + L"”。\n";
                    }
                    else
                    {
                        DWORD dwErr = ::GetLastError();

                        LPWSTR error_string = NULL;
                        int size = ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                            FORMAT_MESSAGE_FROM_SYSTEM,
                            0,  // Use the internal message table.
                            dwErr,
                            0,  // Use default language.
                            (LPWSTR)&error_string,
                            0,  // Buffer size.
                            0);  // Arguments (unused).

                        failed_str += base::StringPrintf(L"插件无法安装到：“%ls”，\ncode=%d, msg=%ls\n",
                            dst_dir.value().c_str(), dwErr, error_string);

                        ::LocalFree(error_string);
                    }
                }
            };
        
            copy_func(plugin32, bit32);
            copy_func(plugin64, bit64);
            base::string16 msg = success_str + ((!success_str.empty() && !failed_str.empty()) ? L"\n" : L"") + failed_str;
            if (!msg.empty())
            {
                ReplaceChars(msg, L"\r", L"", &msg);
                msg.erase(msg.end() - 1);

                HotkeyNotifyView::ShowForm(msg,
                    GetWidget()->GetWindowBoundsInScreen(),
                    nullptr, &ftPrimary);
            }
        }
        else
        {
            livehime::ShowMessageBox(GetWidget()->GetNativeView(),
                GetLocalizedString(IDS_TIP_DLG_TIP),
                L"请选择有效的OBS安装目录",
                GetLocalizedString(IDS_LIVE_ROOM_MSGBOX_IKNOW));
        }
    }
        break;
    case Button_Remove:
        {
            // 删除配置标识文件
            livehime::CreateOrRemoveObsInjectFlag(false);
            // 再删插件
            base::FilePath dst_dir(folder_edit_->text());
            if (!dst_dir.empty())
            {
                base::FilePath bit32 = dst_dir.Append(L"obs-plugins\\32bit");
                base::FilePath bit64 = dst_dir.Append(L"obs-plugins\\64bit");

                base::string16 success_str;
                base::string16 failed_str;
                auto del_func = [&success_str, &failed_str](const base::FilePath& dst_dir)
                {
                    auto dst_file = dst_dir.Append(bililive::kBililiveOBSPluginDll);
                    base::ThreadRestrictions::ScopedAllowIO allow;
                    if (base::PathExists(dst_file))
                    {
                        if (base::DeleteFile(dst_file, false))
                        {
                            success_str += L"插件已成功从“" + dst_dir.value() + L"”移除。\n";
                        }
                        else
                        {
                            DWORD dwErr = ::GetLastError();

                            LPWSTR error_string = NULL;
                            int size = ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                FORMAT_MESSAGE_FROM_SYSTEM,
                                0,  // Use the internal message table.
                                dwErr,
                                0,  // Use default language.
                                (LPWSTR)&error_string,
                                0,  // Buffer size.
                                0);  // Arguments (unused).

                            failed_str += base::StringPrintf(L"插件无法从“%ls”移除，请确认OBS已退出。\ncode=%d, msg=%ls\n",
                                dst_dir.value().c_str(), dwErr, error_string);

                            ::LocalFree(error_string);
                        }
                    }
                    else
                    {
                        success_str += L"插件已成功从“" + dst_dir.value() + L"”移除。\n";
                    }
                };

                del_func(bit32);
                del_func(bit64);
                base::string16 msg = success_str + ((!success_str.empty() && !failed_str.empty()) ? L"\n" : L"") + failed_str;
                if (!msg.empty())
                {
                    ReplaceChars(msg, L"\r", L"", &msg);
                    msg.erase(msg.end() - 1);

                    HotkeyNotifyView::ShowForm(msg,
                        GetWidget()->GetWindowBoundsInScreen(),
                        nullptr, &ftPrimary);
                }
            }
        }
        break;
    default:
        break;
    }
}
