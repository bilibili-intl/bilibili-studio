#ifndef BILILIVE_BILILIVE_LIVEHIME_AUDIO_PLAYER_PRESENTER_H_
#define BILILIVE_BILILIVE_LIVEHIME_AUDIO_PLAYER_PRESENTER_H_

#include <string>

enum class MediaPalyStatus {
	MediaStart = 0,
	MeidPalyEnded
};
using  MediaPlayHandler = std::function<void(MediaPalyStatus status)>;

class AudioPlayerInterface {
public:
	virtual void InitPlayer() = 0;
	virtual int OpenFile(const std::string& filename,
						const std::string& fomat,
						bool is_local) = 0;
	virtual int OpenFileNoEventReport(const std::string& filename,
									const std::string& fomat,
								    bool is_local) = 0;
	virtual void Player() = 0;
	virtual void StopPlayer() = 0;
	virtual void SetVolume(int volume) = 0;
	virtual bool IsOnPlay() = 0;
	virtual float GetDuration() = 0;
	virtual std::string GetFileName() = 0;
	virtual void RegisterMediaPlayStatus(MediaPlayHandler handler) = 0;
};

class AudioPlayerPresenter
	:public AudioPlayerInterface {
public:
	virtual void InitPlayer() {};
	virtual int OpenFile(const std::string& filename,
						const std::string& fomat,
						bool is_local) 
						{ return 0; };
	virtual int OpenFileNoEventReport(const std::string& filename,
									const std::string& fomat,
									bool is_local)
									{ return 0;}
	virtual void Player() {};
	virtual void StopPlayer() {};
	virtual void SetVolume(int volume) {};
	virtual bool IsOnPlay() { return true; };
	virtual float GetDuration() { return 0; };
	virtual std::string GetFileName() { return ""; }
	virtual void RegisterMediaPlayStatus(MediaPlayHandler handler) {}
};

#endif //BILILIVE_BILILIVE_LIVEHIME_AUDIO_PLAYER_PRESENTER_H_