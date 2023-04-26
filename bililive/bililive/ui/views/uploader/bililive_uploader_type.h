#ifndef BILILIVE_BILILIVE_UI_VIEWS_UPLOADER_BILILIVE_UPLOADER_TYPE_H
#define BILILIVE_BILILIVE_UI_VIEWS_UPLOADER_BILILIVE_UPLOADER_TYPE_H

#include "bililive/public/common/refcounted_dictionary.h"

#include "base/strings/string16.h"
#include "base/files/file_path.h"
#include "base/strings/stringprintf.h"

#include "SkColor.h"

#include "ui/gfx/font.h"
#include "ui/gfx/platform_font.h"
#include "ui/gfx/image/image.h"
#include "ui/views/widget/widget.h"

namespace bililive_uploader
{
    enum AlbumEditType
    {
        AET_ALL_ENABLE,
        AET_DIS_TYPE_AND_PARTI,
        AET_ALL_DISABLE,
    };

    enum UploadedDataTableReuseId
    {
        UploadedEmptyCellReuseId,
        UploadedPagingSectionReuseId,
        UploadedAlbumCellReuseId,
    };

    enum FileOperate
    {
        FO_PREVIEW,
    };

    const SkColor clrPink = SkColorSetRGB(255, 112, 158);           // pink主色
    const SkColor clrLightPink = SkColorSetRGB(255, 151, 190);      // 按钮hover色
    const SkColor clrDarkPink = SkColorSetRGB(242, 82, 138);        // 按钮点击色
    const SkColor clrBlackText = SkColorSetRGB(48, 48, 48);         // 黑字色
    const SkColor clrGrayText = SkColorSetRGB(127, 127, 127);       // 灰字色
    const SkColor clrGrayTipText = SkColorSetRGB(180, 180, 180);    // tooltip灰字色、分隔线
    const SkColor clrGrayLine = SkColorSetRGB(228, 228, 228);       // 灰分隔线
    const SkColor clrGrayBackground = SkColorSetRGB(244, 244, 244); // 灰背景色
    
    const SkColor clrGray = clrGrayTipText;
    const SkColor clrWhite = SkColorSetRGB(255, 233, 240);
    const SkColor clrBlue = SkColorSetRGB(0, 161, 214);
    const SkColor clrRed = SkColorSetRGB(229, 12, 12);
    const SkColor clrLightWhite = SK_ColorWHITE;
    const SkColor clrGrayBorder = clrGrayLine;
    const SkColor clrPinkBorder = clrPink;
    const SkColor clrLightPinkBackground = clrLightPink;
    const SkColor clrWhiteBackground = SK_ColorWHITE; // 灰背景色


    const SkColor clrFrameBorder = SkColorSetRGB(255, 220, 232);// SkColorSetRGB(192, 193, 197);
    const SkColor clrHoverPink = SkColorSetRGB(255, 133, 173);
    const SkColor clrButtonGrayBk = SkColorSetRGB(244, 245, 247);
    const SkColor clrButtonHoverGrayBk = SkColorSetRGB(229, 231, 235);
    const SkColor clrButtonGrayBorder = SkColorSetRGB(204, 208, 215);
    const SkColor clrButtonGrayBorderDis = SkColorSetRGB(204, 204, 204)/*SkColorSetRGB(235, 236, 239)*/;
    const SkColor clrButtonGrayBkDis = SkColorSetRGB(251, 251, 252);
    const SkColor clrButtonGrayTextDis = SkColorSetRGB(204, 204, 204)/*SkColorSetRGB(228, 229, 231)*/;
    const SkColor clrButtonGrayTextNr = SkColorSetRGB(204, 208, 215);
    const SkColor clrButtonGrayTextHv = SkColorSetRGB(144, 150, 162);
    const SkColor clrTabActivePink = SkColorSetRGB(255, 238, 243);
    const SkColor clrTabInactiveGray = SkColorSetRGB(244, 245, 247);
    const SkColor clrCtrlPinkBorder = SkColorSetRGB(255, 202, 220);
    const SkColor clrCtrlLightPinkBackground = SkColorSetRGB(255, 251, 252);

