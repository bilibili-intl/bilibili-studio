// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/zlib/google/zip.h"

#include "base/bind.h"
#include "base/file_util.h"
#include "base/files/file_enumerator.h"
#include "base/logging.h"
#include "base/strings/string16.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/files/file_stream_win.h"
#include "third_party/zlib/google/zip_internal.h"
#include "third_party/zlib/google/zip_reader.h"

#if defined(USE_SYSTEM_MINIZIP)
#include <minizip/unzip.h>
#include <minizip/zip.h>
#else
#include "third_party/zlib/contrib/minizip/unzip.h"
#include "third_party/zlib/contrib/minizip/zip.h"
#endif

namespace {

bool AddFileToZip(zipFile zip_file, const base::FilePath& src_dir) {
  base::FileStreamWin stream;
  int flags = base::PLATFORM_FILE_OPEN | base::PLATFORM_FILE_READ;
  if (stream.OpenSync(src_dir, flags) != 0) {
    DLOG(ERROR) << "Could not open stream for path "
                << src_dir.value();
    return false;
  }

  int num_bytes;
  char buf[zip::internal::kZipBufSize];
  do {
    num_bytes = stream.ReadSync(buf, zip::internal::kZipBufSize);
    if (num_bytes > 0) {
      if (ZIP_OK != zipWriteInFileInZip(zip_file, buf, num_bytes)) {
        DLOG(ERROR) << "Could not write data to zip for path "
                    << src_dir.value();
        return false;
      }
    }
  } while (num_bytes > 0);

  return true;
}

bool AddEntryToZip(zipFile zip_file, const base::FilePath& path,
                   const base::FilePath& root_path,
                  /*bililive[*/const zip::ReplaceCallback& replace_cb,
                   const zip::NewFilePathCallback& nfp_cb/*]*/) {
  std::string str_path =
      path.AsUTF8Unsafe().substr(root_path.AsUTF8Unsafe().length() + 1);

  // bililive[
  if (!nfp_cb.is_null()) {
    str_path = nfp_cb.Run(str_path);
  }
  // ]

#if defined(OS_WIN)
  ReplaceSubstringsAfterOffset(&str_path, 0u, "\\", "/");
#endif

  bool is_directory = base::DirectoryExists(path);
  if (is_directory)
    str_path += "/";

  if (ZIP_OK != zipOpenNewFileInZip(
      zip_file, str_path.c_str(),
      NULL, NULL, 0u, NULL, 0u, NULL,  // file info, extrafield local, length,
                                       // extrafield global, length, comment
      Z_DEFLATED, Z_DEFAULT_COMPRESSION)) {
    DLOG(ERROR) << "Could not open zip file entry " << str_path;
    return false;
  }

  bool success = true;
  if (!is_directory) {
    // bililive[
    if (replace_cb.is_null()) {
      success = AddFileToZip(zip_file, path);
    } else {
      auto rep_path = replace_cb.Run(path);
      success = AddFileToZip(zip_file, rep_path);
    }
    //success = AddFileToZip(zip_file, path);
    // ]
  }

  if (ZIP_OK != zipCloseFileInZip(zip_file)) {
    DLOG(ERROR) << "Could not close zip file entry " << str_path;
    return false;
  }

  return success;
}

bool ExcludeNoFilesFilter(const base::FilePath& file_path) {
  return true;
}

bool ExcludeHiddenFilesFilter(const base::FilePath& file_path) {
  return file_path.BaseName().value()[0] != '.';
}

}  // namespace

