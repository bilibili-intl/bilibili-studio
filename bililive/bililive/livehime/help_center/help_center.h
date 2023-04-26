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
        D3DCompiler,    // 1.找不到 D3DCompiler 组件

        // 场景源相关的帮助
        SourceMonitor,  // 5. 电脑抓取显示器黑屏
        SourceGame,  // 16. 某些游戏游戏源采集不到游戏画面
        SourceGameLOL,// 7. 英雄联盟进入游戏后没画面
        SourceWindow,  // 10. 直播姬无法抓到酷狗的主窗口
        SourceCamera,  // 19. 打开摄像头后没有反应或摄像头启动失败了
        SourceProjection,  // 12. 手机无法投屏到直播姬
        SourceMedia,  // 27. 如何使用直播姬采集PS4/switch的画面
        SourceMediaCaptureOption,  // 33. 直播姬抓取多媒体的时候，音频监听的三个选项分别

        ThirdPartyStreaming,  // 37. 第三方推流模式提示本地开启监听失败
        OptimizeForPerformance,  // 31. 直播姬内“为性能优化”勾选与否的区别是什么？
        ColivePkVerTooLow,  // 24. 直播姬是最新版本还提示直播版本过低不支持PK

        VoiceMicVSysI,  // 9.麦克风有声音、系统没声音
        VoiceMicI,  // 23. 麦克风没有声音
        HardwareEncoderError, // 22. 用硬件编码提示无法开始推流或者无法录制，提示更新显
        SoftwareEncoderError, // 32. 直播姬硬件编码和软件编码，怎么向用户解释他们的优劣
        SoundEffectFreeze, // 34. 直播姬音效设置中的冻结模式有什么作用？

        VtuberFigma,      // 38. 如何选择、上传、下载虚拟形象？
        VtuberFace,       // 39. 如何调节面部捕捉参数？
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
        // 是否需要显示帮助
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