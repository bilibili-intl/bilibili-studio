#include "ios_bridge.h"

#include <chrono>
#include <string>
#include <vector>


namespace {

    enum PTFrameType {
        PT_FRAME_NONE         = 0,
        PT_FRAME_DEVICE_INFO  = 100,
        PT_FRAME_TEXT_MESSAGE = 101,
        PT_FRAME_PING         = 102,
        PT_FRAME_PONG         = 103,
        PT_FRAME_JSON         = 105,
        PT_FRAME_START_RECV   = 109,
        PT_FRAME_FLV          = 110,
        PT_FRAME_STOP_RECV    = 111,
        PT_FRAME_END          = 112,
    };

    static const unsigned long kPTProtocolVersion1 = 1;
    static const unsigned long kPTIdentifierPC = 1;

    void w32b(unsigned char* s, unsigned long val, unsigned char** p) {
        *s++ = (val >> 24) & 0xFF;
        *s++ = (val >> 16) & 0xFF;
        *s++ = (val >> 8) & 0xFF;
        *s++ = (val >> 0) & 0xFF;
        if (p) *p = s;
    }

    unsigned long r32b(unsigned char* s, unsigned char** p) {
        auto val = ((unsigned long)(*s) << 24) |
            ((unsigned long)(*(s + 1)) << 16) |
            ((unsigned long)(*(s + 2)) << 8) |
            ((unsigned long)(*(s + 3)) << 0);
        if (p) *p = s + 4;
        return val;
    }

    unsigned long r32b(const unsigned char* s, const unsigned char** p) {
        auto val = ((unsigned long)(*s) << 24) |
            ((unsigned long)(*(s + 1)) << 16) |
            ((unsigned long)(*(s + 2)) << 8) |
            ((unsigned long)(*(s + 3)) << 0);
        if (p) *p = s + 4;
        return val;
    }

    unsigned int r16b(const unsigned char* s, const unsigned char** p) {
        auto val =
            ((unsigned int)(*(s + 0)) << 8) |
            ((unsigned int)(*(s + 1)) << 0);
        if (p) *p = s + 2;
        return val;
    }

    unsigned long r24b(const unsigned char* s, const unsigned char** p) {
        auto val =
            ((unsigned long)(*(s + 0)) << 16) |
            ((unsigned long)(*(s + 1)) << 8) |
            ((unsigned long)(*(s + 2)) << 0);
        if (p) *p = s + 3;
        return val;
    }

    bool flv_parse_tag(const unsigned char* data, size_t len) {
        auto s = data;
        auto type = *s++ & 0x1F;
        auto data_size = r24b(s, &s);
        auto timestamp = r24b(s, &s);
        timestamp |= (unsigned long)(*s++) << 24;
        auto stream_id = r24b(s, &s);

        if (type == 0x08) {
            // audio
        } else if (type == 0x09) {
            // video
            auto tmp = *s++;
            auto frame_type = tmp >> 4;
            auto codec_id = tmp & 0xF;
            if (codec_id == 7) {
                auto avc_type = *s++;
                auto c_time = r24b(s, &s);

                std::vector<unsigned char*> spss;
                std::vector<unsigned char*> ppss;
                if (avc_type == 0) {
                    auto config_ver = *s++;
                    auto pf_id = *s++;
                    auto pf_comp = *s++;
                    auto lv_id = *s++;
                    auto length = (*s++) & 0x3;
                    auto sps_num = (*s++) & 0x1F;
                    for (int i = 0; i < sps_num; ++i) {
                        auto _l = r16b(s, &s);
                        auto sps = new unsigned char[_l];
                        std::memcpy(sps, s, _l); s += _l;
                        spss.push_back(sps);
                    }

                    auto pps_num = *s++;
                    for (int i = 0; i < pps_num; ++i) {
                        auto _l = r16b(s, &s);
                        auto pps = new unsigned char[_l];
                        std::memcpy(pps, s, _l); s += _l;
                        ppss.push_back(pps);
                    }
                    int sdg = 0;
                } else if (avc_type == 1) {
                    int sdg = 0;
                }
            }
        } else if (type == 0x12) {
            // metadata
            return false;
        }

        return true;
    }

}

namespace ios {

    // static
    int IOSBridge::GetDevices(std::vector<std::string>* ids) {
        char** devs;
        int dev_count;
        auto ret = idevice_get_device_list(&devs, &dev_count);
        if (ret != IDEVICE_E_SUCCESS) {
            return RC_E_NO_SERVICE;
        }
        if (dev_count <= 0) {
            return RC_E_NO_DEVICE;
        }

        ids->clear();
        for (int i = 0; i < dev_count; ++i) {
            ids->emplace_back(devs[i]);
        }

        idevice_device_list_free(devs);
        return RC_OK;
    }

    IOSBridge::IOSBridge()
        : pt_frame_{},
          auth_result_(false),
          is_running_(false) {}

