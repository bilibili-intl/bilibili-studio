
#include "bililive_uploader_type.h"
#include "bililive_uploader_real_name_view.h"

#include "ui/views/controls/messagebox/message_box.h"

#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/common/bililive_paths.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/bililive/ui/dialog/shell_dialogs.h"
#include "bililive/bililive/ui/bililive_commands.h"
#include "bililive/bililive/ui/bililive_uploader.h"

#include "base/path_service.h"
#include "base/strings/utf_string_conversions.h"
#include "base/prefs/pref_service.h"

#include "ui/base/resource/resource_bundle.h"
#include "grit/theme_resources.h"
#include "grit/generated_resources.h"


namespace bililive_uploader
{
    VersionInfo g_ugc_version_info;
    UgcUserInfo g_ugc_user_info;

    static std::set<int> AlbumStateSet_AllEnable = { -2, -14, -100 };
    static std::set<int> AlbumStateSet_AlbumLocked = { -3, -4, -5 };   // 稿件被锁定，什么都操作不了
    static std::set<int> AlbumStateSet_TypeAndPartitionDisable = { 1, 0, -1, -6, -7, -8, -9, -10, -13, -15, -20, -30, -11, -12, -16, -40 };// 稿件类型和分区不可编辑

    // 能够通过实名认证的检验码
    static std::set<int> IdentifyCode = { 0 };

    bool GetFileTypeByExts(const base::FilePath &filePath)
    {
        for (int i = 0; i < arraysize(UploadFileExts); i++)
        {
            if (filePath.MatchesExtension(base::StringPrintf(L".%ls", UploadFileExts[i])))
            {
                return true;
            }
        }
        return false;
    }

    bool MatchesExtension(const base::FilePath &filePath, const wchar_t* exts[])
    {
        const wchar_t** ext = exts;
        for (const wchar_t** ext = exts; *ext; ++ext)
        {
            if (filePath.MatchesExtension(base::StringPrintf(L".%ls", *ext)))
            {
                return true;
            }
        }
        /*while (*ext)
        {

        ++ext;
        }*/
        return false;
    }

    base::string16 SelectFile(views::Widget* parent, const base::string16 &title, const wchar_t* exts[], base::string16 *err_msg)
    {
        PrefService *pref = GetBililiveProcess()->profile()->GetPrefs();

        base::string16 path;
        bililive::ISelectFileDialog* dlg = bililive::CreateSelectFileDialog(parent);
        std::wstring filterStr = bililive::AppendExtListToDesc(title, exts);
        dlg->AddFilter(filterStr, exts);
        bool selected = dlg->DoModel();
        if (selected)
        {
            path = dlg->GetSelectedFileName();
        }
        else
        {
            if (err_msg)
            {
                *err_msg = dlg->GetErrorMessage();
            }
        }
        delete dlg;

        return path;
    }

    base::string16 SelectVideoFile(views::Widget* parent/* = 0*/, base::string16 *err_msg/* = NULL*/)
    {
        PrefService *pref = GetBililiveProcess()->profile()->GetPrefs();

        base::string16 path;
        bililive::ISelectFileDialog* dlg = bililive::CreateSelectFileDialog(parent);
        std::wstring filterStr = bililive::AppendExtListToDesc(
            ResourceBundle::GetSharedInstance().GetLocalizedString(IDS_UGC_MSGBOX_VIDEO_FILTER),
            bililive_uploader::UploadFileExts);
        dlg->AddFilter(filterStr, bililive_uploader::UploadFileExts);
        base::FilePath last_video_dir;    // use current directory as a fallback.
        {
            last_video_dir = pref->GetFilePath(prefs::kUploaderLastChooseFolder);
        }
        dlg->SetDefaultPath(last_video_dir.value());
        bool selected = dlg->DoModel();
        if (selected)
        {
            path = dlg->GetSelectedFileName();
            if (!path.empty())
            {
                base::FilePath folder = base::FilePath(path).DirName();
                pref->SetFilePath(prefs::kUploaderLastChooseFolder, folder);
            }
        }
        else
        {
            if (err_msg)
            {
                *err_msg = dlg->GetErrorMessage();
            }
        }
        delete dlg;

        return path;
    }

    base::string16 SelectCoverFile(views::Widget* parent/* = 0*/, base::string16 *err_msg/* = NULL*/)
    {
        PrefService *pref = GetBililiveProcess()->profile()->GetPrefs();

        base::string16 path;
        bililive::ISelectFileDialog* dlg = bililive::CreateSelectFileDialog(parent);
        std::wstring filterStr = bililive::AppendExtListToDesc(
            ResourceBundle::GetSharedInstance().GetLocalizedString(IDS_UGC_MSGBOX_COVER_FILTER),
            bililive_uploader::CoverFileExts);
        dlg->AddFilter(filterStr, bililive_uploader::CoverFileExts);
        base::FilePath last_cover_dir;
        {
            last_cover_dir = pref->GetFilePath(prefs::kUploaderLastCoverFolder);
        }
        dlg->SetDefaultPath(last_cover_dir.value());
        bool selected = dlg->DoModel();
        if (selected)
        {
            path = dlg->GetSelectedFileName();
            if (!path.empty())
            {
                base::FilePath folder = base::FilePath(path).DirName();
                pref->SetFilePath(prefs::kUploaderLastCoverFolder, folder);
            }
        }
        else
        {
            if (err_msg)
            {
                *err_msg = dlg->GetErrorMessage();
            }
        }
        delete dlg;

        return path;
    }

