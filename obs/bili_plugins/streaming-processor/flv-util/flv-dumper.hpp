#pragma once

#include <vector>
#include <deque>
#include <algorithm>

#include "flv-format.hpp"

class FLVInputStream
{
    std::vector<char> buf_;

    int readpos_; // 当前buffer内读取位置
    int will_drop_size_; // 将要被丢弃的字节数
    std::uint64_t did_drop_size_; // 已从buffer丢弃的字节数

    void flush_buffer()
    {
        readpos_ = will_drop_size_;
        did_drop_size_ += will_drop_size_;
        buf_.erase(buf_.begin(), buf_.begin() + will_drop_size_);
        will_drop_size_ = 0;
    }

public:
    FLVInputStream()
    {
        readpos_ = 0;
        did_drop_size_ = 0;
        will_drop_size_ = 0;
    }

    void write(const void *ptr, int len)
    {
        if (buf_.capacity() > 1048576 * 50 || capacity() < len)
            flush_buffer();
        buf_.insert(buf_.end(), (const char*)ptr, (const char*)ptr + len);
    }

    std::uint64_t remaining()
    {
        return buf_.size() - readpos_;
    }

    std::uint64_t capacity()
    {
        return buf_.capacity() - readpos_;
    }

    std::uint64_t read(void *ptr, int len)
    {
        size_t readsize = std::min<size_t>(remaining(), len);
        if (ptr != nullptr)
        {
            std::copy_n(buf_.begin() + readpos_, readsize, (char*)ptr);
        }
        readpos_ += readsize;
        return readsize;
    }

    int readByte()
    {
        uint8_t r;
        if (read(&r, 1) == 1)
            return r;
        else
            return EOF;
    }

    std::uint64_t rposition()
    {
        return did_drop_size_ + readpos_;
    }

    void flush()
    {
        will_drop_size_ = readpos_;
    }

    void rewind()
    {
        readpos_ = will_drop_size_;
    }
};

using FLVTagPtr = std::shared_ptr<FLVTag>;
class FLVParser
{
public:
    enum Result
    {
        Error = -1,
        Data = 0,
        NeedMoreData = 1
    };

    enum ParserStatus
    {
        Header,
        Packet,
    };

    FLVParser::ParserStatus status = Header;
    bool hasVideo = false;
    bool hasAudio = false;

    bool send_data(const char *data, int len)
    {
        buf_.write(data, len);
        return true;
    }

    Result receive_packet(FLVTag& tag)
    {
        return process_buffer(tag);
    }

    FLVParser(bool with_payload_data = false)
    {
        with_payload_data_ = with_payload_data;
    }

private:
    FLVInputStream buf_;
    bool with_payload_data_;

    bool parse_tag_header(FLVTag& tag)
    {
        // previous tag size
        buf_.read(nullptr, 4);

        tag.offsetInStream = buf_.rposition();
        std::uint8_t tagFirstByte = buf_.readByte();
        bool filter = getBit(tagFirstByte, 6); // UB1 Pre-processing
        int tagType = tagFirstByte & 0b11111;  // UB5
        std::uint8_t bufDataSize[3];
        buf_.read(bufDataSize, 3);
        int dataSize = (bufDataSize[0] << 16) + (bufDataSize[1] << 8) + bufDataSize[2]; // UI24
        std::uint8_t bufTS[4];
        buf_.read(bufTS, 4);
        int timestamp = (bufTS[3] << 24) + (bufTS[0] << 16) + (bufTS[1] << 8) + bufTS[2]; // UI8<<24 + UI24
        buf_.read(nullptr, 3);                                                            // stream id

        if (filter)
        {
            // std::cerr << "Not support encryption tag.\n";
            return false;
        }

        tag.tagType = tagType;
        tag.dataSize = dataSize;
        tag.tagSize = dataSize + 11;
        tag.timestamp = timestamp;

        return true;
    }
    
    bool parse_av_header(FLVTag& tag)
    {
        if (tag.tagType == FLVTag::TagType::Video)
            return parse_video_header(tag);
        else if (tag.tagType == FLVTag::TagType::Audio)
            return parse_audio_header(tag);
        else if (tag.tagType == FLVTag::TagType::Script)
            return true;
        
        return false;
    }

    bool parse_audio_header(FLVTag& tag)
    {
        int audioTagFlag = buf_.readByte();
        tag.audioHeader.soundFormat = audioTagFlag >> 4;
        tag.audioHeader.soundRate = (audioTagFlag >> 2) & 0b11;
        tag.audioHeader.soundSize = getBit(audioTagFlag, 1);
        tag.audioHeader.soundType = getBit(audioTagFlag, 0);
        if (tag.audioHeader.soundFormat == FLVTag::AudioFmt::AAC)
        { // IF SoundFormat == 10
            tag.audioHeader.aacPacketType = buf_.readByte();
        }

        return true;
    }

