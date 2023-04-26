#include "ffmpeg_source_audio_player.h"

#include "obs/obs_proxy/common/obs_proxy_constants.h"
#include "obs/obs_proxy/core_proxy/core_proxy_impl.h"

const char kMediaSourceName[] = "ffmpeg_audio_source";

FfmpegSourceAudioPlayer::FfmpegSourceAudioPlayer()
    :audio_source_(nullptr){
    InitPlayer();
}

FfmpegSourceAudioPlayer::~FfmpegSourceAudioPlayer() {
    receiver_signal_started_.Disconnect();
    receiver_signal_ended_.Disconnect();
    RemoveAudioSource();
}

void FfmpegSourceAudioPlayer::TestPlayer(std::string& filename,
                                        const std::string& fomat,
                                        bool is_local) {
    InitPlayer();
    OpenFile(filename,fomat, is_local);
}

void FfmpegSourceAudioPlayer::InitPlayer() {
    CreateAudioSource();
}

int FfmpegSourceAudioPlayer::OpenFile(const std::string& filename,
                                        const std::string& fomat,
                                        bool is_local) {
    //obs_source_t* audio_source = obs_get_output_source(obs_proxy::kAudioPlaySourceChannel);
    if (audio_source_) {
        obs_data_t* settings = obs_source_get_settings(audio_source_);
        int speed_percent = obs_data_get_int(settings,"speed_percent");
        obs_data_set_bool(settings, "is_local_file", is_local);
        if (is_local) {
            obs_data_set_string(settings,"local_file", filename.c_str());
        }
        else {
            obs_data_set_string(settings, "input", filename.c_str());
            obs_data_set_string(settings, "input_format", fomat.c_str());
        }
        obs_data_set_bool(settings, "looping", false);

        obs_source_update(audio_source_, settings);
        obs_data_release(settings);
        //obs_source_release(audio_source);
    }
	return 0;
}

void FfmpegSourceAudioPlayer::Player() {

}

void FfmpegSourceAudioPlayer::StopPlayer() {
    obs_source_media_stop(audio_source_);
}

void FfmpegSourceAudioPlayer::SetVolume(int volume) {

}

float FfmpegSourceAudioPlayer::GetDuration() {
    float duration = 0;
    //obs_source_t* audio_source = obs_get_output_source(obs_proxy::kAudioPlaySourceChannel);
    if (audio_source_) {
        float time = (float)obs_source_media_get_time(audio_source_);
        duration = (float)obs_source_media_get_duration(audio_source_);
    }
    return duration;
}

std::string FfmpegSourceAudioPlayer::GetFileName() {
    if (audio_source_) {
        std::string file_name;
        obs_data_t* settings = obs_source_get_settings(audio_source_);
        bool is_local = obs_data_get_bool(settings, "is_local_file");
        if (is_local) {
            file_name = obs_data_get_string(settings, "local_file");
        }
        else {
            file_name = obs_data_get_string(settings, "input");
        }
        obs_data_release(settings);
        return file_name;
    }
    return "";
}

bool FfmpegSourceAudioPlayer::IsOnPlay() {
    obs_media_state state = obs_source_media_get_state(audio_source_);
    if (state == OBS_MEDIA_STATE_PLAYING) {
        return true;
    }
	return false;
}

void FfmpegSourceAudioPlayer::CreateAudioSource() {

    audio_source_ = obs_get_output_source(obs_proxy::kAudioPlaySourceChannel);
    if (audio_source_) {
        obs_source_release(audio_source_);
        return;
    }

    obs_data_t* settings = obs_data_create();
    audio_source_ = obs_source_create(obs_proxy::kMediaSourceItemID, kMediaSourceName, settings, nullptr);

    if (audio_source_) {
        obs_source_set_audio_mixers(audio_source_, 1 | (1 << 1));
        obs_source_set_monitoring_type(
            audio_source_, obs_monitoring_type::OBS_MONITORING_TYPE_MONITOR_ONLY);
        obs_set_output_source(obs_proxy::kAudioPlaySourceChannel, audio_source_);

        auto signal_handle = obs_source_get_signal_handler(audio_source_);

        receiver_signal_started_.Connect(
            signal_handle,
            "media_started",
            &FfmpegSourceAudioPlayer::OnReceiverSignalMediaStarted,
            this);

        receiver_signal_ended_.Connect(
            signal_handle,
            "media_ended",
            &FfmpegSourceAudioPlayer::OnReceiverSignalMediaEnded,
            this);
    }
}

void FfmpegSourceAudioPlayer::RemoveAudioSource() {
    obs_source_t* audio_source = obs_get_output_source(obs_proxy::kAudioPlaySourceChannel);
    if (audio_source) {
        receiver_signal_started_.Disconnect();
        receiver_signal_ended_.Disconnect();
        obs_set_output_source(obs_proxy::kAudioPlaySourceChannel, nullptr);
    }
    audio_source_ = nullptr;
}
void FfmpegSourceAudioPlayer::OnReceiverSignalMediaStarted(void* data, calldata_t* params)
{
    if (data) {
        FfmpegSourceAudioPlayer* pthis = static_cast<FfmpegSourceAudioPlayer*>(data);
        if (pthis) {
            pthis->media_play_handler_(MediaPalyStatus::MediaStart);
        }
    }
}
void FfmpegSourceAudioPlayer::OnReceiverSignalMediaEnded(void* data, calldata_t* params)
{
    if (data) {
        FfmpegSourceAudioPlayer* pthis = static_cast<FfmpegSourceAudioPlayer*>(data);
        if (pthis) {
            pthis->media_play_handler_(MediaPalyStatus::MeidPalyEnded);
        }
    }
}