    IOSBridge::~IOSBridge() {
        Close();
    }

    int IOSBridge::Connect(uint16_t port, const char* udid) {
        Close();
        is_running_.store(true, std::memory_order_relaxed);

        idevice_t dev;
        auto ret = idevice_new(&dev, udid);
        if (ret != IDEVICE_E_SUCCESS) {
            return RC_E_CREATE_DEVICE;
        }
        imobile_ = dev;

        idevice_connection_t conn;
        ret = idevice_connect(dev, port, &conn);
        if (ret != IDEVICE_E_SUCCESS) {
            return RC_E_CONNECT;
        }
        imobile_conn_ = conn;

        if (!Auth()) {
            Close();
            return RC_E_AUTH;
        }

        return RC_OK;
    }

    void IOSBridge::Disconnect() {
        is_running_.store(false, std::memory_order_relaxed);
    }

    void IOSBridge::Close() {
        Disconnect();

        if (worker_.joinable()) {
            worker_.join();
        }

        if (imobile_conn_) {
            idevice_disconnect(imobile_conn_);
            imobile_conn_ = 0;
        }

        if (imobile_) {
            idevice_free(imobile_);
            imobile_ = 0;
        }

        if (ptfd_pos_ >= 0) {
            delete[] pt_frame_.data;
            ptfd_pos_ = -1;
        }

        auth_result_.store(
            false, std::memory_order_relaxed);
    }

    bool IOSBridge::Subscribe() {
        if (!imobile_conn_) {
            return false;
        }

        PTFrame frame;
        frame.version = kPTProtocolVersion1;
        frame.type = PT_FRAME_START_RECV;
        frame.tag = 0;
        frame.payload_size = 0;
        frame.identifier = kPTIdentifierPC;
        frame.data = nullptr;

        auto ret = pt_send_frame(imobile_conn_, frame);
        if (ret != IDEVICE_E_SUCCESS) {
            return false;
        }

        return true;
    }

    bool IOSBridge::Unsubscribe() {
        if (!imobile_conn_) {
            return false;
        }

        PTFrame frame;
        frame.version = kPTProtocolVersion1;
        frame.type = PT_FRAME_STOP_RECV;
        frame.tag = 0;
        frame.payload_size = 0;
        frame.identifier = kPTIdentifierPC;
        frame.data = nullptr;

        auto ret = pt_send_frame(imobile_conn_, frame);
        if (ret != IDEVICE_E_SUCCESS) {
            return false;
        }

        return true;
    }

    bool IOSBridge::Auth() {
        PTFrame frame;
        frame.version = kPTProtocolVersion1;
        frame.type = PT_FRAME_PING;
        frame.tag = 0;
        frame.payload_size = 0;
        frame.identifier = kPTIdentifierPC;
        frame.data = nullptr;

        auto ret = pt_send_frame(imobile_conn_, frame);
        if (ret != IDEVICE_E_SUCCESS) {
            return false;
        }

        worker_ = std::thread(&IOSBridge::OnAuthWork, this);
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1s);
        if (!auth_result_.load(std::memory_order_relaxed)) {
            return false;
        }

