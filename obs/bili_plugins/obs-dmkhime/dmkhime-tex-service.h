#ifndef OBS_DMKHIME_DMKHIME_TEX_SERVICE_H_
#define OBS_DMKHIME_DMKHIME_TEX_SERVICE_H_

#include <atomic>
#include <condition_variable>
#include <queue>
#include <thread>

#include "dmkhime-service.h"


namespace bililive {

    class DmkhimeTexService : public IDmkhimeTexService {
    public:
        DmkhimeTexService();
        ~DmkhimeTexService();

        bool Launch(const CallbackPtr& callback) override;
        bool HasDispatcher() override;
        bool HasActivatedSource() override;
        void Dispatch(const VideoFramePtr& frame) override;

        void AddDispatcher(IDmkhimeTextureDispatcher* dispatcher);
        void RemoveDispatcher(IDmkhimeTextureDispatcher* dispatcher);

        void AddActiveRef();
        void SubActiveRef();

        static void Create();
        static void Destroy();
        static DmkhimeTexService* Get();

    private:
        void OnWorkingThread();

        bool queue_cv_pred_;
        std::mutex queue_cv_mutex_;
        std::condition_variable queue_cv_;

        std::atomic_bool thread_running_;
        std::thread working_thread_;
        std::mutex dispatcher_mutex_;

        int active_ref_;
        std::mutex active_ref_mutex_;

        std::queue<VideoFramePtr> frame_queue_;
        std::vector<IDmkhimeTextureDispatcher*> dispatchers_;
        CallbackWPtr callback_;
    };

}

#endif  // OBS_DMKHIME_DMKHIME_TEX_SERVICE_H_