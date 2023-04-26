#pragma once
#include "base/command_line.h"
#include "base/memory/singleton.h"
#include "base/strings/string_split.h"


// 支持的控制操作
enum class PluggableType
{
    Active,     // 激活并置顶主界面
    StartLive,  // 开播
    //StopLive,  // 关播
    OpenUnity2D,
    OpenUnityZstar,
    OPenInteractivePlay,
    OPenTogetherPlay,
    OPenVoiceLink,
    OpenPreviewDebug
};
struct PluggableInfo {
    PluggableType type;
    std::string params;
};
// 第三方扩展功能控制器，第三方可以有很多方式来唤起、控制直播姬，
// 把这些响应都收束到PluggableController进行管理；
// 目前支持的方式只有两种：管道和scheme，管道用于第三方推流obs插件，scheme用于Web请求；
// scheme本质上就是通过命令行启动参数对直播姬进行控制，所以日后如有需要，
// 也可以以附加启动参数的方式控制直播姬，把输出信息打到标准输出流中以便第三方检测结果
class PluggableController
{
public:
    static PluggableController* GetInstance();

    void ScheduleTransmitCmdline();
    void ScheduleCurrentProcessCmdline();
    std::wstring GetParameterCmd() { return parameter_cmd_; }

private:
    PluggableController();
    virtual ~PluggableController();

    void ScheduleCmdline(CommandLine* cmdline);
    void SchedulePluggableOperate(PluggableType type, const base::StringPairs& params);

private:
    friend struct DefaultSingletonTraits<PluggableController>;
    std::wstring  parameter_cmd_;


};
