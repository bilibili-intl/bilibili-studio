#include "bililive_ipc_pipe.h"

#include "bililive/common/bililive_context.h"

#include <algorithm>
#include <assert.h>

#include <atlbase.h>
#include <atlconv.h>
#include <atlstr.h>

namespace
{
    const size_t kBufSize = 4096;
}

BiliPipe::BiliPipe(HANDLE data_pipe)
    : data_pipe_handle_(data_pipe)
{}

BiliPipe::~BiliPipe()
{}

void BiliPipe::Close()
{
    data_pipe_handle_.Close();
}

bool BiliPipe::OnBufUnderflow()
{
    if (!buffer_.empty())
    {
        DCHECK("BiliPipe: called OnBufUnderflow when buffer is not null");
        return true;
    }

    DWORD data_readed = 0;
    buffer_.resize(kBufSize);

    for (;;)
    {
        if (ReadFile(data_pipe_handle_.Get(), buffer_.data(), kBufSize, &data_readed, 0) == FALSE)
            return false;

        if (data_readed == 0)
            continue;

        break;
    }

    buffer_.resize(data_readed);
    return true;
}

int BiliPipe::ReadFixed(char* buf, int len)
{
    int read_count = 0;
    while (len > 0)
    {
        int r = ReadSome(buf, len);
        if (r < 0)
            return -1;
        else if (r == 0)
            return read_count;
        else
        {
            read_count += r;
            buf += r;
            len -= r;
        }
    }

    return read_count;
}

int BiliPipe::ReadSome(char* buf, int len)
{
    if (buffer_.empty() && !OnBufUnderflow())
        return -1;

    size_t toCopy = std::min(static_cast<int>(buffer_.size()), len);
    std::copy_n(buffer_.begin(), toCopy, buf);
    buffer_.erase(buffer_.begin(), buffer_.begin() + toCopy);
    return toCopy;
}

int BiliPipe::ReadUntil(char* buf, int len, char deli)
{
    int read_count = 0;

    while (len > 0)
    {
        if (buffer_.empty() && !OnBufUnderflow())
            return -1;

        int to_copy_size;
        bool is_found;
        {
            auto find_pos = std::find(buffer_.begin(), buffer_.end(), deli);

            to_copy_size = std::min(static_cast<int>(find_pos - buffer_.begin()), len);
            is_found = find_pos != buffer_.end();
            if (is_found)
                ++to_copy_size;
        }

        std::copy_n(buffer_.data(), to_copy_size, buf);
        buf += to_copy_size;
        len -= to_copy_size;
        read_count += to_copy_size;
        buffer_.erase(buffer_.begin(), buffer_.begin() + to_copy_size);

        if (is_found)
            break;
    }

    return read_count;
}

int BiliPipe::WriteFixed(const char* buf, int len)
{
    int write_count = 0;
    while (len > 0)
    {
        int r = WriteSome(buf, len);
        if (r < 0)
            return -1;
        else if (r == 0)
            ;
        else
        {
            buf += r;
            len -= r;
            write_count += r;
        }
    }

    return write_count;
}

int BiliPipe::WriteSome(const char* buf, int len)
{
    DWORD data_written = 0;

    if (WriteFile(data_pipe_handle_.Get(), buf, len, &data_written, 0) != FALSE)
        return data_written;
    else
        return -1;
}







static HANDLE sBiliProcessJob = NULL;

static class BiliProcessJobInitializer
{
public:
    BiliProcessJobInitializer()
    {
        BOOL bret;
        sBiliProcessJob = CreateJobObject(NULL, NULL);
        if (sBiliProcessJob)
        {
            JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
            jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
            bret = SetInformationJobObject(sBiliProcessJob, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli));
        }
    }
} biliProcessJobInitializer;

BiliProcess::BiliProcess(const wchar_t* command_line, bool redir_stdin, bool redir_stdout, bool redir_stderr)
    : redir_stderr_(redir_stderr)
    , redir_stdin_(redir_stdin)
    , redir_stdout_(redir_stdout)
    , pipe_err_(nullptr)
    , pipe_in_(nullptr)
    , pipe_out_(nullptr)
{
    cmd_line_.resize(wcslen(command_line) + 1);
    std::copy_n(command_line, cmd_line_.size(), cmd_line_.begin());
}

