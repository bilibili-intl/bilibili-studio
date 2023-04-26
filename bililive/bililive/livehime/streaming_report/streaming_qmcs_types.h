#pragma once

#include "base/basictypes.h"

enum StreamingSampleType
{
    NDF,
    RDF,
    EDF,
    GPU,
    CPU,
    MEM,
    P_CPU,
    P_MEM,
    NET_NEED_TOTAL_FRAMES,
    SRT_SEND_PKTS,
    SRT_LOST_PKTS,
    SRT_DROP_PKTS,
    SRT_RETTRANS_PKTS,
    SRT_RTT_MS,
    SRT_SEND_RATE_KBPS,
    COUNT,
};

struct StreamingReportSampleData
{
    int64 ts = 0;
    double data = 0;    // 网络/渲染/编码 0~1.0f；GPU/CPU 0~100；MEM 0~4096

    // 原始数据，通常data是根据raw_value经过计算得到的值
    // 网络，0：丢帧数；1：丢弃的字节数
    // 渲染，0：丢帧数
    // 编码，0：丢帧数
    // 其他的没有使用
    static const int kRawValueCount = 2;
    int raw_value[kRawValueCount] = {0};
};

struct StreamingReportInfo
{
    StreamingReportInfo(int64 ts)
    {
        for (int i = 0; i < StreamingSampleType::COUNT; i++)
        {
            data[i].ts = ts;
        }
    }

    StreamingReportSampleData data[StreamingSampleType::COUNT];
};