namespace zip {

bool Unzip(const base::FilePath& src_file, const base::FilePath& dest_dir) {
  ZipReader reader;
  if (!reader.Open(src_file)) {
    DLOG(WARNING) << "Failed to open " << src_file.value();
    return false;
  }
  while (reader.HasMore()) {
    if (!reader.OpenCurrentEntryInZip()) {
      DLOG(WARNING) << "Failed to open the current file in zip";
      return false;
    }
    if (reader.current_entry_info()->is_unsafe()) {
      DLOG(WARNING) << "Found an unsafe file in zip "
                    << reader.current_entry_info()->file_path().value();
      return false;
    }
    if (!reader.ExtractCurrentEntryIntoDirectory(dest_dir)) {
      DLOG(WARNING) << "Failed to extract "
                    << reader.current_entry_info()->file_path().value();
      return false;
    }
    if (!reader.AdvanceToNextEntry()) {
      DLOG(WARNING) << "Failed to advance to the next file";
      return false;
    }
  }
  return true;
}

// bililive[
bool Unzip_64(const base::FilePath& src_file, const base::FilePath& dest_dir) {
    ZipReader reader;
    if (!reader.Open_64(src_file)) {
        DLOG(WARNING) << "Failed to open " << src_file.value();
        return false;
    }
    while (reader.HasMore()) {
        if (!reader.OpenCurrentEntryInZip_64()) {
            DLOG(WARNING) << "Failed to open the current file in zip_64";
            return false;
        }
        if (reader.current_entry_info()->is_unsafe()) {
            DLOG(WARNING) << "Found an unsafe file in zip_64 "
                << reader.current_entry_info()->file_path().value();
            return false;
        }
        if (!reader.ExtractCurrentEntryIntoDirectory(dest_dir)) {
            DLOG(WARNING) << "Failed to extract "
                << reader.current_entry_info()->file_path().value();
            return false;
        }
        if (!reader.AdvanceToNextEntry()) {
            DLOG(WARNING) << "Failed to advance to the next file";
            return false;
        }
    }
    return true;
}
// bililive[

// bililive[
bool UnzipFromString(const std::string& zip_data, const base::FilePath& dest_dir) {
    ZipReader reader;
    if (!reader.OpenFromString(zip_data)) {
        DLOG(WARNING) << "Failed to open from string data";
        return false;
    }
    while (reader.HasMore()) {
        if (!reader.OpenCurrentEntryInZip()) {
            DLOG(WARNING) << "Failed to open the current file in zip";
            return false;
        }
        if (reader.current_entry_info()->is_unsafe()) {
            DLOG(WARNING) << "Found an unsafe file in zip "
                << reader.current_entry_info()->file_path().value();
            return false;
        }
        if (!reader.ExtractCurrentEntryIntoDirectory(dest_dir)) {
            DLOG(WARNING) << "Failed to extract "
                << reader.current_entry_info()->file_path().value();
            return false;
        }
        if (!reader.AdvanceToNextEntry()) {
            DLOG(WARNING) << "Failed to advance to the next file";
            return false;
        }
    }
    return true;
}
// ]bililive

bool ZipWithFilterCallback(const base::FilePath& src_dir,
                           const base::FilePath& dest_file,
                           const FilterCallback& filter_cb,
                           /*bililive[*/const ReplaceCallback& replace_cb,
                            const NewFilePathCallback& nfp_cb/*]*/) {
  DCHECK(base::DirectoryExists(src_dir));

  zipFile zip_file = internal::OpenForZipping(dest_file.AsUTF8Unsafe(),
                                              APPEND_STATUS_CREATE);

  if (!zip_file) {
    DLOG(WARNING) << "couldn't create file " << dest_file.value();
    return false;
  }

  bool success = true;
  base::FileEnumerator file_enumerator(src_dir, true /* recursive */,
      base::FileEnumerator::FILES | base::FileEnumerator::DIRECTORIES);
  for (base::FilePath path = file_enumerator.Next(); !path.value().empty();
       path = file_enumerator.Next()) {
    if (!filter_cb.Run(path)) {
      continue;
    }

    if (!AddEntryToZip(zip_file, path, src_dir, replace_cb, nfp_cb)) {
      success = false;
      return false;
    }
  }

  if (ZIP_OK != zipClose(zip_file, NULL)) {
    DLOG(ERROR) << "Error closing zip file " << dest_file.value();
    return false;
  }

  return success;
}

bool Zip(const base::FilePath& src_dir, const base::FilePath& dest_file,
         bool include_hidden_files) {
  if (include_hidden_files) {
    return ZipWithFilterCallback(
        src_dir, dest_file, base::Bind(&ExcludeNoFilesFilter), {}, {});
  } else {
    return ZipWithFilterCallback(
        src_dir, dest_file, base::Bind(&ExcludeHiddenFilesFilter), {}, {});
  }
}

#if defined(OS_POSIX)
bool ZipFiles(const base::FilePath& src_dir,
              const std::vector<base::FilePath>& src_relative_paths,
              int dest_fd) {
  DCHECK(base::DirectoryExists(src_dir));
  zipFile zip_file = internal::OpenFdForZipping(dest_fd, APPEND_STATUS_CREATE);

  if (!zip_file) {
    DLOG(ERROR) << "couldn't create file for fd " << dest_fd;
    return false;
  }

  bool success = true;
  for (std::vector<base::FilePath>::const_iterator iter =
           src_relative_paths.begin();
      iter != src_relative_paths.end(); ++iter) {
    const base::FilePath& path = src_dir.Append(*iter);
    if (!AddEntryToZip(zip_file, path, src_dir)) {
      // TODO(hshi): clean up the partial zip file when error occurs.
      success = false;
      break;
    }
  }

  if (ZIP_OK != zipClose(zip_file, NULL)) {
    DLOG(ERROR) << "Error closing zip file for fd " << dest_fd;
    success = false;
  }

  return success;
}
#endif  // defined(OS_POSIX)

}  // namespace zip
