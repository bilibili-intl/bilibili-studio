#include <iostream>
#include "flv-dumper.hpp"

int main(int argc, char **argv)
{
    if (argc >= 2)
    {
        std::cout << argv[1] << "\n";
        std::vector<char> buf(655360);
        FLVParser flv;
        auto inputSource = flv::CreateFileDataProvider(argv[1]);
        int packetCount = 0;
        while (!inputSource->eos())
        {
            int rb = inputSource->read(buf.data(), buf.size());
            if (rb <= 0)
                break;
            if (!flv.send_data(buf.data(), rb))
                break;
            FLVParser::Result r = FLVParser::NeedMoreData;
            do
            {
                FLVTag packet;
                r = flv.receive_packet(packet);
                if (r == FLVParser::Data)
                {
                    std::cout << "Packet=" << ++packetCount << "\t";
                    std::cout << "OffsetInStream=" << packet.offsetInStream << "\t";
                    std::cout << "Timestamp=" << packet.timestamp << "\t";
                    std::cout << "TagType=" << packet.str_tag_type() << "\t";
                    std::cout << "DataOffset=" << packet.get_payload()->offset() << "\t";
                    std::cout << "DataLen=" << packet.get_payload()->length() << "\t";
                    switch (packet.tagType)
                    {
                    case 8:
                        std::cout << "[" << packet.str_sound_format() << "] ";
                        std::cout << "[" << packet.str_sound_rate() << "] ";
                        std::cout << "[" << packet.str_sound_size() << "] ";
                        std::cout << "[" << packet.str_sound_type() << "] ";
                        std::cout << "[" << packet.str_aac_packet_type() << "]\n";
                        break;
                    case 9:
                        std::cout << "[" << packet.str_frame_type() << "] ";
                        std::cout << "[" << packet.str_codec_id() << "] ";
                        std::cout << "[" << packet.str_avc_type() << "]";
                        if (packet.videoHeader.avcPacketType == FLVTag::AVCType::AVCData)
                        {
                            std::cout << " CTSOffset=" << packet.videoHeader.compositionTime << "";
                        }
                        std::cout << "\n";
                        break;
                    default:
                        std::cout << "\n";
                        break;
                    }
                }
            } while (r == FLVParser::Data);
            if (r == FLVParser::Error)
                break;
        }
    }
    else
    {
        std::cout << "Usage: " << argv[0] << " <filepath> [output time file]\n";
    }
}