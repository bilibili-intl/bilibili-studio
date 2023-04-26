#include "tts_service.h"
#include "tts_helper.h"

#include "base/logging.h"

namespace bililive
{

TTSService::TTSService()
{
    threadLaunched_.store(false);
    exitFlag_.store(false);
    spVoice_ = nullptr;

    InitializeConditionVariable(&condVar_);
    InitializeCriticalSectionAndSpinCount(&cs_, 4000);
}

TTSService::~TTSService()
{
    DeleteCriticalSection(&cs_);

    DCHECK(spVoice_ == nullptr) << "TTS service destroyed without spVoice_ deleted!";
}

SpVoiceHelper* TTSService::CreateSpVoiceObject()
{
    return SpVoiceHelper::Create();
}

unsigned int TTSService::ThreadProcEntry(void* param)
{
    std::unique_ptr<ThreadFuncT> threadFunc((ThreadFuncT*)param);
    (*threadFunc)();
    return 0;
}

void TTSService::ThreadProc()
{
    LOG(INFO) << "[TTS] TTS Service thread started.";
    CoInitializeEx(0, COINIT_APARTMENTTHREADED);

    spVoice_ = CreateSpVoiceObject();
    if (spVoice_)
    {
        while (exitFlag_.load() == false)
        {
            RunOnce();
        }

        delete spVoice_;
        spVoice_ = nullptr;
    }

    CoUninitialize();

    LOG(INFO) << "[TTS] TTS Service thread exit.";
}

bool TTSService::Start()
{
    if (threadLaunched_.load() == true)
        return false;

    std::unique_ptr<ThreadFuncT> threadFunc(new ThreadFuncT(std::bind(&TTSService::ThreadProc, shared_from_this())));
    uintptr_t hThread = _beginthreadex(0, 0, &TTSService::ThreadProcEntry, threadFunc.get(), 0, 0);
    if (hThread > 0)
    {
        threadLaunched_.store(true);
        CloseHandle((HANDLE)hThread);
        threadFunc.release();
        LOG(INFO) << "[TTS] TTS Service started.";
        return true;
    }
    else
    {
        return false;
    }
}

bool TTSService::Stop()
{
    if (threadLaunched_.load() == false)
        return false;

    EnterCriticalSection(&cs_);
    exitFlag_.store(true);
    threadLaunched_.store(false);
    LeaveCriticalSection(&cs_);

    LOG(INFO) << "[TTS] TTS Service stopped.";
    return true;
}

void TTSService::UpdateParam(const SpVoiceParam& param)
{
    QueueLocker locker(this);
    if (newParam_)
    {
        if (param.speed.first)
        {
            newParam_->speed.first = true;
            newParam_->speed.second = param.speed.second;
        }
        if (param.voice.first)
        {
            newParam_->voice.first = true;
            newParam_->voice.second = param.voice.second;
        }
        if (param.volume.first)
        {
            newParam_->volume.first = true;
            newParam_->volume.second = param.volume.second;
        }
    }
    else
        newParam_.reset(new SpVoiceParam(param));
    NotifyNewTodoListChanged();
}

void TTSService::DoUpdateParam()
{
    if (newParam_)
    {
        LOG(INFO) << "[TTS] new params to update found.";

        if (newParam_->volume.first)
        {
            LOG(INFO) << "[TTS] volume set to " << newParam_->volume.second;
            spVoice_->SetVolume(newParam_->volume.second);
        }
        if (newParam_->speed.first)
        {
            LOG(INFO) << "[TTS] speed set to " << newParam_->speed.second;
            spVoice_->SetSpeed(newParam_->speed.second);
        }
        if (newParam_->voice.first)
        {
            auto voiceList = SpVoiceHelper::GetVoiceList();
            auto selectedVoice = voiceList.find(newParam_->voice.second);
            if (selectedVoice != voiceList.end())
            {
                LOG(INFO) << "[TTS] voice set to " << newParam_->voice.second;
                spVoice_->SetVoice(selectedVoice->second);
            }
            else
            {
                DCHECK(0) << "[TTS] voice failed to set to non-exist " << newParam_->voice.second;
            }
        }
        newParam_.reset();
    }
}

void TTSService::RunOnce()
{
    std::wstring toSpeak;

    {
        QueueLocker locker(this);

        while (IsTodoListEmpty())
        {
            SleepConditionVariableCS(&condVar_, &cs_, INFINITE);
        }

        do{
            if (exitFlag_.load() == true)
                break;

            DoUpdateParam();

            toSpeak = GetNextToSpeak();

            LOG(INFO) << "[TTS] Speaking: " << toSpeak;
        } while (false);
    }

    if (!toSpeak.empty())
        spVoice_->Speak(toSpeak);
}

void TTSService::NotifyNewTodoListChanged()
{
    QueueLocker locker(this);
    WakeConditionVariable(&condVar_);
}

TTSService::QueueLocker::QueueLocker(TTSService* p)
{
    serv_ = p;
    EnterCriticalSection(&serv_->cs_);
}

TTSService::QueueLocker::~QueueLocker()
{
    LeaveCriticalSection(&serv_->cs_);
}

};