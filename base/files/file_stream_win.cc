// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <windows.h>

#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "base/metrics/histogram.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/thread_restrictions.h"
#include "base/threading/worker_pool.h"
#include "file_stream_win.h"
#include "io_buffer.h"
#include "net_error_list.h"

namespace base {
// Ensure that we can just use our Whence values directly.
COMPILE_ASSERT(FileStreamWin::FROM_BEGIN == FILE_BEGIN, bad_whence_begin);
COMPILE_ASSERT(FileStreamWin::FROM_CURRENT == FILE_CURRENT, bad_whence_current);
COMPILE_ASSERT(FileStreamWin::FROM_END == FILE_END, bad_whence_end);

namespace {

void SetOffset(OVERLAPPED* overlapped, const LARGE_INTEGER& offset) {
  overlapped->Offset = offset.LowPart;
  overlapped->OffsetHigh = offset.HighPart;
}

void IncrementOffset(OVERLAPPED* overlapped, DWORD count) {
  LARGE_INTEGER offset;
  offset.LowPart = overlapped->Offset;
  offset.HighPart = overlapped->OffsetHigh;
  offset.QuadPart += static_cast<LONGLONG>(count);
  SetOffset(overlapped, offset);
}

// Opens a file with some network logging.
// The opened file and the result code are written to |file| and |result|.
void OpenFile(const FilePath& path,
              int open_flags,
              bool record_uma,
              base::PlatformFile* file,
              int* result) {
  std::string file_name = path.AsUTF8Unsafe();
  *file = base::CreatePlatformFile(path, open_flags, NULL, NULL);
  if (*file == base::kInvalidPlatformFileValue) {
    DWORD error = GetLastError();
    LOG(WARNING) << "Failed to open file: " << error;
    *result = error;
    return;
  }
}

// Closes a file with some network logging.
void CloseFile(base::PlatformFile file) {
  if (file == base::kInvalidPlatformFileValue)
    return;

  CancelIo(file);

  if (!base::ClosePlatformFile(file))
    NOTREACHED();
}

// Closes a file with CloseFile() and signals the completion.
void CloseFileAndSignal(base::PlatformFile* file,
                        base::WaitableEvent* on_io_complete) {
  CloseFile(*file);
  *file = base::kInvalidPlatformFileValue;
  on_io_complete->Signal();
}

// Invokes a given closure and signals the completion.
void InvokeAndSignal(const base::Closure& closure,
                     base::WaitableEvent* on_io_complete) {
  closure.Run();
  on_io_complete->Signal();
}

}  // namespace

// FileStreamWin::AsyncContext ----------------------------------------------

class FileStreamWin::AsyncContext : public MessageLoopForIO::IOHandler {
 public:
  explicit AsyncContext()
      : context_(), is_closing_(false),
        record_uma_(false){
    context_.handler = this;
  }
  ~AsyncContext();

  void IOCompletionIsPending(const CompletionCallback& callback,
                             IOBuffer* buf);

  OVERLAPPED* overlapped() { return &context_.overlapped; }
  const CompletionCallback& callback() const { return callback_; }

  void EnableErrorStatistics() {
    record_uma_ = true;
  }

 private:
  virtual void OnIOCompleted(MessageLoopForIO::IOContext* context,
                             DWORD bytes_read, DWORD error) OVERRIDE;

