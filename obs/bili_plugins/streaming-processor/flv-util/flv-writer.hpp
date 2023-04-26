#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

#include "flv-format.hpp"

class RealtimeSleep
{
    bool hasFirstDts = false;
    uint32_t startDts;
    std::chrono::steady_clock::time_point startSystemTime;
public:
    void sleep(FLVTag& packet)
    {
        if (packet.tagType == FLVTag::TagType::Video && packet.videoHeader.avcPacketType == FLVTag::AVCType::AVCHeader
            || packet.tagType == FLVTag::TagType::Audio && packet.audioHeader.aacPacketType == FLVTag::AACType::AACHeader)
            hasFirstDts = false;

        if(!hasFirstDts)
        {
            startSystemTime = std::chrono::steady_clock::now();
            startDts = packet.timestamp;
            hasFirstDts = true;
        }
        else
        {
            std::this_thread::sleep_until(startSystemTime + std::chrono::milliseconds(packet.timestamp - startDts));
        }
    }
};

namespace flv
{
    bool write_header(bool hasAudio, bool hasVideo, flv::DataSinkPtr sink)
    {
        unsigned char header[] = {
            0x46, // F
            0x4C, // L
            0x56, // V
            0x01, // Version 1
            0x00, // TypeFlags
            0x00, // DataOffset UI32
            0x00,
            0x00,
            0x09,
            0x00, // PreviousTagSize0 UI32 Always 0
            0x00,
            0x00,
            0x00,
        };
        if (hasAudio)
            header[4] += 0b0100;
        if (hasVideo)
            header[4] += 0b0001;
        int size = sizeof(header) / sizeof(header[0]);
        // fwrite(header, 1, size, outputFilePtr);
        sink->write(header, size);
        return true;
    }

    bool write_tag(FLVTag& tag, flv::DataSinkPtr sink)
    {
        unsigned char generalTagHeader[11] = {0x00};
        generalTagHeader[0] = tag.tagType;
        // DataSize UI24
        generalTagHeader[1] = (tag.dataSize >> 16) & 0b11111111;
        generalTagHeader[2] = (tag.dataSize >> 8) & 0b11111111;
        generalTagHeader[3] = tag.dataSize & 0b11111111;
        // Timestamp UI24
        generalTagHeader[4] = (tag.timestamp >> 16) & 0b11111111;
        generalTagHeader[5] = (tag.timestamp >> 8) & 0b11111111;
        generalTagHeader[6] = tag.timestamp & 0b11111111;
        // TimestampExtended UI8
        generalTagHeader[7] = (tag.timestamp >> 24) & 0b11111111;
        // fwrite(generalTagHeader, 1, 11, outputFilePtr);
        sink->write(generalTagHeader, 11);
        switch (tag.tagType)
        {
        case FLVTag::TagType::Audio:
        {
            unsigned char audioTagHeader = 0x00;
            audioTagHeader += tag.audioHeader.soundFormat << 4; // SoundFormat UB4
            audioTagHeader += tag.audioHeader.soundRate << 2;   // SoundRate UB2
            audioTagHeader += tag.audioHeader.soundSize << 1;   // SoundSize UB1
            audioTagHeader += tag.audioHeader.soundType;        // SoundType UB1
            // fputc(audioTagHeader, outputFilePtr);
            
            sink->write((char*)&audioTagHeader, 1);
            if (tag.audioHeader.soundFormat == FLVTag::AudioFmt::AAC)
            {
                // fputc(tag.audioHeader.aacPacketType, outputFilePtr); // AACPacketType UB8
                sink->write((char*)&tag.audioHeader.aacPacketType, 1); // AACPacketType UB8
            }
        };
        break;

        case FLVTag::TagType::Video:
        {
            unsigned char videoTagHeader = 0x00;
            videoTagHeader += tag.videoHeader.frameType << 4; // Frame Type UB4
            videoTagHeader += tag.videoHeader.codecID;        // CodecID UB4
            // fputc(videoTagHeader, outputFilePtr);
            sink->write((char*)&videoTagHeader, 1);
            if (tag.videoHeader.codecID == FLVTag::VideoFmt::AVC || tag.videoHeader.codecID == FLVTag::VideoFmt::HEVC)
            {
                unsigned char avcFlags[4] = {0x00};
                avcFlags[0] = tag.videoHeader.avcPacketType; // AVCPacketType UB8
                // CompositionTime SI24
                avcFlags[1] = (tag.videoHeader.compositionTime >> 16) & 0b11111111;
                avcFlags[2] = (tag.videoHeader.compositionTime >> 8) & 0b11111111;
                avcFlags[3] = tag.videoHeader.compositionTime & 0b11111111;
                // fwrite(avcFlags, 1, 4, outputFilePtr);
                sink->write(avcFlags, 4);
            }
        }
        break;
        default:
            break;
        }
        return true;
    }

    bool write_data(FLVTag& tag, flv::DataSinkPtr sink, flv::DataSourcePtr src)
    {
        auto payload = tag.get_payload();
        std::vector<char> buf(payload->length());
        payload->getdata(*src, buf.data());
        sink->write(buf.data(), buf.size());
        return true;
    }

    bool write_end_tag(FLVTag& tag, flv::DataSinkPtr sink)
    {
        unsigned char tagSize[4] = {0x00};
        // PreviousTagSizeN UI32
        tagSize[0] = (tag.tagSize >> 24) & 0b11111111;
        tagSize[1] = (tag.tagSize >> 16) & 0b11111111;
        tagSize[2] = (tag.tagSize >> 8) & 0b11111111;
        tagSize[3] = tag.tagSize & 0b11111111;
        // fwrite(tagSize, 1, 4, outputFilePtr);
        sink->write(tagSize, 4);
        return true;
    }

    void write_full_tag(FLVTag& packet, flv::DataSinkPtr outputSink, flv::DataSourcePtr inputSource)
    {
        write_tag(packet, outputSink);
        write_data(packet, outputSink, inputSource);
        write_end_tag(packet, outputSink);
    }
};
