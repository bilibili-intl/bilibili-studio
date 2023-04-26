#ifndef BILILIVE_BILILIVE_LIVEHIME_LIVE_ROOM_CONTRACT_H_
#define BILILIVE_BILILIVE_LIVEHIME_LIVE_ROOM_CONTRACT_H_

#include <bitset>


enum class StartLiveRoomType
{
    Normal = 0,
    Studio = 5,
    Melee = 6,
};

enum class StartLiveRoomErrorCode
{
    Authentication = 60014,
    AuthAccountAnomaly = 60024
};

enum class StartLiveNoticeType
{
    Undefine,
    AuthNeedSupplement = 1,
};

enum class StartLiveNoticeStatus
{
    Inactive,
    ShouldHandle = 1,
};

class LiveRestartMode
{
private:
    enum : size_t
    {
        RecordingBit = 0,

        // �������ļ�����ʶλ�ǻ���ģ����ȼ�����ֵ��С���󣬸����ȼ����������̻ậ�ǵ����ȼ�������
        JustRestartStreamBit,       // ������������Ҫ����ȷ��������ַ��������Ҫ�ؿ������ȹ��ٿ���
        UseSpecialAddrBit,          // ��������ʹ��ָ����������ַ������Ҫ�ؿ�������������ַ�Ʋ��ϣ�Ҫ���˵�������֮ǰ���õ��Ǹ��ɵ�ַ
        RedetermineStreamAddrBit,   // ����������Ҫ�ؿ�����ȷ���µ�������ַ���ȹ��ٿ�������ʵ��ǰ��ʵ���������ز�����ֱ���ص������ӿڣ�
        StreamUntilSpeedTestBit,    // ����������Ҫ�Ȳ��٣��������¿�����ȷ���µ�������ַ

        TotalBitsCount
    };

public:
    LiveRestartMode() = default;
    ~LiveRestartMode() = default;

#define DECLARE_RESTART_MEMBER_FUNC(name)  \
    void Clear##name()                     \
    {                                      \
        modes_.reset(name##Bit);           \
    }                                      \
                                           \
    bool Is##name##Set() const             \
    {                                      \
        return modes_.test(name##Bit);     \
    }

    DECLARE_RESTART_MEMBER_FUNC(Recording)
    DECLARE_RESTART_MEMBER_FUNC(JustRestartStream)
    DECLARE_RESTART_MEMBER_FUNC(UseSpecialAddr)
    DECLARE_RESTART_MEMBER_FUNC(RedetermineStreamAddr)
    DECLARE_RESTART_MEMBER_FUNC(StreamUntilSpeedTest)

    void SetRecording();
    void SetJustRestartStream();
    void SetUseSpecialAddr();
    void SetRedetermineStreamAddr();
    void SetStreamUntilSpeedTest();

    void ClearRestartStreamBits();
    bool IsRestartStreamBitsSet() const;

    unsigned long restart_stream_bits_to_ulong() const;
    void SetRestartStreamBitsFromUlong(unsigned long ul);

    // const std::string& addr, const std::string &code, const std::string &prtc
    typedef std::tuple<std::string, std::string, std::string> UseSpecialAddrParam;
    void set_use_special_addr_param(const UseSpecialAddrParam& data);
    const UseSpecialAddrParam& use_special_addr_param() const;

    void Clear();
    size_t Count() const;
    std::string to_string() const;

private:
    std::bitset<TotalBitsCount> modes_;

    UseSpecialAddrParam special_addr_data_;
};

#endif