    //const gfx::Font ftLargeBold1((new gfx::Font(gfx::PlatformFont::CreateDefault()))->DeriveFont(1, gfx::Font::BOLD));

    static const wchar_t *UploadFileExts[] =
    {
        L"mp4", L"flv", L"wmv", L"avi", L"dat", L"asf", L"rm", L"rmvb", L"mov", L"3gp", L"mpg", L"mpeg", L"div", L"dv", L"divx", L"mkv", 0
    };

    static const wchar_t *CoverFileExts[] =
    {
        L"png", L"jpg", L"gif", 0
    };

    static const wchar_t *PictureFileExts[] =
    {
        L"png", L"jpg"/*, L"gif"*/, 0
    };

    static const wchar_t *AudioFileExts[] =
    {
        L"mp3", L"wma", L"m4a", L"aac", L"adts", L"ogg", 0
    };

    static const wchar_t *SubtitleFileExts[] =
    {
        // *.idx;*.smi;*.srt;*.psb;*.ssa;*.ass;*.usf;*.xss;*.txt;*.rt;*.sup
        L"idx", L"smi", L"srt", L"psb", L"ssa", L"ass", L"usf", L"xss", L"rt", L"sup", 0
    };

    // 显示红色
    static std::set<int> RedCode = { -2, -14, -3, -4, -5, -12, -16 };
    // 显示蓝色
    static std::set<int> BlueCode = { 0, 1, -1, -6, -7, -8, -9, -10, -13, -15, -20, -30, -11, -40 };

    enum ArchiveState {
        // 本地稿件（新建稿件或正在添加分P的稿件）
        ArchiveLocal = 9999999,

        // 开放浏览
        ArchiveOpen = 0,

        // 橙色通过
        ArchiveOrangePast = 1,

        // 待审
        ArchivePendingReview = -1,

        // 已退回
        ArchiveReject = -2,

        // 网警锁定
        ArchiveNPLock = -3,

        // 已锁定
        ArchiveLocked = -4,

        // 管理锁定
        ArchiveAdminLock = -5,

        // 修复待审
        ArchivePendingRepair = -6,

        // 暂缓审核
        ArchiveDelayReview = -7,

        // 补档审核
        ArchiveRepairReview = -8,

        // 等待转码
        ArchivePendingEncoding = -9,

        // 延迟审核
        ArchiveDeferReview = -10,

        // 视频源待修
        ArchiveSourceRepair = -11,

        // 上传失败
        ArchiveUploadFailed = -12,

        // 允许评论待审
        ArchiveCommentReivew = -13,

        // 回收站
        ArchiveTrash = -14,

        // 分发中
        ArchiveDistribute = -15,

        // 转码失败
        ArchiveEncodingFailed = -16,

        // 用户软删除
        ArchiveSoftDelete = -100,

        // 创建但未提交
        ArchiveCreateNoSubmit = -20,

        // 完成提交（创建已提交）
        ArchiveSubmitted = -30,

        // 审核通过，等待定时发布...
        ArchiveWaitPublish = -40,

        //用于初始化
        ArchiveNone = 101
    };

    struct VersionInfo
    {
        VersionInfo()
            : coerciveness(0){}

        int coerciveness;
        std::string version;
        std::string infourl;
        std::string exeurl;
        std::string md5;
    };
    extern VersionInfo g_ugc_version_info;

    // 个人用户信息
    struct UgcUserInfo
    {
        UgcUserInfo()
            : init(false)
        {}

        // 本地使用
        bool init;
    };
    extern UgcUserInfo g_ugc_user_info;

