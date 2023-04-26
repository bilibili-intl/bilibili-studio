#ifndef OBS_RECEIVER_IOS_BRIDGE_H_
#define OBS_RECEIVER_IOS_BRIDGE_H_

#include "libimobiledevice/libimobiledevice.h"

#include <atomic>
#include <string>
#include <thread>
#include <vector>


namespace ios {

    class IOSBridge {
    public:
        enum RetCode {
            RC_OK = 0,
            RC_E_NO_SERVICE,
            RC_E_NO_DEVICE,
            RC_E_AUTH,
            RC_E_CREATE_DEVICE,
            RC_E_CONNECT,

            RC_E_RECV = -1,
            RC_S_END  = -2,
        };

        static int GetDevices(std::vector<std::string>* ids);

        IOSBridge();
        ~IOSBridge();

        int Connect(uint16_t port, const char* udid);
        void Disconnect();
        void Close();

        bool Subscribe();
        bool Unsubscribe();

        int Recv(unsigned char* buf, size_t len);

    private:
        struct PTFrame {
            unsigned long version;
            unsigned long type;
            unsigned long tag;
            unsigned long payload_size;
            unsigned long identifier;
            unsigned char* data;
        };

        bool Auth();
        void OnAuthWork();

        static idevice_error_t pt_speed_test(idevice_connection_t conn);
        static idevice_error_t pt_send_frame(idevice_connection_t conn, const PTFrame& frame);
        static idevice_error_t pt_recv_frame(
            idevice_connection_t conn, std::atomic_bool& ctrl, PTFrame* frame);

        idevice_t imobile_ = 0;
        idevice_connection_t imobile_conn_ = 0;
        std::thread worker_;
        std::atomic_bool auth_result_;
        std::atomic_bool is_running_;

        long ptfd_pos_ = -1;
        PTFrame pt_frame_;
    };

}

#endif  // OBS_RECEIVER_IOS_BRIDGE_H_