    time_t SystemTimeToTimeStamp(LPSYSTEMTIME pSystime)
    {
        tm temptm = 
        {
            pSystime->wSecond,
            pSystime->wMinute,
            pSystime->wHour,
            pSystime->wDay,
            pSystime->wMonth - 1,
            pSystime->wYear - 1900,
            pSystime->wDayOfWeek,
            0,
            0
        };
        // 将tm按本地时区转换为UTC时间戳
        time_t mt = mktime(&temptm);

        tzset();
        mt += _timezone + 28800;

        return mt;
    }

    SYSTEMTIME TimeStampToSystemTime(time_t tt)
    {
        //tm ltm = *localtime(&tt);
        // 将UTC时间加8小时转为北京时间
        tt += +28800;
        // time_t转tm
        tm utc_8 = *gmtime(&tt);
        SYSTEMTIME st = 
        {
            utc_8.tm_year + 1900,
            utc_8.tm_mon + 1,
            utc_8.tm_wday,
            utc_8.tm_mday,
            utc_8.tm_hour,
            utc_8.tm_min,
            utc_8.tm_sec,
            0
        };

        return st;
    }

    base::string16 TimeStampToStandardString(double db)
    {
        SYSTEMTIME st = TimeStampToSystemTime(db);
        return base::StringPrintf(L"%d-%02d-%02d %02d:%02d:%02d", 
            st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
        /*time_t tick = (time_t)db;
        struct tm *ti = NULL;
        char s[128] = { 0 };

        ti = localtime(&tick);
        strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", ti);

        return base::ASCIIToWide(s);*/
    }

    base::string16 ByteSpeedToSuitablySpeed(int64_t speed_int_byte)
    {
        static const int64_t kb = 1024;
        static const int64_t mb = 1024 * kb;
        static const int64_t gb = 1024 * mb;

        if (speed_int_byte > gb)
        {
            return base::StringPrintf(L"%0.1f GB/S", speed_int_byte * 1.0f / gb);
        }
        else if (speed_int_byte > mb)
        {
            return base::StringPrintf(L"%0.1f MB/S", speed_int_byte * 1.0f / mb);
        }
        else if (speed_int_byte > kb)
        {
            return base::StringPrintf(L"%0.1f KB/S", speed_int_byte * 1.0f / kb);
        }
        else
        {
            return L"小于1KB/S";
        }
        return L"";
    }

    bool CheckValidCover(const std::string &cover_url)
    {
        if (cover_url.empty())
        {
            return false;
        }
        bool valid = true;
        // 没有封面的稿件服务端会返回一个透明的图http://static.hdslb.com/images/transparent.gif，我们应该展现的是默认的图
        if (cover_url.substr(cover_url.rfind('/') + 1).compare("transparent.gif") == 0)
        {
            valid = false;
        }
        return valid;
    }

    AlbumEditType GetAlbumEditType(int state)
    {
        AlbumEditType aet = AET_ALL_ENABLE;
        if (AlbumStateSet_AllEnable.count(state))
        {
            aet = AET_ALL_ENABLE;
        }
        else if (AlbumStateSet_TypeAndPartitionDisable.count(state))
        {
            aet = AET_DIS_TYPE_AND_PARTI;
        }
        else if (AlbumStateSet_AlbumLocked.count(state))
        {
            aet = AET_ALL_DISABLE;
        }
        return aet;
    }

    bool IsAlbumLocked(int album_state)
    {
        return (AlbumStateSet_AlbumLocked.count(album_state) != 0);
    }

    bool IsTagsEditable(int album_state)
    {
        if (IsAlbumLocked(album_state))
        {
            return false;
        }

        static std::set<int> AlbumStateSet_TagsEditable = { -2, -14 };
        return (AlbumStateSet_TagsEditable.count(album_state) != 0);
    }

    bool ShouldShowRejectInfo(int album_state)
    {
        static std::set<int> AlbumStateSet_ShowRejectInfo = { -2, -3, -4, -5, -12, -14, -16 };
        return (AlbumStateSet_ShowRejectInfo.count(album_state) != 0);
    }

    SkColor GetStateColor(int state)
    {
        if (BlueCode.count(state))
        {
            return clrBlue;
        }
        else
        {
            return clrRed;
        }
    }

    bool CheckSubmitAccessAndNotify(HWND hwnd)
    {
        return true;
    }

    base::string16 GetAVSErrorCodeMessage(int code)
    {
        std::wstring errmsg;
        switch ((AVSErrorCode)code)
        {
        case AVSErrorCode::kFileNameCharsetError:
            errmsg = L"字幕路径中包含无效的字符！";
            break;
        case AVSErrorCode::kAvisynthEnvironmentError:
            errmsg = L"字幕编辑环境错误！";
            break;
        case AVSErrorCode::kLoadVideoError:
            errmsg = L"视频的视频轨检测失败！";
            break;
        case AVSErrorCode::kLoadAudioError:
            errmsg = L"视频的音频轨检测失败！";
            break;
        case AVSErrorCode::kLoadSubtitleError:
            errmsg = L"无效的字幕文件！";
            break;
        case AVSErrorCode::kOtherAvisynthError:
        default:
            errmsg = L"未知的错误！";
            break;
        }
        return errmsg;
    }

}