    // 右下角非焦点弹窗链接信息
    struct NotifyDetails
    {
        explicit NotifyDetails(const base::string16 &reason)
            : notice(reason)
            , operation(0)
            , params(NULL)
        {
        }

        base::string16 notice;
        int operation;
        void *params;
    };

    enum UploaderInfoType
    {
        FIT_FROM_SERVER = -1,
        FIT_FROM_COMPRESS,
        FIT_FROM_UPLOADING,
        FIT_FROM_UPLOADED,
    };

    // 提交稿件时服务端接口需要用的视频信息
    struct UploaderVideoInfo
    {
        base::string16 title;
        base::string16 desc;
        std::string filename;
    };

    // 压制参数
    struct CompressParam
    {
        CompressParam()
            : compress_id(-1)
            , changed(false)
            , speed(0)
            , target(0)
            , cfg_index(0)
        {}

        int compress_id;
        base::FilePath path;
        base::FilePath compress_path;

        bool changed;
        std::vector<std::string> cfgs;
        int cfg_index;
        int speed;
        int target;
    };

    // 压制任务
    struct CompressTaskInfo : public base::RefCountedThreadSafe<CompressTaskInfo>
    {
        explicit CompressTaskInfo(int64 id, int order, const base::FilePath& video_path)
            : aid(id)
            , index(order)
            , path(video_path)
            , compression_id(-1)
            , add_for_upload(true)
        {
        }

        explicit CompressTaskInfo(const base::FilePath& video_path)
            : aid(0)
            , index(0)
            , path(video_path)
            , compression_id(-1)
            , add_for_upload(false)
        {
        }

        int64 aid;
        int index;
        base::FilePath path;    // 上传的文件的绝对路径
        int compression_id;

        bool add_for_upload;

    private:
        friend class base::RefCountedThreadSafe<CompressTaskInfo>;
        ~CompressTaskInfo(){};
    };
    typedef scoped_refptr<CompressTaskInfo> RefCompressTaskInfo;

    struct UploadFileInfo;
    typedef scoped_refptr<bililive_uploader::UploadFileInfo> RefUploadFileInfo;
    // 稿件信息
    struct UploadAlbumInfo : public base::RefCountedThreadSafe<UploadAlbumInfo>
    {
        UploadAlbumInfo(bool fromsrv)
            : aid(0)
            , no_reprint(0)
            , state(-1)
            , mission_id(0)
            , order_id(0)
            , keywords_change(0)
            , type_id(0)
            , ptime(0)
            , dtime(0)
            , view(0)
            , danmaku(0)
            , reply(0)
            , coin(0)
            , desc_format_id(0)
            , user_elec_stat_state(1)
            , auto_submit(false)
        {
        }

        // 本地使用的字段
        gfx::Image cover_image;
        bool auto_submit;   // 自动提交

        // 服务器接口返回的字段
        int64 aid;
        base::string16 title;
        std::string cover;
        base::string16 pubtime;// 审核通过的开放时间
        double ptime;// 审核通过的开放时间
        double dtime;// 延时发布的时间
        int state;
        base::string16 state_desc;
        base::string16 desc;
        base::string16 reject_reason;
        base::string16 tag;//,分割
        std::vector<base::string16> tags;
        int no_reprint;    // 不允许转载
        int mission_id;// 自制时参加活动的活动ID
        base::string16 mission_name;// 活动ID对应的名称
        int order_id;// 商单ID
        base::string16 order_name;// 商单ID对应的名称
        base::string16 reprint_description;// 转载描述，转载时转载描述与稿件自身描述合为一个字段，以空格分离
        int keywords_change;
        int type_id;// 分区
        base::string16 type_name;
        int view;
        int danmaku;
        int reply;
        int coin;
        std::vector<RefUploadFileInfo> videos;// 从服务器获取的历史稿件的全部分P列表
        int desc_format_id;

        int user_elec_stat_state;   // 充电计划，新建投稿时默认开启

