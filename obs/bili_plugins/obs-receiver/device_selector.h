#ifndef OBS_RECEIVER_DEVICE_SELECTOR_H_
#define OBS_RECEIVER_DEVICE_SELECTOR_H_

#include <mutex>
#include <string>
#include <vector>


namespace ios {

    class DeviceSelector {
    public:
        DeviceSelector();

        std::string Select(const std::vector<std::string>& devs);
        void AddToLRL(const std::string& id);
        void RemoveFromGRL(const std::string& id);
        void ClearLRL();

    private:
        void ArrangeGRL(const std::vector<std::string>& devs);

        bool IsInLRL(const std::string& id);
        bool IsInGRL(const std::string& id);

        std::vector<std::string> l_retry_list_;
        static std::vector<std::string> g_retry_list_;
        static std::mutex retry_sync_;
    };

}

#endif  // OBS_RECEIVER_DEVICE_SELECTOR_H_