BiliProcess::~BiliProcess()
{
    if (mpv_proc_handle_.IsValid())
        Detach();
}

void BiliProcess::Detach()
{
    if (mpv_proc_handle_.IsValid())
    {
        mpv_proc_handle_.Close();
    }
}

bool BiliProcess::Launch()
{
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = {};

    bool succeed = false;
    do
    {
        if (redir_stdin_ || redir_stdout_ || redir_stderr_)
        {
            si.dwFlags = STARTF_USESTDHANDLES;

            if (redir_stdin_)
            {
                HANDLE read_handle = NULL, wirte_handle = NULL;
                if (!CreatePipe(&read_handle, &wirte_handle, 0, 0))
                {
                    break;
                }
                SetHandleInformation(read_handle, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
                si.hStdInput = read_handle;
                pipe_in_.reset(new BiliPipe(wirte_handle));
            }
            else
                si.hStdInput = INVALID_HANDLE_VALUE;

            if (redir_stdout_)
            {
                HANDLE read_handle = NULL, wirte_handle = NULL;
                if (!CreatePipe(&read_handle, &wirte_handle, 0, 0))
                {
                    break;
                }
                SetHandleInformation(wirte_handle, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
                si.hStdOutput = wirte_handle;
                pipe_out_.reset(new BiliPipe(read_handle));
            }
            else
                si.hStdOutput = INVALID_HANDLE_VALUE;

            if (redir_stderr_)
            {
                HANDLE read_handle = NULL, wirte_handle = NULL;
                if (!CreatePipe(&read_handle, &wirte_handle, 0, 0))
                {
                    break;
                }
                SetHandleInformation(wirte_handle, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
                si.hStdError = wirte_handle;
                pipe_err_.reset(new BiliPipe(read_handle));
            }
            else
                si.hStdError = INVALID_HANDLE_VALUE;
        }

        if (CreateProcessW(nullptr, cmd_line_.data(), 0, 0, TRUE, CREATE_NO_WINDOW, 0, 0, &si, &pi) == FALSE)
        {
            break;
        }
        CloseHandle(pi.hThread);
        AssignProcessToJobObject(sBiliProcessJob, pi.hProcess);
        mpv_proc_handle_.Set(pi.hProcess);

        succeed = true;
    } while (0);

    if (si.hStdInput != INVALID_HANDLE_VALUE)
        CloseHandle(si.hStdInput);
    if (si.hStdOutput != INVALID_HANDLE_VALUE)
        CloseHandle(si.hStdOutput);
    if (si.hStdError != INVALID_HANDLE_VALUE)
        CloseHandle(si.hStdError);

    if (!succeed)
    {
        pipe_in_.reset();
        pipe_out_.reset();
        pipe_err_.reset();
    }

    return succeed;
}

BiliPipe* BiliProcess::Stderr()
{
    DCHECK(redir_stderr_);

    return pipe_err_.get();
}

BiliPipe* BiliProcess::Stdin()
{
    DCHECK(redir_stdin_);

    return pipe_in_.get();
}

BiliPipe* BiliProcess::Stdout()
{
    DCHECK(redir_stdout_);

    return pipe_out_.get();
}

int BiliProcess::Join(int timeout_ms)
{
    if (!mpv_proc_handle_.IsValid())
        return -1;

    DWORD time_to_wait = timeout_ms;

    if (timeout_ms == -1)
        time_to_wait = INFINITE;

    if (WaitForSingleObject(mpv_proc_handle_.Get(), timeout_ms) == WAIT_OBJECT_0)
    {
        DWORD exit_code = 0;
        GetExitCodeProcess(mpv_proc_handle_.Get(), &exit_code);
        return exit_code;
    }
    else
        return -1;
}

bool BiliProcess::Terminate(int exit_code)
{
    if (!mpv_proc_handle_.IsValid())
        return false;

    return TerminateProcess(mpv_proc_handle_.Get(), exit_code) != FALSE;
}

BiliProcess::operator void* ()
{
    if (mpv_proc_handle_.IsValid())
        return this;
    else
        return nullptr;
}
