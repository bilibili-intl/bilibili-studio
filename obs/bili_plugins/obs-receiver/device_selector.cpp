#include "device_selector.h"


namespace ios {

    // static
    std::vector<std::string> DeviceSelector::g_retry_list_;
    // static
    std::mutex DeviceSelector::retry_sync_;

    DeviceSelector::DeviceSelector() {}

    std::string DeviceSelector::Select(const std::vector<std::string>& devs) {
        std::lock_guard<std::mutex> lg(retry_sync_);

        // ɾ��ȫ���б���Ĺ�����
        ArrangeGRL(devs);

        /**
         * �ȿ�������Ͷ��Դ����ʹ�õ��豸�б�
         * Ҫ�ܿ���Щ�豸��ǿ������ȥ���ܻ��֮ǰ�Ķ�����
         */
        bool usable = false;
        std::string first_usable;
        std::string candidate;
        for (const auto& dev : devs) {
            if (IsInGRL(dev)) {
                continue;
            }

            usable = true;
            if (first_usable.empty()) {
                first_usable = dev;
            }

            if (IsInLRL(dev)) {
                continue;
            }
            candidate = dev;
            break;
        }

        if (!usable) {
            // û�п����豸
            return {};
        }

        if (candidate.empty()) {
            // �Թ�һ���ˣ����¿�ʼ
            l_retry_list_.clear();
            candidate = std::move(first_usable);
        }

        g_retry_list_.push_back(candidate);
        return candidate;
    }

    void DeviceSelector::AddToLRL(const std::string& id) {
        if (!IsInLRL(id)) {
            l_retry_list_.push_back(id);
        }
    }

    void DeviceSelector::RemoveFromGRL(const std::string& id) {
        std::lock_guard<std::mutex> lg(retry_sync_);
        for (auto it = g_retry_list_.begin(); it != g_retry_list_.end(); ++it) {
            if (*it == id) {
                g_retry_list_.erase(it);
                return;
            }
        }
    }

    void DeviceSelector::ClearLRL() {
        l_retry_list_.clear();
    }

    void DeviceSelector::ArrangeGRL(const std::vector<std::string>& devs) {
        for (auto it = g_retry_list_.begin(); it != g_retry_list_.end(); ) {
            auto it2 = std::find(devs.cbegin(), devs.cend(), *it);
            if (it2 == devs.cend()) {
                it = g_retry_list_.erase(it);
            } else {
                ++it;
            }
        }
    }

    bool DeviceSelector::IsInLRL(const std::string& id) {
        auto it = std::find(l_retry_list_.cbegin(), l_retry_list_.cend(), id);
        if (it != l_retry_list_.cend()) {
            return true;
        }
        return false;
    }

    bool DeviceSelector::IsInGRL(const std::string& id) {
        auto it = std::find(g_retry_list_.cbegin(), g_retry_list_.cend(), id);
        if (it != g_retry_list_.cend()) {
            return true;
        }
        return false;
    }

}