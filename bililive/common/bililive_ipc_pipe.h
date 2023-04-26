#ifndef BILILIVE_IPC_PIPE_H_
#define BILILIVE_IPC_PIPE_H_

#include <memory>
#include <vector>
#include <string>
#include <Windows.h>

#include "base/basictypes.h"
#include "base/win/scoped_handle.h"

class BiliPipe
{
public:
    explicit BiliPipe(HANDLE data_pipe);

    ~BiliPipe();

    int ReadFixed(char* buf, int len);
    int ReadSome(char* buf, int len);
    int ReadUntil(char* buf, int len, char deli);

    int WriteFixed(const char* buf, int len);
    int WriteSome(const char* buf, int len);

    void Close();

private:
    bool OnBufUnderflow();

    DISALLOW_COPY_AND_ASSIGN(BiliPipe);

private:
    base::win::ScopedHandle data_pipe_handle_;
    std::vector<char> buffer_;
};

using BiliPipePtr = std::unique_ptr<BiliPipe>;

class BiliProcess
{
public:
    // utf8 commandline
    BiliProcess(const wchar_t* command_line, bool redir_stdin, bool redir_stdout, bool redir_stderr);

    ~BiliProcess();

    void Detach();
    bool Launch();

    BiliPipe* Stderr();
    BiliPipe* Stdin();
    BiliPipe* Stdout();

    int Join(int timeout_ms); //return -1 or exitcode
    bool Terminate(int exit_code);

    operator void* (); //for if (*this)

private:
    DISALLOW_COPY_AND_ASSIGN(BiliProcess);

private:
    std::vector<wchar_t> cmd_line_;
    bool redir_stderr_;
    bool redir_stdin_;
    bool redir_stdout_;

    base::win::ScopedHandle mpv_proc_handle_;

    BiliPipePtr pipe_err_;
    BiliPipePtr pipe_in_;
    BiliPipePtr pipe_out_;
};

using BiliProcessPtr = std::unique_ptr<BiliProcess>;

#endif