    private:
        friend class base::RefCountedThreadSafe<UploadAlbumInfo>;
        ~UploadAlbumInfo(){};
    };
    typedef scoped_refptr<bililive_uploader::UploadAlbumInfo> RefUploadAlbumInfo;

    // 稿件分P信息
    struct UploadFileInfo : public base::RefCountedThreadSafe<UploadFileInfo>
    {
        UploadFileInfo()
            : fit(FIT_FROM_UPLOADING)
            , index_order(0)
            , vid(-1)
            , status(0)
            , fail_code(0)
            , upload_immediately(false)
            , compression_id(-1)
            , is_compressed(false)
        {
            static volatile long incid = 0;
            vid = ::InterlockedDecrement(&incid);
            compression_id = vid;
        }

        UploaderInfoType fit;
        int vid;
        std::string filename;
        base::string16 title;
        base::string16 desc;
        int index_order;
        int status;
        base::string16 status_desc;
        int fail_code;
        base::string16 fail_desc;
        base::string16 ctime;
        base::string16 reject_reason;

        // 本地使用的字段
        RefUploadAlbumInfo album_info;
        base::string16 path;    // 上传的文件的绝对路径
        bool upload_immediately;    // 是否即刻开始

        // 压制模块
        bool is_compressed; // 文件是否是被压制过的
        int compression_id;
        base::string16 compression_path;    // 压制的文件的绝对路径

    private:
        friend class base::RefCountedThreadSafe<UploadFileInfo>;
        ~UploadFileInfo(){};
    };

    // 本地使用的标签信息
    struct DigestInfo
    {
        DigestInfo(const base::string16 &text, bool sel = true)
            : tag(text)
            , is_selected(sel){}

        base::string16 tag;
        bool is_selected;
    };

    // 开启视频任务的状态
    enum VideoTaskState
    {
        VTS_INVALID = -1,
        VTS_ISRUNNING,
        VTS_WAITTING,
        VTS_STARTING,
    };

    // 判断文件是否为"mp4", "flv"后缀
    bool GetFileTypeByExts(const base::FilePath &filePath);
    bool MatchesExtension(const base::FilePath &filePath, const wchar_t* exts[]);
    base::string16 SelectFile(views::Widget* parent, const base::string16 &title, const wchar_t* exts[], base::string16 *err_msg);
    base::string16 SelectVideoFile(views::Widget* parent, base::string16 *err_msg);
    base::string16 SelectCoverFile(views::Widget* parent, base::string16 *err_msg);
    // 将服务端接口返回的时间戳转成时间字符串xxxx-xx-xx xx:xx:xx
    base::string16 TimeStampToStandardString(double db);
    // 将SYSTEMTIME转时间戳time_t
    time_t SystemTimeToTimeStamp(LPSYSTEMTIME pSystime);
    SYSTEMTIME TimeStampToSystemTime(time_t tt);
    // 将byte速率转为合适的速率
    base::string16 ByteSpeedToSuitablySpeed(int64_t speed_int_byte);
    // 判断服务器返回的封面url是不是针对无封面稿件的透明图
    bool CheckValidCover(const std::string &cover_url);
    // 判断稿件状态以获取其编辑状态
    AlbumEditType GetAlbumEditType(int state);
    // 根据状态判断是该显示稿件被打回的信息
    bool ShouldShowRejectInfo(int album_state);
    // 稿件是否被锁定
    bool IsAlbumLocked(int album_state);
    // 标签是否可编辑
    bool IsTagsEditable(int album_state);
    // 根据稿件状态获取其应该对应的文案显示色
    SkColor GetStateColor(int state);
    // 用户是否有效
    bool IsUserValid();
    // 上传权限检测
    bool CheckSubmitAccessAndNotify(HWND hwnd);
    // 根据AVSErrorCode得到具体错误描述字符串
    base::string16 GetAVSErrorCodeMessage(int code);
}

#endif