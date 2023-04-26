#include "game_launcher.h"

#include <future>


namespace {

    struct EnumContext {
        DWORD target_proc_id;
        HWND hWnd;
    };

    BOOL CALLBACK EnumWindowProc(HWND hWnd, LPARAM lParam) {
        auto context = reinterpret_cast<EnumContext*>(lParam);

        DWORD proc_id = 0;
        ::GetWindowThreadProcessId(hWnd, &proc_id);

        if (proc_id == context->target_proc_id) {
            context->hWnd = hWnd;
            ::SetLastError(ERROR_SUCCESS);
            return FALSE;
        }

        return TRUE;
    }

}

namespace livehime {
namespace game {

    GameLauncher::GameLauncher(GameLauncherListener* l)
        : is_running_(false),
          listener_(l)
    {}

    GameLauncher::~GameLauncher() {
        Detach();
    }

    bool GameLauncher::ListernProcess(DWORD pid) {
        proc_handle_ = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
        if (!proc_handle_) {
            return false;
        }

        if (working_thread_.joinable()) {
            working_thread_.join();
        }

        if (event_) {
            ::CloseHandle(event_);
            event_ = nullptr;
        }

        is_running_ = true;
        event_ = ::CreateEventW(nullptr, FALSE, FALSE, nullptr);
        working_thread_ = std::thread(&GameLauncher::OnWorker, this, true,(int)pid);

        return true;
    }

    void GameLauncher::AppClose() {
        app_stop_ = true;
    }

    void GameLauncher::OnWorker(bool notify_exit, int pid) {
        if (!proc_handle_) {
            return;
        }

        HANDLE handle[]{ proc_handle_, event_ };
        if (::WaitForMultipleObjects(2, handle, FALSE, INFINITE) == WAIT_FAILED) {
            Log("[game_LAUNCHER] Error: Failed to wait game process: " + std::to_string(::GetLastError()));
            return;
        }

        DWORD exit_code;
        if (::GetExitCodeProcess(proc_handle_, &exit_code) == 0) {
        }

        if (app_stop_) {
            return;
        }

        is_running_ = false;
        if (notify_exit && listener_) {
            listener_->OnGameExited(pid);
        }

        //if (event_) {
        //    ::CloseHandle(event_);
        //    event_ = nullptr;
        //}

        if (proc_handle_) {
            ::CloseHandle(proc_handle_);
            proc_handle_ = nullptr;
        }
    }

    void GameLauncher::Log(const std::string& msg) {
        if (listener_) {
            listener_->OnGameLauncherLog(msg);
        }
    }

    void GameLauncher::Detach() {
        if (working_thread_.joinable()) {
            if (is_running_) {
                working_thread_.detach();
            }
            else {
                working_thread_.join();
            }
        }

        if (event_) {
            ::CloseHandle(event_);
            event_ = nullptr;
        }

        if (proc_handle_) {
            ::CloseHandle(proc_handle_);
            proc_handle_ = nullptr;
        }

        is_running_ = false;
    }



















    void GameLauncher::Close() {
        bool is_sent_close = false;
        if (proc_handle_) {
            /*HWND hWnd = FindUnityWindowByProcessHandle(proc_handle_);
            if (hWnd) {
                if (CloseUnityWindow(hWnd)) {
                    is_sent_close = true;
                }
            } else {
                Log("[UNITY_LAUNCHER] Error: Cannot find unity window!");
            }*/
        }

        if (working_thread_.joinable()) {
            if (is_sent_close) {
                working_thread_.join();
            } else {
                working_thread_.detach();
            }
        }

        if (proc_handle_) {
            ::CloseHandle(proc_handle_);
            proc_handle_ = nullptr;
        }

        is_running_ = false;
    }

    bool GameLauncher::Terminate() {
        if (!is_running_) {
            return true;
        }

        if (proc_handle_) {
            bool is_terminated = !!::TerminateProcess(proc_handle_, 0);
            if (!is_terminated) {
                Log("[UNITY_LAUNCHER] Error: Failed to terminate unity process: "
                    + std::to_string(::GetLastError()));
            }

            if (working_thread_.joinable()) {
                if (is_terminated) {
                    working_thread_.join();
                } else {
                    working_thread_.detach();
                }
            }


            if (proc_handle_) {
                ::CloseHandle(proc_handle_);
                proc_handle_ = nullptr;
            }

            is_running_ = false;
            return true;
        }
        return false;
    }

    bool GameLauncher::IsRunning() const {
        return is_running_;
    }

}
}