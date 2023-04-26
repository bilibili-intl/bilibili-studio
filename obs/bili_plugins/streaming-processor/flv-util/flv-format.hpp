#pragma once

#include <iostream>
#include <string>

#include "flv-util.hpp"

class AudioHeader
{
public:
    int soundFormat;
    int soundRate;
    int soundSize;
    int soundType;
    // IF SoundFormatMap == 10
    int aacPacketType;
};

class VideoHeader
{
public:
    int frameType;
    int codecID;
    // IF CodecID == 7
    int avcPacketType;
    /*
    IF CodecID == 7
    IF AVCPacketType == 1
    Composition time offset
    ELSE
    0
    */
    int compositionTime;
};

class FLVTag
{
    flv::DataSpanPtr payload;

public:
    enum TagType
    {
        Audio = 0x8,
        Video = 0x9,
        Script = 0x12
    };

    enum AVCType
    {
        AVCHeader = 0,
        AVCData = 1,
        EOS = 2
    };

    enum AACType
    {
        AACHeader = 0,
        AACData = 1
    };

    enum AudioFmt
    {
        AAC = 10
    };

    enum VideoFmt
    {
        AVC = 7,
        HEVC = 12
    };

    enum FrameType
    {
        KeyFrame = 1,
        InterFrame = 2,
        DisposableFrame = 3
    };

    FLVTag()
    {
    }

    FLVTag(const FLVTag&) = delete;
    const FLVTag& operator = (const FLVTag&) = delete;

    virtual ~FLVTag() {}

    int tagType; // FLV TagType
    std::uint64_t dataSize; // FLV DataSize + 11 = tagsize
    int timestamp; // FLV Timestamp
    AudioHeader audioHeader;
    VideoHeader videoHeader;

    std::uint64_t offsetInStream;
    std::uint64_t dataOffsetInTag;
    std::uint64_t tagSize; // FLV TagSize, in the end of tag

    void set_payload(flv::DataSpanPtr newPayload)
    {
        payload = newPayload;
        if(payload)
            dataSize = payload->length() + dataOffsetInTag - 11;
        else
            dataSize = dataOffsetInTag - 11;

        tagSize = dataSize + 11;
    }

    flv::DataSpanPtr get_payload()
    {
        return payload;
    }


    // ==================== General

    std::string str_tag_type()
    {
        switch (tagType)
        {
        case TagType::Audio:
            return "Audio";
        case TagType::Video:
            return "Video";
        case TagType::Script:
            return "Script";
        default:
            return "Unknown";
        }
    }

    // ==================== Audio

    const char* str_sound_format()
    {
        switch (audioHeader.soundFormat)
        {
        case 0:
            return "Linear PCM, platform endian";
        case 1:
            return "ADPCM";
        case 2:
            return "MP3";
        case 3:
            return "Linear PCM, little endian";
        case 4:
            return "Nellymoser 16 kHz mono";
        case 5:
            return "Nellymoser 8 kHz mono";
        case 6:
            return "Nellymoser";
        case 7:
            return "G.711 A-law logarithmic PCM";
        case 8:
            return "G.711 mu-law logarithmic PCM";
        case 9:
            return "reserved";
        case 10:
            return "AAC";
        case 11:
            return "Speex";
        case 14:
            return "MP3 8 kHz";
        case 15:
            return "Device-specific sound";
        default:
            return "Unknown";
        }
    }

    const char* str_sound_rate()
    {
        switch (audioHeader.soundRate)
        {
        case 0:
            return "5.5 kHz";
        case 1:
            return "11 kHz";
        case 2:
            return "22 kHz";
        case 3:
            return "44 kHz";
        default:
            return "Unknown";
        }
    }

    const char* str_sound_size()
    {
        switch (audioHeader.soundSize)
        {
        case 0:
            return "8-bit";
        case 1:
            return "16-bit";
        default:
            return "Unknown";
        }
    }

    const char* str_sound_type()
    {
        switch (audioHeader.soundType)
        {
        case 0:
            return "Mono";
        case 1:
            return "Stereo";
        default:
            return "Unknown";
        }
    }

    const char* str_aac_packet_type()
    {
        switch (audioHeader.aacPacketType)
        {
        case AACType::AACHeader:
            return "AAC sequence header";
        case AACType::AACData:
            return "AAC raw";
        default:
            return "Unknown";
        }
    }

    // ==================== Video

    const char* str_frame_type()
    {
        switch (videoHeader.frameType)
        {
        case 1:
            return "key frame";
        case 2:
            return "inter frame";
        case 3:
            return "disposable inter frame";
        case 4:
            return "generated key frame";
        case 5:
            return "video info/command frame";
        default:
            return "Unknown";
        }
    }

    const char* str_codec_id()
    {
        switch (videoHeader.codecID)
        {
        case 2:
            return "Sorenson H.263";
        case 3:
            return "Screen video";
        case 4:
            return "On2 VP6";
        case 5:
            return "On2 VP6 with alpha channel";
        case 6:
            return "Screen video version 2";
        case 7:
            return "AVC";
        case 12:
            return "HEVC";
        default:
            return "Unknown";
        }
    }

    const char* str_avc_type()
    {
        switch (videoHeader.avcPacketType)
        {
        case AVCType::AVCHeader:
            return "AVC sequence header";
        case AVCType::AVCData:
            return "AVC NALU";
        case AVCType::EOS:
            return "AVC end of sequence";
        default:
            return "Unknown";
        }
    }
};
