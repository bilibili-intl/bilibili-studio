#pragma once
#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"

#include "threads/obs_livehime_thread_manager.h"

class Profile;
class OBSPluginLivehimeIPCService;

class LivehimeOBSPluginImpl : public OBSPluginThreadDelegate
{
public:
    LivehimeOBSPluginImpl();
    ~LivehimeOBSPluginImpl();

    OBSPluginThreadManager* thread_manager() const
    {
        return thread_manager_.get();
    }

    Profile* profile() const
    {
        return profile_.get();
    }

    OBSPluginLivehimeIPCService* ipc_service() const
    {
        return ipc_service_.get();
    }

    bool IsLivehimeConnected() const;

protected:
    // OBSPluginThreadDelegate
    void PostMainThreadStarted() override;
    void PostIPCThreadStarted() override;
    void PostWorkerPoolStarted() override;

private:
    void Initialize();
    void Uninitialize();

private:
    base::FilePath user_data_dir_;
    std::unique_ptr<OBSPluginThreadManager> thread_manager_;
    scoped_refptr<Profile> profile_;
    scoped_refptr<OBSPluginLivehimeIPCService> ipc_service_;
};

LivehimeOBSPluginImpl* LivehimeOBSPlugin();