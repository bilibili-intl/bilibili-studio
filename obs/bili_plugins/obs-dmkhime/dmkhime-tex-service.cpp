#include "dmkhime-tex-service.h"

#include <cassert>


namespace {

    std::unique_ptr<bililive::DmkhimeTexService> g_dmkhime_tex_service;

    class DmkhimeServiceFactory : public bililive::IDmkhimeServiceFactory {
    public:
        bililive::IDmkhimeTexService* Get() override {
            return bililive::DmkhimeTexService::Get();
        }
    } g_dmkhime_service_factory;

}

namespace bililive {

    DmkhimeTexService::DmkhimeTexService()
        : queue_cv_pred_(false),
          thread_running_(ATOMIC_VAR_INIT(false)),
          active_ref_(0) {
    }

    DmkhimeTexService::~DmkhimeTexService() {
        if (thread_running_) {
            thread_running_ = false;

            {
                std::lock_guard<std::mutex> lg(queue_cv_mutex_);
                queue_cv_pred_ = true;
            }
            queue_cv_.notify_one();

            working_thread_.join();
        }
    }

    bool DmkhimeTexService::Launch(const CallbackPtr& callback) {
        if (thread_running_) {
            return true;
        }

        callback_ = callback;
        thread_running_ = true;
        working_thread_ = std::thread(&DmkhimeTexService::OnWorkingThread, this);
        return true;
    }

    bool DmkhimeTexService::HasDispatcher() {
        std::lock_guard<std::mutex> lg(dispatcher_mutex_);
        return !dispatchers_.empty();
    }

    bool DmkhimeTexService::HasActivatedSource() {
        std::lock_guard<std::mutex> lg(active_ref_mutex_);
        return active_ref_ > 0;
    }

    void DmkhimeTexService::OnWorkingThread() {
        while (thread_running_) {
            bool is_empty;
            VideoFramePtr frame;
            {
                std::unique_lock<std::mutex> lk(queue_cv_mutex_);
                is_empty = frame_queue_.empty();
                if (!is_empty) {
                    frame = frame_queue_.front();
                    frame_queue_.pop();
                }
            }
            if (!is_empty) {
                std::lock_guard<std::mutex> lg(dispatcher_mutex_);
                for (auto& dispatcher : dispatchers_) {
                    dispatcher->OnTexture(frame.get());
                }
            } else {
                std::unique_lock<std::mutex> lk(queue_cv_mutex_);
                queue_cv_.wait(lk, [this] { return queue_cv_pred_; });
                queue_cv_pred_ = false;
            }
        }
    }

    void DmkhimeTexService::Dispatch(const VideoFramePtr& frame) {
        {
            std::lock_guard<std::mutex> lg(queue_cv_mutex_);
            frame_queue_.push(frame);
            queue_cv_pred_ = true;
        }
        queue_cv_.notify_one();
    }

    void DmkhimeTexService::AddDispatcher(IDmkhimeTextureDispatcher* dispatcher) {
        std::lock_guard<std::mutex> lg(dispatcher_mutex_);
        dispatchers_.push_back(dispatcher);
    }

    void DmkhimeTexService::RemoveDispatcher(IDmkhimeTextureDispatcher* dispatcher) {
        std::lock_guard<std::mutex> dispatcher_lg(dispatcher_mutex_);
        for (auto it = dispatchers_.begin(); it != dispatchers_.end();) {
            if (*it == dispatcher) {
                it = dispatchers_.erase(it);
            } else {
                ++it;
            }
        }

        if (dispatchers_.empty()) {
            std::lock_guard<std::mutex> queue_lg(queue_cv_mutex_);
            while (!frame_queue_.empty()) {
                frame_queue_.pop();
            }
            queue_cv_pred_ = false;
        }
    }

    void DmkhimeTexService::AddActiveRef() {
        std::lock_guard<std::mutex> lg(active_ref_mutex_);
        ++active_ref_;

        auto ptr = callback_.lock();
        if (ptr) {
            ptr->OnRefreshTexture(active_ref_ == 1);
        }
    }

    void DmkhimeTexService::SubActiveRef() {
        std::lock_guard<std::mutex> lg(active_ref_mutex_);
        if (active_ref_ == 0) {
            return;
        }
        --active_ref_;
        if (active_ref_ == 0) {
            auto ptr = callback_.lock();
            if (ptr) {
                ptr->OnAutism();
            }
        }
    }

    // static
    void DmkhimeTexService::Create() {
        assert(!g_dmkhime_tex_service);
        g_dmkhime_tex_service = std::make_unique<DmkhimeTexService>();
    }

    // static
    void DmkhimeTexService::Destroy() {
        assert(!!g_dmkhime_tex_service);
        if (g_dmkhime_tex_service) {
            g_dmkhime_tex_service.reset();
        }
    }

    // static
    DmkhimeTexService* DmkhimeTexService::Get() {
        assert(!!g_dmkhime_tex_service);
        return g_dmkhime_tex_service.get();
    }

}

extern "C"
{
    bililive::IDmkhimeServiceFactory* GetDmkhimeServiceFactory() {
        return &g_dmkhime_service_factory;
    }
}