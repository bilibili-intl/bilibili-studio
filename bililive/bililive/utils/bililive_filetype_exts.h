#ifndef BILILIVE_BILILIVE_UTILS_BILILIVE_FILETYPE_EXTS_H_
#define BILILIVE_BILILIVE_UTILS_BILILIVE_FILETYPE_EXTS_H_

#include "base/files/file_path.h"

#include "obs/obs_proxy/public/proxy/obs_sources_wrapper.h"

namespace bililive {

enum class SourceType;

const wchar_t **GetMediaFileExts();
const wchar_t **GetImageFileExts();
const wchar_t **GetTextFileExts();

bool GuessSourceType(const base::FilePath& filePath, SourceType& source_type);

}   // namespace bililive

#endif  // BILILIVE_BILILIVE_UTILS_BILILIVE_FILETYPE_EXTS_H_