        return true;
    }

    int IOSBridge::Recv(unsigned char* buf, size_t len) {
        if (!imobile_conn_) {
            return RC_E_RECV;
        }

        if (ptfd_pos_ >= 0) {
            if (pt_frame_.payload_size > (unsigned long)ptfd_pos_) {
                auto rem_len = pt_frame_.payload_size - ptfd_pos_;
                auto act_len = (std::min)(rem_len, (unsigned long)len);

                std::memcpy(
                    buf,
                    pt_frame_.data + ptfd_pos_,
                    act_len);
                ptfd_pos_ += act_len;
                return act_len;
            }

            delete[] pt_frame_.data;
            ptfd_pos_ = -1;
        }

        for (;;) {
            PTFrame frame;
            auto ret = pt_recv_frame(imobile_conn_, is_running_, &frame);
            if (ret != IDEVICE_E_SUCCESS) {
                return RC_E_RECV;
            }

            if (frame.version != kPTProtocolVersion1) {
                delete[] frame.data;
                continue;
            }

            switch (frame.type) {
            case PT_FRAME_DEVICE_INFO:
            {
                std::string text((char*)frame.data, frame.payload_size);
                delete[] frame.data;
                continue;
            }

            case PT_FRAME_FLV:
            {
                auto act_len = (std::min)(frame.payload_size, (unsigned long)len);
                if (len < frame.payload_size) {
                    ptfd_pos_ = act_len;
                    pt_frame_ = frame;
                    std::memcpy(buf, frame.data, act_len);
                } else {
                    std::memcpy(buf, frame.data, frame.payload_size);
                    delete[] frame.data;
                }
                return act_len;
            }

            case PT_FRAME_END:
                delete[] frame.data;
                return RC_S_END;

            case PT_FRAME_PONG:
                delete[] frame.data;
                break;

            case PT_FRAME_PING:
            case PT_FRAME_TEXT_MESSAGE:
            default:
                delete[] frame.data;
                continue;
            }
        }
    }

    void IOSBridge::OnAuthWork() {
        PTFrame frame;

        for (;;) {
            auto ret = pt_recv_frame(imobile_conn_, is_running_, &frame);
            if (ret != IDEVICE_E_SUCCESS) {
                return;
            }

            switch (frame.type) {
            case PT_FRAME_PONG:
                auth_result_.store(true, std::memory_order_relaxed);
                delete[] frame.data;
                return;

            default:
                delete[] frame.data;
                continue;
            }
        }
    }

    // static
    idevice_error_t IOSBridge::pt_speed_test(idevice_connection_t conn) {
        char buf[1024 * 8];
        uint32_t act_len;
        uint32_t wan_len = 1024 * 8;

        // 跳过设备信息
        auto ret = idevice_connection_receive_timeout(conn, buf, wan_len, &act_len, 0);
        if (ret != IDEVICE_E_SUCCESS) {
            return ret;
        }

        // 等第一个包
        ret = idevice_connection_receive_timeout(conn, buf, wan_len, &act_len, 0);
        if (ret != IDEVICE_E_SUCCESS) {
            return ret;
        }

        auto st = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();

        unsigned long long total_size = 0;

        size_t count = 50 * 1024 * 1024 / wan_len;
        for (size_t i = 0; i < count; ++i) {
            ret = idevice_connection_receive_timeout(conn, buf, wan_len, &act_len, 0);
            if (ret != IDEVICE_E_SUCCESS) {
                return ret;
            }

            total_size += act_len;
        }

        auto et = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();

        auto speed = total_size / ((et - st) / 1000000.0) / 1024.0 / 1024.0;

        return IDEVICE_E_SUCCESS;
    }

    // static
    idevice_error_t IOSBridge::pt_send_frame(idevice_connection_t conn, const PTFrame& frame) {
        char* buf = new char[20 + frame.payload_size];
        auto s = (unsigned char*)buf;

        w32b(s, frame.version, &s);
        w32b(s, frame.type, &s);
        w32b(s, frame.tag, &s);
        w32b(s, frame.payload_size, &s);
        w32b(s, frame.identifier, &s);

        if (frame.data) {
            std::memcpy(s, frame.data, frame.payload_size);
        }

        uint32_t act_len;
        auto ret = idevice_connection_send(
            conn, buf, 20 + frame.payload_size, &act_len);
        delete[] buf;
        return ret;
    }

    // static
    idevice_error_t IOSBridge::pt_recv_frame(
        idevice_connection_t conn, std::atomic_bool& ctrl, PTFrame* frame)
    {
        char buf[512];
        uint32_t act_len = 0;
        uint32_t want_len = 20;
        idevice_error_t ret = IDEVICE_E_UNKNOWN_ERROR;

        auto src = buf;
        for (;;) {
            for (;;) {
                if (!ctrl.load(std::memory_order_relaxed)) {
                    return IDEVICE_E_UNKNOWN_ERROR;
                }
                ret = idevice_connection_receive_timeout(conn, src, want_len, &act_len, 1000);
                if (ret != IDEVICE_E_TIMEOUT) {
                    break;
                }
            }
            if (ret != IDEVICE_E_SUCCESS) {
                return ret;
            }

            if (act_len >= want_len) {
                break;
            }

            src += act_len;
            want_len -= act_len;
        }

        auto b = (unsigned char*)buf;
        frame->version = r32b(b, &b);
        frame->type = r32b(b, &b);
        frame->tag = r32b(b, &b);
        frame->payload_size = r32b(b, &b);
        frame->identifier = r32b(b, &b);
        if (frame->payload_size > 0) {
            frame->data = new unsigned char[frame->payload_size];
        } else {
            frame->data = nullptr;
        }

        if (frame->payload_size > 0) {
            size_t cur_len = 0;
            auto dst = frame->data;
            ret = IDEVICE_E_UNKNOWN_ERROR;
            for (;;) {
                want_len = (uint32_t)(frame->payload_size - cur_len);
                for (;;) {
                    if (!ctrl.load(std::memory_order_relaxed)) {
                        return IDEVICE_E_UNKNOWN_ERROR;
                    }
                    ret = idevice_connection_receive_timeout(conn, (char*)dst, want_len, &act_len, 1000);
                    if (ret != IDEVICE_E_TIMEOUT) {
                        break;
                    }
                }
                if (ret != IDEVICE_E_SUCCESS) {
                    delete[] frame->data;
                    return ret;
                }

                cur_len += act_len;
                if (cur_len >= frame->payload_size) {
                    break;
                }
                dst += act_len;
            }
        }

        return IDEVICE_E_SUCCESS;
    }

}