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

    const SkColor clrPink = SkColorSetRGB(255, 112, 158);           // pink��ɫ
    const SkColor clrLightPink = SkColorSetRGB(255, 151, 190);      // ��ťhoverɫ
    const SkColor clrDarkPink = SkColorSetRGB(242, 82, 138);        // ��ť���ɫ
    const SkColor clrBlackText = SkColorSetRGB(48, 48, 48);         // ����ɫ
    const SkColor clrGrayText = SkColorSetRGB(127, 127, 127);       // ����ɫ
    const SkColor clrGrayTipText = SkColorSetRGB(180, 180, 180);    // tooltip����ɫ���ָ���
    const SkColor clrGrayLine = SkColorSetRGB(228, 228, 228);       // �ҷָ���
    const SkColor clrGrayBackground = SkColorSetRGB(244, 244, 244); // �ұ���ɫ
    
    const SkColor clrGray = clrGrayTipText;
    const SkColor clrWhite = SkColorSetRGB(255, 233, 240);
    const SkColor clrBlue = SkColorSetRGB(0, 161, 214);
    const SkColor clrRed = SkColorSetRGB(229, 12, 12);
    const SkColor clrLightWhite = SK_ColorWHITE;
    const SkColor clrGrayBorder = clrGrayLine;
    const SkColor clrPinkBorder = clrPink;
    const SkColor clrLightPinkBackground = clrLightPink;
    const SkColor clrWhiteBackground = SK_ColorWHITE; // �ұ���ɫ


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

    // ��ʾ��ɫ
    static std::set<int> RedCode = { -2, -14, -3, -4, -5, -12, -16 };
    // ��ʾ��ɫ
    static std::set<int> BlueCode = { 0, 1, -1, -6, -7, -8, -9, -10, -13, -15, -20, -30, -11, -40 };

    enum ArchiveState {
        // ���ظ�����½������������ӷ�P�ĸ����
        ArchiveLocal = 9999999,

        // �������
        ArchiveOpen = 0,

        // ��ɫͨ��
        ArchiveOrangePast = 1,

        // ����
        ArchivePendingReview = -1,

        // ���˻�
        ArchiveReject = -2,

        // ��������
        ArchiveNPLock = -3,

        // ������
        ArchiveLocked = -4,

        // ��������
        ArchiveAdminLock = -5,

        // �޸�����
        ArchivePendingRepair = -6,

        // �ݻ����
        ArchiveDelayReview = -7,

        // �������
        ArchiveRepairReview = -8,

        // �ȴ�ת��
        ArchivePendingEncoding = -9,

        // �ӳ����
        ArchiveDeferReview = -10,

        // ��ƵԴ����
        ArchiveSourceRepair = -11,

        // �ϴ�ʧ��
        ArchiveUploadFailed = -12,

        // �������۴���
        ArchiveCommentReivew = -13,

        // ����վ
        ArchiveTrash = -14,

        // �ַ���
        ArchiveDistribute = -15,

        // ת��ʧ��
        ArchiveEncodingFailed = -16,

        // �û���ɾ��
        ArchiveSoftDelete = -100,

        // ������δ�ύ
        ArchiveCreateNoSubmit = -20,

        // ����ύ���������ύ��
        ArchiveSubmitted = -30,

        // ���ͨ�����ȴ���ʱ����...
        ArchiveWaitPublish = -40,

        //���ڳ�ʼ��
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

    // �����û���Ϣ
    struct UgcUserInfo
    {
        UgcUserInfo()
            : init(false)
        {}

        // ����ʹ��
        bool init;
    };
    extern UgcUserInfo g_ugc_user_info;

    // ���½Ƿǽ��㵯��������Ϣ
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

    // �ύ���ʱ����˽ӿ���Ҫ�õ���Ƶ��Ϣ
    struct UploaderVideoInfo
    {
        base::string16 title;
        base::string16 desc;
        std::string filename;
    };

    // ѹ�Ʋ���
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

    // ѹ������
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
        base::FilePath path;    // �ϴ����ļ��ľ���·��
        int compression_id;

        bool add_for_upload;

    private:
        friend class base::RefCountedThreadSafe<CompressTaskInfo>;
        ~CompressTaskInfo(){};
    };
    typedef scoped_refptr<CompressTaskInfo> RefCompressTaskInfo;

    struct UploadFileInfo;
    typedef scoped_refptr<bililive_uploader::UploadFileInfo> RefUploadFileInfo;
    // �����Ϣ
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

        // ����ʹ�õ��ֶ�
        gfx::Image cover_image;
        bool auto_submit;   // �Զ��ύ

        // �������ӿڷ��ص��ֶ�
        int64 aid;
        base::string16 title;
        std::string cover;
        base::string16 pubtime;// ���ͨ���Ŀ���ʱ��
        double ptime;// ���ͨ���Ŀ���ʱ��
        double dtime;// ��ʱ������ʱ��
        int state;
        base::string16 state_desc;
        base::string16 desc;
        base::string16 reject_reason;
        base::string16 tag;//,�ָ�
        std::vector<base::string16> tags;
        int no_reprint;    // ������ת��
        int mission_id;// ����ʱ�μӻ�ĻID
        base::string16 mission_name;// �ID��Ӧ������
        int order_id;// �̵�ID
        base::string16 order_name;// �̵�ID��Ӧ������
        base::string16 reprint_description;// ת��������ת��ʱת��������������������Ϊһ���ֶΣ��Կո����
        int keywords_change;
        int type_id;// ����
        base::string16 type_name;
        int view;
        int danmaku;
        int reply;
        int coin;
        std::vector<RefUploadFileInfo> videos;// �ӷ�������ȡ����ʷ�����ȫ����P�б�
        int desc_format_id;

        int user_elec_stat_state;   // ���ƻ����½�Ͷ��ʱĬ�Ͽ���

    private:
        friend class base::RefCountedThreadSafe<UploadAlbumInfo>;
        ~UploadAlbumInfo(){};
    };
    typedef scoped_refptr<bililive_uploader::UploadAlbumInfo> RefUploadAlbumInfo;

    // �����P��Ϣ
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

        // ����ʹ�õ��ֶ�
        RefUploadAlbumInfo album_info;
        base::string16 path;    // �ϴ����ļ��ľ���·��
        bool upload_immediately;    // �Ƿ񼴿̿�ʼ

        // ѹ��ģ��
        bool is_compressed; // �ļ��Ƿ��Ǳ�ѹ�ƹ���
        int compression_id;
        base::string16 compression_path;    // ѹ�Ƶ��ļ��ľ���·��

    private:
        friend class base::RefCountedThreadSafe<UploadFileInfo>;
        ~UploadFileInfo(){};
    };

    // ����ʹ�õı�ǩ��Ϣ
    struct DigestInfo
    {
        DigestInfo(const base::string16 &text, bool sel = true)
            : tag(text)
            , is_selected(sel){}

        base::string16 tag;
        bool is_selected;
    };

    // ������Ƶ�����״̬
    enum VideoTaskState
    {
        VTS_INVALID = -1,
        VTS_ISRUNNING,
        VTS_WAITTING,
        VTS_STARTING,
    };

    // �ж��ļ��Ƿ�Ϊ"mp4", "flv"��׺
    bool GetFileTypeByExts(const base::FilePath &filePath);
    bool MatchesExtension(const base::FilePath &filePath, const wchar_t* exts[]);
    base::string16 SelectFile(views::Widget* parent, const base::string16 &title, const wchar_t* exts[], base::string16 *err_msg);
    base::string16 SelectVideoFile(views::Widget* parent, base::string16 *err_msg);
    base::string16 SelectCoverFile(views::Widget* parent, base::string16 *err_msg);
    // ������˽ӿڷ��ص�ʱ���ת��ʱ���ַ���xxxx-xx-xx xx:xx:xx
    base::string16 TimeStampToStandardString(double db);
    // ��SYSTEMTIMEתʱ���time_t
    time_t SystemTimeToTimeStamp(LPSYSTEMTIME pSystime);
    SYSTEMTIME TimeStampToSystemTime(time_t tt);
    // ��byte����תΪ���ʵ�����
    base::string16 ByteSpeedToSuitablySpeed(int64_t speed_int_byte);
    // �жϷ��������صķ���url�ǲ�������޷�������͸��ͼ
    bool CheckValidCover(const std::string &cover_url);
    // �жϸ��״̬�Ի�ȡ��༭״̬
    AlbumEditType GetAlbumEditType(int state);
    // ����״̬�ж��Ǹ���ʾ�������ص���Ϣ
    bool ShouldShowRejectInfo(int album_state);
    // ����Ƿ�����
    bool IsAlbumLocked(int album_state);
    // ��ǩ�Ƿ�ɱ༭
    bool IsTagsEditable(int album_state);
    // ���ݸ��״̬��ȡ��Ӧ�ö�Ӧ���İ���ʾɫ
    SkColor GetStateColor(int state);
    // �û��Ƿ���Ч
    bool IsUserValid();
    // �ϴ�Ȩ�޼��
    bool CheckSubmitAccessAndNotify(HWND hwnd);
    // ����AVSErrorCode�õ�������������ַ���
    base::string16 GetAVSErrorCodeMessage(int code);
}

#endif