    bool parse_video_header(FLVTag& tag)
    {
        int videoTagFlag = buf_.readByte();
        tag.videoHeader.frameType = videoTagFlag >> 4;
        tag.videoHeader.codecID = videoTagFlag & 0b1111;
        if (tag.videoHeader.codecID == FLVTag::VideoFmt::AVC)
        { // IF AVC (CodecID==7)
            tag.videoHeader.avcPacketType = buf_.readByte();
            std::uint8_t bufCTSOffset[3];
            buf_.read(bufCTSOffset, 3);
            tag.videoHeader.compositionTime = (bufCTSOffset[0] << 16) + (bufCTSOffset[1] << 8) + bufCTSOffset[2]; // SI24 IF AVCPacketType == 1 THEN Composition time offset ELSE 0
        }

        return true;
    }

    bool parse_payload(FLVTag& tag)
    {
        tag.dataOffsetInTag = buf_.rposition() - tag.offsetInStream;

        int dataLen = (int)(tag.dataSize + 11 - tag.dataOffsetInTag); // dataSize - headerSize
        if (with_payload_data_)
        {
            std::vector<char> buf(dataLen);
            buf_.read(buf.data(), dataLen);
            tag.set_payload(flv::CreateMemDataSpan(buf.data(), dataLen));
        }
        else
        {
            buf_.read(nullptr, dataLen);
            tag.set_payload(flv::CreateDataSpan(tag.offsetInStream + tag.dataOffsetInTag, dataLen));
        }

        return true;
    }

    Result process_buffer(FLVTag& tag)
    {
        buf_.rewind();
        switch (status)
        {
        case ParserStatus::Header:
            if (buf_.remaining() >= 9) // Header 9 bytes
            {
                if (buf_.readByte() == 0x46 && buf_.readByte() == 0x4C && buf_.readByte() == 0x56 && buf_.readByte() == 0x01)
                {
                    int typeFlags = buf_.readByte();
                    hasVideo = getBit(typeFlags, 0);
                    hasAudio = getBit(typeFlags, 2);
                    // std::cerr << "HasVideo: " << (hasVideo ? "true" : "false") << "\n";
                    // std::cerr << "HasAudio: " << (hasAudio ? "true" : "false") << "\n";
                    //std::cerr << "FLAGS: " << getBit(typeFlags, 0) << getBit(typeFlags, 1) << getBit(typeFlags, 2) << getBit(typeFlags, 3) << getBit(typeFlags, 4) << getBit(typeFlags, 5) << getBit(typeFlags, 6) << getBit(typeFlags, 7) << "\n";
                    // data offset
                    std::uint8_t bufDataoffset[4];
                    buf_.read(bufDataoffset, 4);
                    if (!(bufDataoffset[0] == 0 && bufDataoffset[1] == 0 && bufDataoffset[2] == 0 && bufDataoffset[3] == 9)) // 如果长度不为9则报错
                    {
                        return Result::Error;
                    }

                    status = ParserStatus::Packet;
                    buf_.flush();
                    if (buf_.remaining() >= 11 + 4) // 4 bytes previous tag size and 11 bytes TagHeader (until end of StreamID).
                    { // Has a full TagHeader (end of StreamID)
                        return process_buffer(tag);
                    }
                }
                else
                {
                    // std::cerr << "Not a FLV Video.\n";
                    return Result::Error;
                }
            }
            return Result::NeedMoreData;
            break;

        case ParserStatus::Packet:
        {
            if (buf_.remaining() < 11 + 4) // 4 bytes previous tag size and 11 bytes TagHeader (until end of StreamID).
                return Result::NeedMoreData;
            if (!parse_tag_header(tag))
                return Result::Error;

            if (buf_.remaining() < tag.dataSize + 11 + 4) // datasize Equal to length of the tag – 11
                return Result::NeedMoreData;

            if (parse_av_header(tag) && parse_payload(tag))
            {
                buf_.flush();
                return Result::Data;
            }
            else
                return Result::Error;
            break;
        }

        default:
            // std::cerr << "Unknown Status.\n";
            return Result::Error;
            break;
        }
    }

    bool getBit(unsigned char byte, int position) // Position in range 0-7
    {
        return (byte >> position) & 0x1;
    }
};
