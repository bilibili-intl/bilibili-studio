#ifndef OBS_TEXTURE_RENDERER_TEXTURE_SERVICE_IMPL_H_
#define OBS_TEXTURE_RENDERER_TEXTURE_SERVICE_IMPL_H_

#include <atomic>
#include <condition_variable>
#include <queue>
#include <thread>

#include "texture-service.h"


namespace livehime {

    class TextureServiceImpl : public ITextureService {
    public:
        TextureServiceImpl();
        ~TextureServiceImpl();

        bool Launch(const CallbackPtr& callback) override;
        bool HasDispatcher() override;
        bool HasActivatedSource() override;
        void Dispatch(const VideoFramePtr& frame, TextureType type, int id) override;

        void AddDispatcher(ITextureDispatcher* dispatcher);
        void RemoveDispatcher(ITextureDispatcher* dispatcher);

        void AddActiveRef();
        void SubActiveRef();

        static std::shared_ptr<TextureServiceImpl> Fetch();

    private:
        struct QueuedData {
            int id;
            TextureType type;
            VideoFramePtr frame;
        };

        void OnWorkingThread();

        bool queue_cv_pred_;
        std::mutex queue_cv_mutex_;
        std::condition_variable queue_cv_;

        std::atomic_bool thread_running_;
        std::thread working_thread_;
        std::mutex dispatcher_mutex_;

        int active_ref_;
        std::mutex active_ref_mutex_;

        std::queue<QueuedData> frame_queue_;
        std::vector<ITextureDispatcher*> dispatcher_;
        CallbackWPtr callback_;
    };

    extern std::mutex g_dmkhime_tex_service_mutex;
    extern std::shared_ptr<TextureServiceImpl> g_dmkhime_tex_service;

}

#endif  // OBS_TEXTURE_RENDERER_TEXTURE_SERVICE_IMPL_H_