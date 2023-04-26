#include "audio_player_presenter_imp.h"
#include "ffmpeg_source_audio_player.h"

#include "base/notification/notification_service.h"

#include "bililive/secret/public/event_tracking_service.h"
#include "bililive/bililive/livehime/event_report/event_report_center.h"
#include "bililive/public/bililive/bililive_notification_types.h"

AudioPlayerPresenterImp* AudioPlayerPresenterImp::GetInstance(){
	return Singleton<AudioPlayerPresenterImp>::get();
}

AudioPlayerPresenterImp::AudioPlayerPresenterImp()
:audio_player_(std::make_unique<FfmpegSourceAudioPlayer>()){
	
	audio_player_->RegisterMediaPlayStatus(
		std::bind(&AudioPlayerPresenterImp::OnMediaPlayStatus, this, std::placeholders::_1));
}

AudioPlayerPresenterImp::~AudioPlayerPresenterImp() {

}

void AudioPlayerPresenterImp::InitPlayer() {
	audio_player_->InitPlayer();
}

int AudioPlayerPresenterImp::OpenFile(const std::string& filename,
										const std::string& fomat, 
										bool is_local) {
	is_loacl_ = is_local;

	livehime::PolarisEventReport(
		secret::LivehimePolarisBehaviorEvent::VoiceDanmuClick,
		"action_type:" + std::to_string(!audio_player_->IsOnPlay()));

	//业务逻辑
	if (audio_player_->GetFileName() == filename) {
		if (audio_player_->IsOnPlay()) {
			audio_player_->StopPlayer(); 
		}
		else {
			audio_player_->OpenFile(filename, fomat, is_local);
		}	
	}
	else {
		audio_player_->OpenFile(filename, fomat, is_local);
	}
	return 0;
}

int AudioPlayerPresenterImp::OpenFileNoEventReport(const std::string& filename, const std::string& fomat, bool is_local)
{
	is_loacl_ = is_local;
	if (audio_player_->GetFileName() == filename) {
		if (audio_player_->IsOnPlay()) {
			audio_player_->StopPlayer();
		}
		audio_player_->OpenFile(filename, fomat, is_local);
	}
	else {
		audio_player_->OpenFile(filename, fomat, is_local);
	}
	return 0;
}

void AudioPlayerPresenterImp::Player() {
	audio_player_->Player();
}

void AudioPlayerPresenterImp::StopPlayer() {
	audio_player_->StopPlayer();
}

void AudioPlayerPresenterImp::SetVolume(int volume) {
	audio_player_->SetVolume(volume);
}

bool AudioPlayerPresenterImp::IsOnPlay() {
	return audio_player_->IsOnPlay();
}

float AudioPlayerPresenterImp::GetDuration() {
	return audio_player_->GetDuration();
}
void AudioPlayerPresenterImp::OnMediaPlayStatus(MediaPalyStatus status)
{
	//BililiveThread::PostTask(BililiveThread::UI,
	//	FROM_HERE,
	//	base::Bind(&AudioPlayerPresenterImp::OnMediaPlayStatusUI,
	//		base::Unretained(this), status));
}

void AudioPlayerPresenterImp::OnMediaPlayStatusUI(MediaPalyStatus status)
{
	bool play = false;
	if (MediaPalyStatus::MediaStart == status) {
		play = true;
	}
	else if (MediaPalyStatus::MeidPalyEnded == status) {
		play = false;
	}
	//通知弹幕姬
	auto details_info = AudioPlayInfo();
	details_info.paly = play;
	details_info.file_name = audio_player_->GetFileName();

	if (details_info.file_name.empty()) {
		return;
	}
	base::NotificationService::current()->Notify(
		bililive::NOTIFICATION_LIVEHIME_DANMAKU_VOICE_PLAY_STATE,
		base::NotificationService::AllSources(),
		base::Details<AudioPlayInfo>(&details_info));
}
