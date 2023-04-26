// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file implements FileStream for Windows.

#ifndef BASE_FILE_STREAM_WIN_H_
#define BASE_FILE_STREAM_WIN_H_
#pragma once

#include "base/callback.h"
#include "base/cancelable_callback.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/platform_file.h"
#include "base/synchronization/waitable_event.h"

class FilePath;

namespace base {
// A callback specialization that takes a single int parameter. Usually this is
// used to report a byte count or network error code.
typedef base::Callback<void(int)> CompletionCallback;

// 64bit version of callback specialization that takes a single int64 parameter.
// Usually this is used to report a file offset, size or network error code.
typedef base::Callback<void(int64)> Int64CompletionCallback;

typedef base::CancelableCallback<void(int)> CancelableCompletionCallback;

class WaitableEvent;

class IOBuffer;

class BASE_EXPORT FileStreamWin {
 public:

   enum Whence {
     FROM_BEGIN   = 0,
     FROM_CURRENT = 1,
     FROM_END     = 2
   };

  explicit FileStreamWin();
  FileStreamWin(base::PlatformFile file, int flags);
  ~FileStreamWin();

  // FileStream implementations.
  void Close(const CompletionCallback& callback);
  void CloseSync();
  int Open(const FilePath& path, int open_flags,
           const CompletionCallback& callback);
  int OpenSync(const FilePath& path, int open_flags);
  bool IsOpen() const;
  int Seek(Whence whence, int64 offset,
           const Int64CompletionCallback& callback);
  int64 SeekSync(Whence whence, int64 offset);
  int64 Available();
  int Read(IOBuffer* buf, int buf_len, const CompletionCallback& callback);
  int ReadSync(char* buf, int buf_len);
  int ReadUntilComplete(char *buf, int buf_len);
  int Write(IOBuffer* buf, int buf_len, const CompletionCallback& callback);
  int WriteSync(const char* buf, int buf_len);
  int64 Truncate(int64 bytes);
  int Flush();
  void EnableErrorStatistics();
  base::PlatformFile GetPlatformFileForTesting();

 private:
  class AsyncContext;

  // A helper method for Seek.
  void SeekFile(Whence whence, int64 offset, int64* result);

  // Called when the file_ is opened asynchronously. |result| contains the
  // result as a network error code.
  void OnOpened(const CompletionCallback& callback, int* result);

  // Called when the file_ is closed asynchronously.
  void OnClosed(const CompletionCallback& callback);

  // Called when the file_ is seeked asynchronously.
  void OnSeeked(const Int64CompletionCallback& callback, int64* result);

  // Resets on_io_complete_ and WeakPtr's.
  // Called in OnOpened, OnClosed and OnSeeked.
  void ResetOnIOComplete();

  // Waits until the in-flight async open/close operation is complete.
  void WaitForIOCompletion();

  // This member is used to support asynchronous reads.  It is non-null when
  // the FileStreamWin was opened with PLATFORM_FILE_ASYNC.
  scoped_ptr<AsyncContext> async_context_;

  base::PlatformFile file_;
  int open_flags_;
  bool auto_closed_;
  bool record_uma_;
  base::WeakPtrFactory<FileStreamWin> weak_ptr_factory_;
  scoped_ptr<base::WaitableEvent> on_io_complete_;

  DISALLOW_COPY_AND_ASSIGN(FileStreamWin);
};

}  // namespace base

#endif  // NET_BASE_FILE_STREAM_WIN_H_
