#ifndef OBS_SCENE_UTIL_H_
#define OBS_SCENE_UTIL_H_

#include "base/memory/weak_ptr.h"
#include "base/timer/timer.h"

class LivehimeSceneItemFitToScreenPresenter
{
public:
    LivehimeSceneItemFitToScreenPresenter();
    ~LivehimeSceneItemFitToScreenPresenter();
    bool Start();
    void Shutdown();

private:
    void OnTimer();

private:
    base::WeakPtrFactory<LivehimeSceneItemFitToScreenPresenter> weakptr_factory_;
    base::RepeatingTimer<LivehimeSceneItemFitToScreenPresenter> timer_;

    DISALLOW_COPY_AND_ASSIGN(LivehimeSceneItemFitToScreenPresenter);
};

#endif
