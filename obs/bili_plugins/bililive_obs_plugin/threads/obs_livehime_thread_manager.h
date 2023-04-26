#ifndef OBS_PLUGINS_OBS_THREADS_MANAGER_H
#define OBS_PLUGINS_OBS_THREADS_MANAGER_H


#include "base/threading/sequenced_worker_pool.h"
#include "base/threading/thread.h"

class LivehimeOBSPluginImpl;

class OBSPluginThreadDelegate
{
public:
    virtual void PostMainThreadStarted() = 0;
    virtual void PostIPCThreadStarted() = 0;
    virtual void PostWorkerPoolStarted() = 0;

protected:
    virtual ~OBSPluginThreadDelegate() = default;
};

class OBSPluginThreadManager
{
public:
    explicit OBSPluginThreadManager(OBSPluginThreadDelegate* delegate);
    virtual ~OBSPluginThreadManager();

    base::SequencedWorkerPool* GetWorkerPool();

    scoped_refptr<base::MessageLoopProxy> main_message_loop_proxy() const;
    scoped_refptr<base::MessageLoopProxy> ipc_message_loop_proxy() const;

private:
    void Initialize();
    void Uninitialize();

    void PostMainThreadStarted();
    void PostIPCThreadStarted();

    void CreateWorkerPool();
    void ShutdownWorkerPool();

private:
    friend class LivehimeOBSPluginImpl;

    OBSPluginThreadDelegate* delegate_ = nullptr;
    base::Thread main_thread_;
    base::Thread ipc_thread_;
    scoped_refptr<base::SequencedWorkerPool> worker_pool_;
};

#endif