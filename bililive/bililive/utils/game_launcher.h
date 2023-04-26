#ifndef BILIBILI_UTIL_GAME_LAUNCHER_H_
#define BILIBILI_UTIL_GAME_LAUNCHER_H_

#include <atomic>
#include <string>
#include <thread>

#include <Windows.h>


namespace livehime {
namespace game {

    class GameLauncherListener {
    public:
        virtual ~GameLauncherListener() = default;

        virtual void OnGameExited(int pid) = 0;
        virtual void OnGameLauncherLog(const std::string& msg) = 0;
    };

    class GameLauncher {
    public:
        explicit GameLauncher(GameLauncherListener* l);
        ~GameLauncher();

        bool ListernProcess(DWORD pid);
        void Detach();
        void Close();
        bool Terminate();
        void AppClose();
        bool IsRunning() const;

    private:
        void OnWorker(bool notify_exit,int pid);
        void Log(const std::string& msg);

        std::wstring cmd_;
        bool app_stop_ = false;
        std::atomic_bool is_running_;
        std::thread working_thread_;
        HANDLE event_ = nullptr;
        HANDLE proc_handle_ = nullptr;
        GameLauncherListener* listener_;
    };

}
}

#endif  // BILIBILI_UTIL_GAME_LAUNCHER_H_