#include "obs_livehime_thread_manager.h"

#include <string>
#include "base/atomicops.h"
#include "base/bind.h"
#include "base/compiler_specific.h"
#include "base/lazy_instance.h"
#include "base/message_loop/message_loop.h"
#include "base/message_loop/message_loop_proxy.h"
#include "base/threading/sequenced_worker_pool.h"
#include "base/threading/thread_restrictions.h"

namespace
{
    class OBSPluginMainThread : public base::Thread
    {
    public:
        OBSPluginMainThread(const char* name)
            : base::Thread(name)
        {
        }

    protected:
        // Called just prior to starting the message loop
        void Init() override
        {

        }
    };
}

OBSPluginThreadManager::OBSPluginThreadManager(OBSPluginThreadDelegate* delegate)
    : main_thread_("livehime_obs_plugin_main_thread")
    , ipc_thread_("livehime_obs_plugin_ipc_thread")
    , delegate_(delegate)
{
}

OBSPluginThreadManager::~OBSPluginThreadManager()
{
    Uninitialize();
}

void OBSPluginThreadManager::Initialize()
{
    DCHECK(!main_thread_.IsRunning());
    if (!main_thread_.IsRunning())
    {
        base::Thread::Options option;
        option.message_loop_type = base::MessageLoop::Type::TYPE_UI;
        main_thread_.StartWithOptions(option);

        PostMainThreadStarted();
    }

    DCHECK(!ipc_thread_.IsRunning());
    if (!ipc_thread_.IsRunning())
    {
        base::Thread::Options option;
        option.message_loop_type = base::MessageLoop::Type::TYPE_IO;
        ipc_thread_.StartWithOptions(option);

        PostIPCThreadStarted();
    }
}

void OBSPluginThreadManager::Uninitialize()
{
    if (ipc_thread_.IsRunning())
    {
        ipc_thread_.Stop();
    }
    if (main_thread_.IsRunning())
    {
        main_message_loop_proxy()->PostTask(FROM_HERE,
            base::Bind(&OBSPluginThreadManager::ShutdownWorkerPool, base::Unretained(this)));

        main_thread_.Stop();
    }
}

void OBSPluginThreadManager::PostMainThreadStarted()
{
    if (delegate_)
    {
        delegate_->PostMainThreadStarted();
    }
    main_message_loop_proxy()->PostTask(FROM_HERE,
        base::Bind(&OBSPluginThreadManager::CreateWorkerPool, base::Unretained(this)));
}

void OBSPluginThreadManager::PostIPCThreadStarted()
{
    if (delegate_)
    {
        delegate_->PostIPCThreadStarted();
    }
}

void OBSPluginThreadManager::CreateWorkerPool()
{
    if (!worker_pool_)
    {
        worker_pool_ = new base::SequencedWorkerPool(1, "obs_livehime_plugin_worker_pool");

        if (delegate_)
        {
            delegate_->PostWorkerPoolStarted();
        }
    }
}

void OBSPluginThreadManager::ShutdownWorkerPool()
{
    if (worker_pool_)
    {
        static const int kMaxNewShutdownBlockingTasks = 1000;
        worker_pool_->Shutdown(kMaxNewShutdownBlockingTasks);
        worker_pool_ = nullptr;
    }
}

base::SequencedWorkerPool* OBSPluginThreadManager::GetWorkerPool()
{
    return worker_pool_.get();
}

scoped_refptr<base::MessageLoopProxy> OBSPluginThreadManager::main_message_loop_proxy() const
{
    return main_thread_.message_loop_proxy();
}

scoped_refptr<base::MessageLoopProxy> OBSPluginThreadManager::ipc_message_loop_proxy() const
{
    return ipc_thread_.message_loop_proxy();
}
