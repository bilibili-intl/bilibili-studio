#ifndef TTS_SERVICE_H_
#define TTS_SERVICE_H_

#include <Windows.h>

#include <string>
#include <memory>
#include <atomic>
#include <functional>

namespace bililive {

class SpVoiceHelper;

struct SpVoiceParam
{
    std::pair<bool, int> volume;
    std::pair<bool, int> speed;
    std::pair<bool, std::wstring> voice;
};

// must use with shared_ptr
class TTSService : public std::enable_shared_from_this < TTSService >
{
public:
    class QueueLocker
    {
    public:
        QueueLocker(TTSService* p);
        ~QueueLocker();
    private:
        TTSService* serv_;
        QueueLocker(const QueueLocker&) = delete;
        QueueLocker& operator =(const QueueLocker&) = delete;
    };

    friend class QueueLocker;

public:
    bool Start();
    bool Stop();

    void UpdateParam(const SpVoiceParam& param);

    TTSService();
    virtual ~TTSService();

protected:
    virtual std::wstring GetNextToSpeak() = 0; //will be called within lock
    virtual bool IsTodoListEmpty() = 0; //will be called within lock

    virtual SpVoiceHelper* CreateSpVoiceObject(); //override this function to set custom parameter to SpVoiceHelper

    void NotifyNewTodoListChanged();
    void DoUpdateParam(); //should called in lock

private:
    typedef std::function<void()> ThreadFuncT;
    static unsigned int __stdcall ThreadProcEntry(void* param);
    void ThreadProc();

    void RunOnce();

    TTSService(const TTSService&) = delete;
    TTSService& operator = (const TTSService&) = delete;

private:
    CRITICAL_SECTION cs_;
    CONDITION_VARIABLE condVar_;
    SpVoiceHelper* spVoice_;

    std::atomic_bool threadLaunched_;
    std::atomic_bool exitFlag_;

    std::unique_ptr<SpVoiceParam> newParam_;
};

};

#endif