  MessageLoopForIO::IOContext context_;
  CompletionCallback callback_;
  scoped_refptr<IOBuffer> in_flight_buf_;
  bool is_closing_;
  bool record_uma_;
};

FileStreamWin::AsyncContext::~AsyncContext() {
  is_closing_ = true;
  bool waited = false;
  base::TimeTicks start = base::TimeTicks::Now();
  while (!callback_.is_null()) {
    waited = true;
    MessageLoopForIO::current()->WaitForIOCompletion(INFINITE, this);
  }
  if (waited) {
    // We want to see if we block the message loop for too long.
    UMA_HISTOGRAM_TIMES("AsyncIO.FileStreamClose",
                        base::TimeTicks::Now() - start);
  }
}

void FileStreamWin::AsyncContext::IOCompletionIsPending(
    const CompletionCallback& callback,
    IOBuffer* buf) {
  DCHECK(callback_.is_null());
  callback_ = callback;
  in_flight_buf_ = buf;  // Hold until the async operation ends.
}

void FileStreamWin::AsyncContext::OnIOCompleted(
    MessageLoopForIO::IOContext* context, DWORD bytes_read, DWORD error) {
  DCHECK_EQ(&context_, context);
  DCHECK(!callback_.is_null());

  if (is_closing_) {
    callback_.Reset();
    in_flight_buf_ = NULL;
    return;
  }

  int result = static_cast<int>(bytes_read);
  if (bytes_read)
    IncrementOffset(&context->overlapped, bytes_read);

  CompletionCallback temp_callback = callback_;
  callback_.Reset();
  scoped_refptr<IOBuffer> temp_buf = in_flight_buf_;
  in_flight_buf_ = NULL;
  temp_callback.Run(result);
}

// FileStream ------------------------------------------------------------

FileStreamWin::FileStreamWin()
    : file_(base::kInvalidPlatformFileValue),
      open_flags_(0),
      auto_closed_(true),
      record_uma_(false),
      weak_ptr_factory_(this) {
}

FileStreamWin::FileStreamWin(
    base::PlatformFile file, int flags)
    : file_(file),
      open_flags_(flags),
      auto_closed_(false),
      record_uma_(false),
      weak_ptr_factory_(this) {
  // If the file handle is opened with base::PLATFORM_FILE_ASYNC, we need to
  // make sure we will perform asynchronous File IO to it.
  if (flags & base::PLATFORM_FILE_ASYNC) {
    async_context_.reset(new AsyncContext());
    MessageLoopForIO::current()->RegisterIOHandler(file_,
                                                   async_context_.get());
  }
}

FileStreamWin::~FileStreamWin() {
  if (open_flags_ & base::PLATFORM_FILE_ASYNC) {
    // Block until the in-flight open/close operation is complete.
    // TODO(satorux): Ideally we should not block. crbug.com/115067
    WaitForIOCompletion();

    // Block until the last read/write operation is complete.
    async_context_.reset();
  }

  if (auto_closed_) {
    if (open_flags_ & base::PLATFORM_FILE_ASYNC) {
      // Close the file in the background.
      if (IsOpen()) {
        const bool posted = base::WorkerPool::PostTask(
            FROM_HERE,
            base::Bind(&CloseFile, file_),
            true /* task_is_slow */);
        DCHECK(posted);
      }
    } else {
      CloseSync();
    }
  }
}

void FileStreamWin::Close(const CompletionCallback& callback) {
  DCHECK(open_flags_ & base::PLATFORM_FILE_ASYNC);
  DCHECK(!weak_ptr_factory_.HasWeakPtrs());
  DCHECK(!on_io_complete_.get());
  on_io_complete_.reset(new base::WaitableEvent(
      false  /* manual_reset */, false  /* initially_signaled */));

  // Passing &file_ to a thread pool looks unsafe but it's safe here as the
  // destructor ensures that the close operation is complete with
  // WaitForIOCompletion(). See also the destructor.
  const bool posted = base::WorkerPool::PostTaskAndReply(
      FROM_HERE,
      base::Bind(&CloseFileAndSignal, &file_, on_io_complete_.get()),
      base::Bind(&FileStreamWin::OnClosed,
                 weak_ptr_factory_.GetWeakPtr(),
                 callback),
      true /* task_is_slow */);
  DCHECK(posted);
}

void FileStreamWin::CloseSync() {
  // The logic here is similar to CloseFile() but async_context_.reset() is
  // caled in this function.

  // Block until the in-flight open operation is complete.
  // TODO(satorux): Replace this with a DCHECK(open_flags & ASYNC) once this
  // once all async clients are migrated to use Close(). crbug.com/114783
  WaitForIOCompletion();

  if (file_ != base::kInvalidPlatformFileValue)
    CancelIo(file_);

  // Block until the last read/write operation is complete.
  async_context_.reset();

  if (file_ != base::kInvalidPlatformFileValue) {
    if (!base::ClosePlatformFile(file_))
      NOTREACHED();
    file_ = base::kInvalidPlatformFileValue;
  }
}

int FileStreamWin::Open(const FilePath& path, int open_flags,
                        const CompletionCallback& callback) {
  if (IsOpen()) {
    DLOG(FATAL) << "File is already open!";
    return ERR_UNEXPECTED;
  }

  open_flags_ = open_flags;
  DCHECK(open_flags_ & base::PLATFORM_FILE_ASYNC);
  DCHECK(!weak_ptr_factory_.HasWeakPtrs());
  DCHECK(!on_io_complete_.get());
  on_io_complete_.reset(new base::WaitableEvent(
      false  /* manual_reset */, false  /* initially_signaled */));

  // Passing &file_ to a thread pool looks unsafe but it's safe here as the
  // destructor ensures that the open operation is complete with
  // WaitForIOCompletion().  See also the destructor.
  int* result = new int(OK);
  const bool posted = base::WorkerPool::PostTaskAndReply(
      FROM_HERE,
      base::Bind(&InvokeAndSignal,
                 base::Bind(&OpenFile, path, open_flags, record_uma_, &file_,
                            result),
                 on_io_complete_.get()),
      base::Bind(&FileStreamWin::OnOpened,
                 weak_ptr_factory_.GetWeakPtr(),
                 callback, base::Owned(result)),
      true /* task_is_slow */);
  DCHECK(posted);
  return ERR_IO_PENDING;
}

int FileStreamWin::OpenSync(const FilePath& path, int open_flags) {
  if (IsOpen()) {
    DLOG(FATAL) << "File is already open!";
    return ERR_UNEXPECTED;
  }

  open_flags_ = open_flags;

  int result = OK;
  OpenFile(path, open_flags_, record_uma_, &file_, &result);
  if (result != OK)
    return result;

  // TODO(satorux): Remove this once all async clients are migrated to use
  // Open(). crbug.com/114783
  if (open_flags_ & base::PLATFORM_FILE_ASYNC) {
    async_context_.reset(new AsyncContext());
    if (record_uma_)
      async_context_->EnableErrorStatistics();
    MessageLoopForIO::current()->RegisterIOHandler(file_,
                                                   async_context_.get());
  }

  return OK;
}

bool FileStreamWin::IsOpen() const {
  return file_ != base::kInvalidPlatformFileValue;
}

int FileStreamWin::Seek(Whence whence, int64 offset,
                        const Int64CompletionCallback& callback) {
  if (!IsOpen())
    return ERR_UNEXPECTED;

  // Make sure we're async and we have no other in-flight async operations.
  DCHECK(open_flags_ & base::PLATFORM_FILE_ASYNC);
  DCHECK(!weak_ptr_factory_.HasWeakPtrs());
  DCHECK(!on_io_complete_.get());

  int64* result = new int64(-1);
  on_io_complete_.reset(new base::WaitableEvent(
      false  /* manual_reset */, false  /* initially_signaled */));

  const bool posted = base::WorkerPool::PostTaskAndReply(
      FROM_HERE,
      base::Bind(&InvokeAndSignal,
                 // Unretained should be fine as we wait for a signal on
                 // on_io_complete_ at the destructor.
                 base::Bind(&FileStreamWin::SeekFile, base::Unretained(this),
                            whence, offset, result),
                 on_io_complete_.get()),
      base::Bind(&FileStreamWin::OnSeeked,
                 weak_ptr_factory_.GetWeakPtr(),
                 callback, base::Owned(result)),
      true /* task is slow */);
  DCHECK(posted);
  return ERR_IO_PENDING;
}

int64 FileStreamWin::SeekSync(Whence whence, int64 offset) {
  if (!IsOpen())
    return ERR_UNEXPECTED;

  DCHECK(!async_context_.get() || async_context_->callback().is_null());
  int64 result = -1;
  SeekFile(whence, offset, &result);
  return result;
}

int64 FileStreamWin::Available() {
  base::ThreadRestrictions::AssertIOAllowed();

  if (!IsOpen())
    return ERR_UNEXPECTED;

  int64 cur_pos = SeekSync(FROM_CURRENT, 0);
  if (cur_pos < 0)
    return cur_pos;

  LARGE_INTEGER file_size;
  if (!GetFileSizeEx(file_, &file_size)) {
    DWORD error = GetLastError();
    LOG(WARNING) << "GetFileSizeEx failed: " << error;
    return error;
  }

  return file_size.QuadPart - cur_pos;
}

int FileStreamWin::Read(
    IOBuffer* buf, int buf_len, const CompletionCallback& callback) {
  DCHECK(async_context_.get());

  if (!IsOpen())
    return ERR_UNEXPECTED;

  DCHECK(open_flags_ & base::PLATFORM_FILE_READ);

  OVERLAPPED* overlapped = NULL;
  DCHECK(!callback.is_null());
  DCHECK(async_context_->callback().is_null());
  overlapped = async_context_->overlapped();

  int rv = 0;

  DWORD bytes_read;
  if (!ReadFile(file_, buf->data(), buf_len, &bytes_read, overlapped)) {
    DWORD error = GetLastError();
    if (error == ERROR_IO_PENDING) {
      async_context_->IOCompletionIsPending(callback, buf);
      rv = ERR_IO_PENDING;
    } else if (error == ERROR_HANDLE_EOF) {
      rv = 0;  // Report EOF by returning 0 bytes read.
    } else {
      LOG(WARNING) << "ReadFile failed: " << error;
      rv = error;
    }
  } else if (overlapped) {
    async_context_->IOCompletionIsPending(callback, buf);
    rv = ERR_IO_PENDING;
  } else {
    rv = static_cast<int>(bytes_read);
  }
  return rv;
}

int FileStreamWin::ReadSync(char* buf, int buf_len) {
  DCHECK(!async_context_.get());
  base::ThreadRestrictions::AssertIOAllowed();

  if (!IsOpen())
    return ERR_UNEXPECTED;

  DCHECK(open_flags_ & base::PLATFORM_FILE_READ);

  int rv = 0;

  DWORD bytes_read;
  if (!ReadFile(file_, buf, buf_len, &bytes_read, NULL)) {
    DWORD error = GetLastError();
    if (error == ERROR_HANDLE_EOF) {
      rv = 0;  // Report EOF by returning 0 bytes read.
    } else {
      LOG(WARNING) << "ReadFile failed: " << error;
      rv = error;
    }
  } else {
    rv = static_cast<int>(bytes_read);
  }
  return rv;
}

int FileStreamWin::ReadUntilComplete(char *buf, int buf_len) {
  int to_read = buf_len;
  int bytes_total = 0;

  do {
    int bytes_read = ReadSync(buf, to_read);
    if (bytes_read <= 0) {
      if (bytes_total == 0)
        return bytes_read;

      return bytes_total;
    }

    bytes_total += bytes_read;
    buf += bytes_read;
    to_read -= bytes_read;
  } while (bytes_total < buf_len);

  return bytes_total;
}

int FileStreamWin::Write(
    IOBuffer* buf, int buf_len, const CompletionCallback& callback) {
  DCHECK(async_context_.get());

  if (!IsOpen())
    return ERR_UNEXPECTED;

  DCHECK(open_flags_ & base::PLATFORM_FILE_WRITE);

  OVERLAPPED* overlapped = NULL;
  DCHECK(!callback.is_null());
  DCHECK(async_context_->callback().is_null());
  overlapped = async_context_->overlapped();

  int rv = 0;
  DWORD bytes_written = 0;
  if (!WriteFile(file_, buf->data(), buf_len, &bytes_written, overlapped)) {
    DWORD error = GetLastError();
    if (error == ERROR_IO_PENDING) {
      async_context_->IOCompletionIsPending(callback, buf);
      rv = ERR_IO_PENDING;
    } else {
      LOG(WARNING) << "WriteFile failed: " << error;
      rv = error;
    }
  } else if (overlapped) {
    async_context_->IOCompletionIsPending(callback, buf);
    rv = ERR_IO_PENDING;
  } else {
    rv = static_cast<int>(bytes_written);
  }
  return rv;
}

int FileStreamWin::WriteSync(
    const char* buf, int buf_len) {
  DCHECK(!async_context_.get());
  base::ThreadRestrictions::AssertIOAllowed();

  if (!IsOpen())
    return ERR_UNEXPECTED;

  DCHECK(open_flags_ & base::PLATFORM_FILE_WRITE);

  int rv = 0;
  DWORD bytes_written = 0;
  if (!WriteFile(file_, buf, buf_len, &bytes_written, NULL)) {
    DWORD error = GetLastError();
    LOG(WARNING) << "WriteFile failed: " << error;
    rv = error;
  } else {
    rv = static_cast<int>(bytes_written);
  }
  return rv;
}

int FileStreamWin::Flush() {
  base::ThreadRestrictions::AssertIOAllowed();

  if (!IsOpen())
    return ERR_UNEXPECTED;

  DCHECK(open_flags_ & base::PLATFORM_FILE_WRITE);
  if (FlushFileBuffers(file_)) {
    return OK;
  }

  return GetLastError();
}

int64 FileStreamWin::Truncate(int64 bytes) {
  base::ThreadRestrictions::AssertIOAllowed();

  if (!IsOpen())
    return ERR_UNEXPECTED;

  // We'd better be open for writing.
  DCHECK(open_flags_ & base::PLATFORM_FILE_WRITE);

  // Seek to the position to truncate from.
  int64 seek_position = SeekSync(FROM_BEGIN, bytes);
  if (seek_position != bytes)
    return ERR_UNEXPECTED;

  // And truncate the file.
  BOOL result = SetEndOfFile(file_);
  if (!result) {
    DWORD error = GetLastError();
    LOG(WARNING) << "SetEndOfFile failed: " << error;
    return error;
  }

  // Success.
  return seek_position;
}

void FileStreamWin::EnableErrorStatistics() {
  record_uma_ = true;

  if (async_context_.get())
    async_context_->EnableErrorStatistics();
}

base::PlatformFile FileStreamWin::GetPlatformFileForTesting() {
  return file_;
}

void FileStreamWin::OnClosed(const CompletionCallback& callback) {
  file_ = base::kInvalidPlatformFileValue;

  // Reset this before Run() as Run() may issue a new async operation.
  ResetOnIOComplete();
  callback.Run(OK);
}

void FileStreamWin::SeekFile(Whence whence, int64 offset, int64* result) {
  LARGE_INTEGER distance, res;
  distance.QuadPart = offset;
  DWORD move_method = static_cast<DWORD>(whence);
  if (!SetFilePointerEx(file_, distance, &res, move_method)) {
    DWORD error = GetLastError();
    LOG(WARNING) << "SetFilePointerEx failed: " << error;
    *result = error;
    return;
  }
  if (async_context_.get()) {
    SetOffset(async_context_->overlapped(), res);
  }
  *result = res.QuadPart;
}

void FileStreamWin::OnOpened(const CompletionCallback& callback, int* result) {
  if (*result == OK) {
    async_context_.reset(new AsyncContext());
    if (record_uma_)
      async_context_->EnableErrorStatistics();
    MessageLoopForIO::current()->RegisterIOHandler(file_,
                                                   async_context_.get());
  }

  // Reset this before Run() as Run() may issue a new async operation.
  ResetOnIOComplete();
  callback.Run(*result);
}

void FileStreamWin::OnSeeked(
    const Int64CompletionCallback& callback,
    int64* result) {
  // Reset this before Run() as Run() may issue a new async operation.
  ResetOnIOComplete();
  callback.Run(*result);
}

void FileStreamWin::ResetOnIOComplete() {
  on_io_complete_.reset();
  weak_ptr_factory_.InvalidateWeakPtrs();
}

void FileStreamWin::WaitForIOCompletion() {
  // http://crbug.com/115067
  base::ThreadRestrictions::ScopedAllowWait allow_wait;
  if (on_io_complete_.get()) {
    on_io_complete_->Wait();
    on_io_complete_.reset();
  }
}

}  // namespace net
