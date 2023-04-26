#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "avc-sei-util.h"

struct AVCSPS
{
    int w;
    int h;
    int sar_w;
    int sar_h;

    int max_frame_num;
    int max_poc_lsb;
    int bits;
};

struct AVCPPS
{
};

struct AVCSlice
{
    int ref_idc;
    int slice_type;
    int frame_num;
    int field_pic_flag;
    int idr_pic_id;
    int pic_order_cnt_lsb;
};

struct AVCSEI
{
    std::vector<sei_msg_t> sei_msgs;
};

struct AVCOther
{
    int nal_unit_type;
};

struct AVCNalu
{
    std::tuple<AVCSPS,bool> sps;
    std::tuple<AVCPPS,bool> pps;
    std::tuple<AVCSEI,bool> sei;
    std::tuple<AVCSlice,bool> slice;
    std::tuple<AVCOther,bool> others;
    AVCNalu()
        :sps(AVCSPS(), false)
        ,pps(AVCPPS(),false)
        ,sei(AVCSEI(),false)
        ,slice(AVCSlice(),false)
        , others(AVCOther(),false)
    {}
};

class IAVCParser
{
public:
    virtual ~IAVCParser() {}
    virtual std::vector<AVCNalu> ParseAVCC(char* buf, int len) = 0;
    virtual std::vector<AVCNalu> ParseNalu(char* buf, int len) = 0;
};

IAVCParser* CreateAVCParser();
