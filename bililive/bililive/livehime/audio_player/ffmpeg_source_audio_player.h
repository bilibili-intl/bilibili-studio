#ifndef BILILIVE_BILILIVE_LIVEHIME_FFMPEG_SOURCE_AUDIO_PLAYER_H_
#define BILILIVE_BILILIVE_LIVEHIME_FFMPEG_SOURCE_AUDIO_PLAYER_H_

#include <string>
#include "bililive/bililive/livehime/audio_player/audio_player_presenter.h"
#include "obs/obs_proxy/core_proxy/core_proxy_impl.h"

class FfmpegSourceAudioPlayer 
	:public AudioPlayerPresenter {
public:
	FfmpegSourceAudioPlayer();
	~FfmpegSourceAudioPlayer();
	virtual void InitPlayer() override;
	virtual int OpenFile(const std::string& filename,
						const std::string& fomat,
						bool is_local) override;
	virtual void Player() override;
	virtual void StopPlayer() override;
	virtual void SetVolume(int volume) override;
	virtual bool IsOnPlay() override;
	virtual float GetDuration() override;
	virtual std::string GetFileName() override;

	void TestPlayer(std::string& filename, const std::string& fomat,bool is_local);
	virtual void RegisterMediaPlayStatus(MediaPlayHandler handle) override {
		media_play_handler_ = handle;
	}
private:
	void CreateAudioSource();
	void RemoveAudioSource();

	static void OnReceiverSignalMediaEnded(void* data, calldata_t* params);
	static void OnReceiverSignalMediaStarted(void* data, calldata_t* params);
	
	obs_source_t* audio_source_;
	OBSSignal     receiver_signal_started_;
	OBSSignal     receiver_signal_ended_;
	MediaPlayHandler   media_play_handler_;
};


#endif //BILILIVE_BILILIVE_LIVEHIME_FFMPEG_SOURCE_AUDIO_PLAYER_H_