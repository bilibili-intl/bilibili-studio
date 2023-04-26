#pragma once
#include "base/command_line.h"
#include "base/memory/singleton.h"
#include "base/strings/string_split.h"


// ֧�ֵĿ��Ʋ���
enum class PluggableType
{
    Active,     // ����ö�������
    StartLive,  // ����
    //StopLive,  // �ز�
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
// ��������չ���ܿ������������������кܶ෽ʽ�����𡢿���ֱ������
// ����Щ��Ӧ��������PluggableController���й���
// Ŀǰ֧�ֵķ�ʽֻ�����֣��ܵ���scheme���ܵ����ڵ���������obs�����scheme����Web����
// scheme�����Ͼ���ͨ������������������ֱ�������п��ƣ������պ�������Ҫ��
// Ҳ�����Ը������������ķ�ʽ����ֱ�������������Ϣ�򵽱�׼��������Ա�����������
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
