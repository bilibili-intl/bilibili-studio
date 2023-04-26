#include "texture_service_impl.h"


namespace {

    class TextureServiceFactory : public livehime::ITextureServiceFactory {
    public:
        std::shared_ptr<livehime::ITextureService> Fetch() override {
            return livehime::TextureServiceImpl::Fetch();
        }
    } g_texture_service_factory;

}

namespace livehime {

    std::mutex g_dmkhime_tex_service_mutex;
    std::shared_ptr<TextureServiceImpl> g_dmkhime_tex_service;

    TextureServiceImpl::TextureServiceImpl()
        : queue_cv_pred_(false),
          thread_running_(ATOMIC_VAR_INIT(false)),
          active_ref_(0) {}

    TextureServiceImpl::~TextureServiceImpl() {
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

    bool TextureServiceImpl::Launch(const CallbackPtr& callback) {
        if (thread_running_) {
            return true;
        }

        callback_ = callback;
        thread_running_ = true;
        working_thread_ = std::thread(&TextureServiceImpl::OnWorkingThread, this);
        return true;
    }

    bool TextureServiceImpl::HasDispatcher() {
        std::lock_guard<std::mutex> lg(dispatcher_mutex_);
        return !dispatcher_.empty();
    }

    bool TextureServiceImpl::HasActivatedSource() {
        std::lock_guard<std::mutex> lg(active_ref_mutex_);
        return active_ref_ > 0;
    }

    void TextureServiceImpl::OnWorkingThread() {
        while (thread_running_) {
            bool is_empty;
            QueuedData data;
            {
                std::unique_lock<std::mutex> lk(queue_cv_mutex_);
                is_empty = frame_queue_.empty();
                if (!is_empty) {
                    data = frame_queue_.front();
                    frame_queue_.pop();
                }
            }
            if (!is_empty) {
                std::lock_guard<std::mutex> lg(dispatcher_mutex_);
                for (auto& dispatcher : dispatcher_) {
                    dispatcher->OnTexture(data.frame.get());
                }
            } else {
                std::unique_lock<std::mutex> lk(queue_cv_mutex_);
                queue_cv_.wait(lk, [this] { return queue_cv_pred_; });
                queue_cv_pred_ = false;
            }
        }
    }

    void TextureServiceImpl::Dispatch(const VideoFramePtr& frame, TextureType type, int id) {
        {
            std::lock_guard<std::mutex> lg(queue_cv_mutex_);
            frame_queue_.push({ id, type, frame });
            queue_cv_pred_ = true;
        }
        queue_cv_.notify_one();
    }

    void TextureServiceImpl::AddDispatcher(ITextureDispatcher* dispatcher) {
        std::lock_guard<std::mutex> lg(dispatcher_mutex_);
        dispatcher_.push_back(dispatcher);
    }

    void TextureServiceImpl::RemoveDispatcher(ITextureDispatcher* dispatcher) {
        std::lock_guard<std::mutex> dispatcher_lg(dispatcher_mutex_);
        for (auto it = dispatcher_.begin(); it != dispatcher_.end();) {
            if (*it == dispatcher) {
                it = dispatcher_.erase(it);
            } else {
                ++it;
            }
        }

        if (dispatcher_.empty()) {
            std::lock_guard<std::mutex> queue_lg(queue_cv_mutex_);
            while (!frame_queue_.empty()) {
                frame_queue_.pop();
            }
            queue_cv_pred_ = false;
        }
    }

    void TextureServiceImpl::AddActiveRef() {
        std::lock_guard<std::mutex> lg(active_ref_mutex_);
        ++active_ref_;

        auto ptr = callback_.lock();
        if (ptr) {
            ptr->OnRefreshTexture(active_ref_ == 1);
        }
    }

    void TextureServiceImpl::SubActiveRef() {
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
    std::shared_ptr<TextureServiceImpl> TextureServiceImpl::Fetch() {
        std::lock_guard<std::mutex> lg(g_dmkhime_tex_service_mutex);
        if (!g_dmkhime_tex_service) {
            g_dmkhime_tex_service = std::make_shared<TextureServiceImpl>();
        }
        return g_dmkhime_tex_service;
    }

}

extern "C"
{
    livehime::ITextureServiceFactory* GetTextureServiceFactory() {
        return &g_texture_service_factory;
    }
}