#ifndef BILILIVE_BILILIVE_LIVEHIME_HELP_CENTER_H_
#define BILILIVE_BILILIVE_LIVEHIME_HELP_CENTER_H_

#include "ui/views/view.h"
#include "ui/views/controls/button/button.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"

namespace livehime
{
    enum class HelpType
    {
        None = -1,
        Home,
        D3DCompiler,    // 1.�Ҳ��� D3DCompiler ���

        // ����Դ��صİ���
        SourceMonitor,  // 5. ����ץȡ��ʾ������
        SourceGame,  // 16. ĳЩ��Ϸ��ϷԴ�ɼ�������Ϸ����
        SourceGameLOL,// 7. Ӣ�����˽�����Ϸ��û����
        SourceWindow,  // 10. ֱ�����޷�ץ���ṷ��������
        SourceCamera,  // 19. ������ͷ��û�з�Ӧ������ͷ����ʧ����
        SourceProjection,  // 12. �ֻ��޷�Ͷ����ֱ����
        SourceMedia,  // 27. ���ʹ��ֱ�����ɼ�PS4/switch�Ļ���
        SourceMediaCaptureOption,  // 33. ֱ����ץȡ��ý���ʱ����Ƶ����������ѡ��ֱ�

        ThirdPartyStreaming,  // 37. ����������ģʽ��ʾ���ؿ�������ʧ��
        OptimizeForPerformance,  // 31. ֱ�����ڡ�Ϊ�����Ż�����ѡ����������ʲô��
        ColivePkVerTooLow,  // 24. ֱ���������°汾����ʾֱ���汾���Ͳ�֧��PK

        VoiceMicVSysI,  // 9.��˷���������ϵͳû����
        VoiceMicI,  // 23. ��˷�û������
        HardwareEncoderError, // 22. ��Ӳ��������ʾ�޷���ʼ���������޷�¼�ƣ���ʾ������
        SoftwareEncoderError, // 32. ֱ����Ӳ�������������룬��ô���û��������ǵ�����
        SoundEffectFreeze, // 34. ֱ������Ч�����еĶ���ģʽ��ʲô���ã�

        VtuberFigma,      // 38. ���ѡ���ϴ���������������
        VtuberFace,       // 39. ��ε����沿��׽������
    };

    class HelpSupportView :
        public views::View,
        public views::ButtonListener
    {
    public:
        HelpSupportView(views::View* view, HelpType type,
            int h_padding = kPaddingColWidthForCtrlTips);
        ~HelpSupportView() = default;

    protected:
        // View

        // ButtonListener
        void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    private:
        HelpType help_type_ = HelpType::None;
    };

    class HelpSupport
    {
    public:
        // �Ƿ���Ҫ��ʾ����
        void EnableShowHelp(bool enable, HelpType help_type);

        bool ShowHelp(views::View* anchor_view = nullptr, bool show = true);

    protected:
        virtual ~HelpSupport() = default;

        bool enable_help() const { return enable_help_; }
        HelpType help_type() const { return help_type_; }

    private:
        bool enable_help_ = false;
        HelpType help_type_ = HelpType::None;
    };

    bool ShowHelp(HelpType type);
    bool ShowHelp(HelpType type, views::View* anchor_view, bool show);
}
#endif