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

        // 重推流的几个标识位是互斥的，优先级随着值由小至大，高优先级的重推流程会涵盖低优先级的流程
        JustRestartStreamBit,       // 重推流，不需要重新确定推流地址，即不需要重开播（先关再开）
        UseSpecialAddrBit,          // 重推流，使用指定的推流地址，不需要重开播，如果这个地址推不上，要回退到重推流之前在用的那个旧地址
        RedetermineStreamAddrBit,   // 重推流，需要重开播以确定新的推流地址（先关再开，但其实当前的实际情况不会关播，是直接重调开播接口）
        StreamUntilSpeedTestBit,    // 重推流，需要先测速，测完重新开播以确定新的推流地址

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
