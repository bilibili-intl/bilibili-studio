#ifndef OBS_OBS_PROXY_UTILS_AUDIO_ENCODERS_H_
#define OBS_OBS_PROXY_UTILS_AUDIO_ENCODERS_H_

#include <map>
#include <string>

namespace obs_proxy {

using BitrateMap = std::map<int, std::string>;

// Not thread-safe.
const BitrateMap& GetAudioEncoderBitrateTable();

int GetProperAudioBitrate();

std::string GetEncoderIDForBitrate(int bitrate);

}   // namespace obs_proxy

#endif  // BILILIVE_OBS_OBS_PROXY_UTILS_AUDIO_ENCODERS_H_