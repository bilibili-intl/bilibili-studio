#ifndef BILILIVE_BILILIVE_LIVEHIME_AUDIO_PLAYER_IMP_PRESENTER_H_
#define BILILIVE_BILILIVE_LIVEHIME_AUDIO_PLAYER_IMP_PRESENTER_H_

#include <string>
#include "base/timer/timer.h"
#include "base/memory/singleton.h"
#include "bililive/bililive/livehime/audio_player/audio_player_presenter.h"

class FfmpegSourceAudioPlayer;

struct AudioPlayInfo {
	bool paly;
	std::string file_name;
};

class AudioPlayerPresenterImp
	:public AudioPlayerPresenter{
public:
	static AudioPlayerPresenterImp* GetInstance();
	AudioPlayerPresenterImp();
	~AudioPlayerPresenterImp();
	virtual void InitPlayer() override;
	virtual int OpenFile(const std::string& filename,
						const std::string& fomat,
						bool is_local ) override;

	virtual int OpenFileNoEventReport(const std::string& filename,
									const std::string& fomat,
									bool is_local) override;

	virtual void Player() override;
	virtual void StopPlayer() override;
	virtual void SetVolume(int volume) override;
	virtual bool IsOnPlay() override;
	virtual float GetDuration() override;
private:
	void OnMediaPlayStatus(MediaPalyStatus status);
	void OnMediaPlayStatusUI(MediaPalyStatus status);

private:
	friend struct DefaultSingletonTraits<AudioPlayerPresenterImp>;
	std::unique_ptr<AudioPlayerPresenter> audio_player_;
	bool is_loacl_ = false;
};

#endif //BILILIVE_BILILIVE_LIVEHIME_AUDIO_PLAYER_IMP_PRESENTER_H_