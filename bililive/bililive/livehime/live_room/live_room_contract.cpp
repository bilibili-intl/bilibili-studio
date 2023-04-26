#include "bililive/bililive/livehime/live_room/live_room_contract.h"

#include "base/logging.h"



void LiveRestartMode::SetRecording()
{
    modes_.set(RecordingBit);
}

void LiveRestartMode::SetJustRestartStream()
{
    DCHECK(!IsRestartStreamBitsSet());
    ClearRestartStreamBits();

    modes_.set(JustRestartStreamBit);
}

void LiveRestartMode::SetUseSpecialAddr()
{
    DCHECK(!IsRestartStreamBitsSet());
    ClearRestartStreamBits();

    modes_.set(UseSpecialAddrBit);
}

void LiveRestartMode::SetRedetermineStreamAddr()
{
    DCHECK(!IsRestartStreamBitsSet());
    ClearRestartStreamBits();

    modes_.set(RedetermineStreamAddrBit);
}

void LiveRestartMode::SetStreamUntilSpeedTest()
{
    DCHECK(!IsRestartStreamBitsSet());
    ClearRestartStreamBits();

    modes_.set(StreamUntilSpeedTestBit);
}

void LiveRestartMode::ClearRestartStreamBits()
{
    bool record = IsRecordingSet();
    Clear();

    if (record)
    {
        SetRecording();
    }
}

bool LiveRestartMode::IsRestartStreamBitsSet() const
{
    return IsJustRestartStreamSet() || IsUseSpecialAddrSet() ||
        IsRedetermineStreamAddrSet() || IsStreamUntilSpeedTestSet();
}

unsigned long LiveRestartMode::restart_stream_bits_to_ulong() const
{
    std::bitset<TotalBitsCount> mode = modes_;
    mode.reset(RecordingBit);

    return mode.to_ulong();
}

void LiveRestartMode::SetRestartStreamBitsFromUlong(unsigned long ul)
{
    std::bitset<TotalBitsCount> mode(ul);
    bool record = IsRecordingSet();
    modes_ = mode;
    if (record)
    {
        SetRecording();
    }
}

void LiveRestartMode::set_use_special_addr_param(const UseSpecialAddrParam& data)
{
    special_addr_data_ = data;
}

const LiveRestartMode::UseSpecialAddrParam& LiveRestartMode::use_special_addr_param() const
{
    return special_addr_data_;
}

void LiveRestartMode::Clear()
{
    special_addr_data_ = {};
    modes_.reset();
}

size_t LiveRestartMode::Count() const
{
    return modes_.count();
}

std::string LiveRestartMode::to_string() const
{
    return modes_.to_string();
}
