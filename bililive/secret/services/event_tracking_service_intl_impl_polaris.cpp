#include "bililive/secret/services/event_tracking_service_intl_impl.h"

#include <tuple>

#include "base/command_line.h"
#include "base/ext/callable_callback.h"
#include "base/file_util.h"
#include "base/guid.h"
#include "base/json/json_writer.h"
#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "base/values.h"

#include "net/http/http_request_headers.h"

#include "url/gurl.h"

#include "bililive/bililive/utils/net_util.h"
#include "bililive/common/bililive_context.h"
#include "bililive/secret/core/bililive_secret_core_impl.h"
#include "bililive/secret/services/service_utils.h"
#include "bililive/public/common/bililive_constants.h"
#include "bililive/public/protobuf/polaris_bsns_event_report.pb.h"
#include "bililive/public/protobuf/infoc_app_event.pb.h"

namespace
{
    using secret::ResponseInfo;
    using secret::LivehimePolarisBehaviorEvent;

    // @https://info.bilibili.co/pages/viewpage.action?pageId=176580825
    const char kURLLancerDataCenter[] = "";
    const char kLivehimeLogIDPolaris[] = "";
    const char kLivehimeTechLogIDPolaris[] = "";

    #define DETERMINE_FIELD_VALUE(value)    \
        value <= 0 ? secret::kRequireValueDummyInt : value;

    #define DETERMINE_FIELD_VALUE_STR(value)    \
        value.empty() ? secret::kRequireValueDummy : value;

    std::string LiveStatus(int live_status)
    {
        return (0 == live_status) ? "preparing" : "live";
    }

    const std::map<LivehimePolarisBehaviorEvent, std::string> kLivehimeBehaviorEventPolarisTable
    {
        { LivehimePolarisBehaviorEvent::StartLive, "live.live-zbj.startlive.0.click" },
        { LivehimePolarisBehaviorEvent::StartLiveResult, "live.live-zbj.startlive.result.show"},
        { LivehimePolarisBehaviorEvent::StartLiveTag,"live.live-zbj.startlive-tag.result.show"},
        { LivehimePolarisBehaviorEvent::ActiveClick, "live.live-zbj.sys-active.0.click" },
        { LivehimePolarisBehaviorEvent::StartUpShow, "live.live-zbj.startup.0.show" },

        { LivehimePolarisBehaviorEvent::MaterialItemClick, "live.live-zbj.material.0.click" },
        { LivehimePolarisBehaviorEvent::ApplistShow, "live.live-zbj.appli-list.0.show" },
        { LivehimePolarisBehaviorEvent::AppClick, "live.live-zbj.appli-list.0.click" },
        { LivehimePolarisBehaviorEvent::BannerItemShow, "live.live-zbj.banner.0.show" },
        { LivehimePolarisBehaviorEvent::BannerItemClick, "live.live-zbj.banner.0.click" },

        { LivehimePolarisBehaviorEvent::ActivityCenterClick, "live.live-zbj.activity.0.click" },
        { LivehimePolarisBehaviorEvent::GiftFoldClick, "live.live-zbj.dmj.gift-fold.click" },
        { LivehimePolarisBehaviorEvent::DanmakuFoldClick, "live.live-zbj.dmj.dm-fold.click" },
        { LivehimePolarisBehaviorEvent::DmkGiftTabClick, "live.live-zbj.dmj.gift-panel-tab.click" },
        { LivehimePolarisBehaviorEvent::PreviewBtnClick, "live.live-zbj.live-preview.0.click" },
        { LivehimePolarisBehaviorEvent::DmkMsgSend, "live.live-zbj.dmj.dm-send.click" },
        { LivehimePolarisBehaviorEvent::MuteBlacklistCommit, "live.live-zbj.dmj.mute-panel-commit.click" },
        { LivehimePolarisBehaviorEvent::DmkMsgReport, "live.live-zbj.dmj.report-panel-commit.click" },
        { LivehimePolarisBehaviorEvent::DmkBlockWordAddCommit, "live.live-zbj.room-set-panel.block-word-commit.click" },

        { LivehimePolarisBehaviorEvent::LiveStopMoreDataClick, "live.live-end-zbj.data-card.view-data.click" },
        { LivehimePolarisBehaviorEvent::LiveStopStarShow, "live.live-end-zbj.starlight-card.0.show" },
        { LivehimePolarisBehaviorEvent::LiveStopStarTipClick, "live.live-end-zbj.starlight-card.view-detail.click" },
        { LivehimePolarisBehaviorEvent::LiveStopClipShow, "live.live-end-zbj.dt-pushlish-card.0.show" },
        { LivehimePolarisBehaviorEvent::LiveStopPublicClick, "live.live-end-zbj.dt-pushlish-card.one-click-palyback.click" },
        { LivehimePolarisBehaviorEvent::LiveStopWatchReplayClick, "live.live-end-zbj.dt-pushlish-card.view-detial.click" },
        { LivehimePolarisBehaviorEvent::LiveStopAutoReplayClick, "live.live-end-zbj.dt-pushlish-card.sync-to-dt.click" },
        { LivehimePolarisBehaviorEvent::LiveStopReplayDlgShow, "live.live-end-zbj.default-palyback.0.show" },
        { LivehimePolarisBehaviorEvent::LiveStopReplayDlgClick, "live.live-end-zbj.default-palyback.0.click" },
        { LivehimePolarisBehaviorEvent::LiveStopHighlightsDlgShow, "live.live-end-zbj.highlights-moment.0.show" },
        { LivehimePolarisBehaviorEvent::LiveStopHighlightsBtnClick, "live.live-end-zbj.highlights-moment.card.click" },
        { LivehimePolarisBehaviorEvent::LiveStopHighlightsBtnAllClick, "live.live-end-zbj.highlights-moment.all.click" },

        { LivehimePolarisBehaviorEvent::SourceSettingShow, "live.live-zbj.material-setting.0.show" },
        { LivehimePolarisBehaviorEvent::SourceTextSensitiveShow, "live.live-zbj.text.remind.show" },
        { LivehimePolarisBehaviorEvent::SubtractSwitchClick, "live.live-zbj.material-setting.subtract-background.click" },
        { LivehimePolarisBehaviorEvent::SourceSettingAdvanceClick, "live.live-zbj.material-setting.advanced-setting.click" },
        { LivehimePolarisBehaviorEvent::SourceSettingCaptureMethod, "live.live-zbj.material-setting.capture-method.click" },
        { LivehimePolarisBehaviorEvent::SourceSettingWndAreaClick, "live.live-zbj.material-setting.window.click" },
        { LivehimePolarisBehaviorEvent::SourceSettingMultiGraphicClick, "live.live-zbj.material-setting.graphics.click" },
        { LivehimePolarisBehaviorEvent::SourceSettingAreoClick, "live.live-zbj.material-setting.areo.click" },
        { LivehimePolarisBehaviorEvent::SourceSettingSubmitClick, "live.live-zbj.material-setting.submit.click" },
        { LivehimePolarisBehaviorEvent::HandonLiveVerificationShow, "live.live-zbj.verification-panel.0.show" },
        { LivehimePolarisBehaviorEvent::HandonLiveVerificationClick, "live.live-zbj.verification-panel.0.click" },

        { LivehimePolarisBehaviorEvent::VtuberUploadFigma, "live.live-zbj.virtual-image-panel.upload.click" },
        { LivehimePolarisBehaviorEvent::VtuberDownloadFigma, "live.live-zbj.virtual-image-panel.download.click" },
        { LivehimePolarisBehaviorEvent::VtuberSetFigmaConfig, "live.live-zbj.virtual-image-set.set.click" },
        { LivehimePolarisBehaviorEvent::VtuverFaceAlineClick, "live.live-zbj.virtual-image-set.expression-calibration.click" },
        { LivehimePolarisBehaviorEvent::VtuverHeadAlineClick, "live.live-zbj.virtual-image-set.head-calibration.click" },
        { LivehimePolarisBehaviorEvent::VtuberPropWindow, "live.live-zbj.virtual-image-set.0.show" },
        { LivehimePolarisBehaviorEvent::VtuberSelectMotion, "live.live-zbj.virtual-image-set.action.click" },
        { LivehimePolarisBehaviorEvent::VtuberSelectKeyCombo, "live.live-zbj.virtual-image-set.hot-key.click" },
        { LivehimePolarisBehaviorEvent::VtuberStartMotion, "live.live-zbj.virtual-image-action.0.click" },

        { LivehimePolarisBehaviorEvent::ProjectionPanelShow, "live.live-zbj.screen-sharing.0.show" },
        { LivehimePolarisBehaviorEvent::ProjectionPanelClick, "live.live-zbj.screen-sharing.tab.click" },
        { LivehimePolarisBehaviorEvent::ProjectionIosInstall, "live.live-zbj.screen-sharing.install.click" },
        { LivehimePolarisBehaviorEvent::ProjectionIosSuccess, "live.live-zbj.screen-sharing.install-success.show" },

        { LivehimePolarisBehaviorEvent::ProjectionVoiceClick, "live.live-zbj.screen-sharing.voice-source.click" },
        { LivehimePolarisBehaviorEvent::ProjectionAudioClick, "live.live-zbj.screen-sharing.audio-monitor.click" },
        { LivehimePolarisBehaviorEvent::ProjectionSuccessShow, "live.live-zbj.screen-sharing.success.show" },

        { LivehimePolarisBehaviorEvent::VtuberGiftInteractSetting, "live.live-zbj.virtual-image-panel.gift-interaction.click" },
        { LivehimePolarisBehaviorEvent::VtuberGiftInteractSettingChanged, "live.live-zbj.virtual-image-gift-set.set.click" },

        { LivehimePolarisBehaviorEvent::VtuberPanelShow, "live.live-zbj.virtual-image-panel.figure.show" },
        { LivehimePolarisBehaviorEvent::VtuberPanelClick, "live.live-zbj.virtual-image-panel.figure.click" },


        { LivehimePolarisBehaviorEvent::LiveNewRoomBtnClick, "live.live-zbj.announcement.0.click" },
        { LivehimePolarisBehaviorEvent::LiveNewRoomSubClick, "live.live-zbj.announcement-panel.save.click" },

        { LivehimePolarisBehaviorEvent::VoiceDanmuSwitchClick, "live.live-zbj.danmu.voice-danmu.click" },
        { LivehimePolarisBehaviorEvent::VoiceDanmuClick, "live.live-zbj.dmj.voice-danmu.click" },

        { LivehimePolarisBehaviorEvent::InstallNew, "live.live-zbj.install.install.click" },
        { LivehimePolarisBehaviorEvent::InstalledResultNew, "live.live-zbj.install.result.click" },

        { LivehimePolarisBehaviorEvent::MainNoviceGuide, "live.live-zbj.guide-page.0.show" },
        { LivehimePolarisBehaviorEvent::MainWndPV, "live.live-zbj.0.0.pv" },
        { LivehimePolarisBehaviorEvent::Install, "live.live-zbj.install-panel.install.click" },
        { LivehimePolarisBehaviorEvent::InstalledResult, "live.live-zbj.install-result.0.click" },
        { LivehimePolarisBehaviorEvent::UserNotRealNameAuthToast, "live.live-zbj.real-name-auth-toast.0.show" },
        { LivehimePolarisBehaviorEvent::LiveRoomNoCoverToast, "live.live-zbj.cover-toast.0.show" },
        { LivehimePolarisBehaviorEvent::LiveRoomNoAreaToast, "live.live-zbj.area-toast.0.show" },
        { LivehimePolarisBehaviorEvent::LiveRoomCoverEntryClick, "live.live-zbj.cover.0.click" },
        { LivehimePolarisBehaviorEvent::LiveRoomAreaEntryClick, "live.live-zbj.area.0.click" },
        { LivehimePolarisBehaviorEvent::LiveRoomAreaSubmitClick, "live.live-zbj.area-panel.submit.click" },
        { LivehimePolarisBehaviorEvent::LiveRoomCoverRefusePV, "live.live-zbj.cover-unreview-panel.0.show" },
        { LivehimePolarisBehaviorEvent::LiveRoomCoverRefuseUplClick, "live.live-zbj.cover-unreview-panel.0.click" },
        { LivehimePolarisBehaviorEvent::AppMaterialEntryClick, "live.live-zbj.add-material.0.click" },
        { LivehimePolarisBehaviorEvent::PkPanelShow, "live.live-zbj.pk-panel.0.show" },
        { LivehimePolarisBehaviorEvent::PkPanelRecAnchorShow, "live.live-zbj.pk-panel.rec-anchor.show" },
        { LivehimePolarisBehaviorEvent::PkPanelRecAnchorClick, "live.live-zbj.pk-panel.rec-anchor.click" },
        { LivehimePolarisBehaviorEvent::PkInvitationShow, "live.live-zbj.pk-invitation.0.show" },
        { LivehimePolarisBehaviorEvent::PkInvitationClick, "live.live-zbj.pk-invitation.0.click" },
        { LivehimePolarisBehaviorEvent::CpmLiveBubbleShow, "live.live-zbj.live-bubble.0.show"},
        { LivehimePolarisBehaviorEvent::PkPanelEchoHelpShow, "live.live-zbj.pk-panel.echo-help.show"},
        { LivehimePolarisBehaviorEvent::PkPanelEchoHelpClick, "live.live-zbj.pk-panel.echo-help.click"},
        { LivehimePolarisBehaviorEvent::StormProgressShow, "live.live-zbj.thermal-storm-task.0.show" },
        { LivehimePolarisBehaviorEvent::StormProgressClick, "live.live-zbj.thermal-storm-task.0.click" },
        { LivehimePolarisBehaviorEvent::StormProgressTracker, "live.live-zbj.thermal-storm-progress.0.tracker" },
        { LivehimePolarisBehaviorEvent::StormDownLoadTracker, "live.live-zbj.storm-mp4-download.0.tracker" },
        { LivehimePolarisBehaviorEvent::StormApiProgressTracker, "live.live-zbj.thermal-storm-progress.api.tracker" },
        { LivehimePolarisBehaviorEvent::StormPlayTracker, "live.live-zbj.storm-mp4-play.0.tracker" },
        { LivehimePolarisBehaviorEvent::RecordClick, "live.live-zbj.record.0.click" },
        { LivehimePolarisBehaviorEvent::CertificationShow, "live.live-zbj.certification-install.0.show" },
        { LivehimePolarisBehaviorEvent::CertificationResultShow, "live.live-zbj.icertification-result.0.show" },
        { LivehimePolarisBehaviorEvent::DmjWindowStatus, "live.live-zbj.dmj.window-status.show" },
        { LivehimePolarisBehaviorEvent::DmjActiviyTaskSetting, "live.live-zbj.dmj.activiy-task-setting.show" },
        { LivehimePolarisBehaviorEvent::DmjRevenueSetting, "live.live-zbj.dmj.revenue-setting.show" },
        { LivehimePolarisBehaviorEvent::DmjDanmuSetting, "live.live-zbj.dmj.danmu-setting.show" },
        { LivehimePolarisBehaviorEvent::DmjActivityWindowEnterance, "live.live-zbj.dmj.activity-window-enterance.click" },
        { LivehimePolarisBehaviorEvent::DmjActivityWindowDetails, "live.live-zbj.dmj.activity-window-details.click" },
        { LivehimePolarisBehaviorEvent::DmjActivityWindowTask, "live.live-zbj.dmj.activity-window-task.click" },
        { LivehimePolarisBehaviorEvent::LiveRoomCoverShow, "live.live-zbj.cover.0.show" },
        { LivehimePolarisBehaviorEvent::LiveAssistantShow, "live.live-zbj.guidance-msg.0.show" },
        { LivehimePolarisBehaviorEvent::LiveAssistantClick, "live.live-zbj.guidance-msg.0.click" },
        { LivehimePolarisBehaviorEvent::CameraRotateClick, "live.live-zbj.material-setting.camera-rotation.click" },
        { LivehimePolarisBehaviorEvent::PresetMaterialMainViewShow, "live.live-zbj.preset.0.show" },
        { LivehimePolarisBehaviorEvent::PresetMaterialSkipClick, "live.live-zbj.preset.skip.click" },
        { LivehimePolarisBehaviorEvent::PresetMaterialPrimaryLiveContentClick, "live.live-zbj.preset.primary-live-content.click" },
        { LivehimePolarisBehaviorEvent::PresetMaterialSubLiveContentClick, "live.live-zbj.preset.sub-live-content.click" },
        { LivehimePolarisBehaviorEvent::PresetMaterialGoToStartLiveClick, "live.live-zbj.preset.go-to-start-live.click" },
        { LivehimePolarisBehaviorEvent::SceneHideClick, "live.live-zbj.scene-manage.hide.click" },
        { LivehimePolarisBehaviorEvent::SceneLockClick, "live.live-zbj.scene-manage.lock.click" },
        { LivehimePolarisBehaviorEvent::SceneDeleteClick, "live.live-zbj.scene-manage.delete.click" },
        { LivehimePolarisBehaviorEvent::SceneEditClick, "live.live-zbj.scene-manage.edit.click" },
        { LivehimePolarisBehaviorEvent::BrowserRefreshClick, "live.live-zbj.plug-in-set.refresh.click" },
        { LivehimePolarisBehaviorEvent::BrowserBackgroundClick, "live.live-zbj.plug-in-set.subtract-background.click" },
        { LivehimePolarisBehaviorEvent::BrowserCustomizedClick, "live.live-zbj.plug-in-set.customized-subtract-background.click" },
        { LivehimePolarisBehaviorEvent::LeftToolbarCollapseClick, "live.live-zbj.toolbar-collapse.0.click" },
        { LivehimePolarisBehaviorEvent::PkStreamQuality, "blink.app.live.video.pk.data.v1.track"},
        { LivehimePolarisBehaviorEvent::SingPanelShow, "live.live-zbj.sing-panel.0.show" },
        { LivehimePolarisBehaviorEvent::SingPanelConfirmClick, "live.live-zbj.sing-panel.confirm.click" },
        { LivehimePolarisBehaviorEvent::SingPanelNoMoreClick, "live.live-zbj.sing-panel.no-more.click" },
        { LivehimePolarisBehaviorEvent::StopLiveInterceptWindowShow, "live.live-zbj.close-live-panel.0.show"},
        { LivehimePolarisBehaviorEvent::StopLiveInterceptWindowClick, "live.live-zbj.close-live-panel.0.click" },
        { LivehimePolarisBehaviorEvent::EndLiveDataWindowTaskClick, "live.live-end-zbj.task-card.0.click" },
        { LivehimePolarisBehaviorEvent::VtuberJoinTipDlgShow, "live.live-zbj.virtual-connection.0.show" },
        { LivehimePolarisBehaviorEvent::VertCoverShow, "live.live-zbj.cover-panel.cover.show" },
        { LivehimePolarisBehaviorEvent::VertCoverClick, "live.live-zbj.cover-panel.cover.click" },
        { LivehimePolarisBehaviorEvent::ColivePresetFunctionPresetClick, "live.live-zbj.pk-panel.preset.click"},
        { LivehimePolarisBehaviorEvent::ColivePresetFunctionResetClick, "live.live-zbj.pk-preset.reset.click" },
        { LivehimePolarisBehaviorEvent::ColivePresetFunctionCropSave, "live.live-zbj.pk-preset.save.click" },
        { LivehimePolarisBehaviorEvent::ColivePresetFunctionRecordClick, "live.live-zbj.pk-preset.record.click" },
        { LivehimePolarisBehaviorEvent::ColivePresetFunctionAdviceShow, "live.live-zbj.pk-preset.advice.show" },
        { LivehimePolarisBehaviorEvent::ColivePresetFunctionBtnClick, "live.live-zbj.pk-preset.advice.click" },
        { LivehimePolarisBehaviorEvent::DanmuReadinPanelSwitchClick, "live.live-zbj.danmu-reading-panel.switch.click" },
        { LivehimePolarisBehaviorEvent::DanmuReadingCloseClick, "live.live-zbj.danmu-reading.close.click" },
        { LivehimePolarisBehaviorEvent::DanmuReadingClick, "live.live-zbj.reading-danmu.read.click" },
        { LivehimePolarisBehaviorEvent::DanmuReadingReceiverClick, "live.live-zbj.reading-danmu.receive.click" },
        { LivehimePolarisBehaviorEvent::VoiceBroadcastDelaytimeTrack, "blink-tts.performance.detail-delaytime.track" },
        { LivehimePolarisBehaviorEvent::VoiceBroadcastUsageTrack, "blink-tts.usage.track" },
        { LivehimePolarisBehaviorEvent::BeautyIconClick, "live.live-zbj.filter-panel.icon.click" },
        { LivehimePolarisBehaviorEvent::FilterItemClick, "live.live-zbj.filter-panel.sub-icon.click" },
        { LivehimePolarisBehaviorEvent::StickersViewItemClick, "live.live-zbj.stickers-panel.icon.click" },
        { LivehimePolarisBehaviorEvent::BeautyMaterialTabClick, "live.live-zbj.beauty-panel.tab.click"},
        { LivehimePolarisBehaviorEvent::BeautyMaterialSecondaryTabClick, "live.live-zbj.beauty-panel.sub-tab.click" },
        { LivehimePolarisBehaviorEvent::BeautyMaterialItemClick, "live.live-zbj.beauty-panel.icon.click"},
        { LivehimePolarisBehaviorEvent::AllBeautyParamsSaveClick, "live.live-zbj.beauty-setting.submit.click" },
        { LivehimePolarisBehaviorEvent::LiveQualityInfoReport, "blink.living.performance.track" },
        { LivehimePolarisBehaviorEvent::ThreeVtuberViewShow, "live.live-zbj.unity-3d-download-panel.0.show"},
        { LivehimePolarisBehaviorEvent::TwoVtuberViewShow, "live.live-zbj.unity-2d-download-panel.0.show"},
        { LivehimePolarisBehaviorEvent::StartThreeVtuberClick, "live.live-zbj.unity-3d.start.click" },
        { LivehimePolarisBehaviorEvent::StartTwoVtuberClick, "live.live-zbj.unity-2d.start.click" },
        { LivehimePolarisBehaviorEvent::UnityDownloadResultClick, "live.live-zbj.unity-3d-download-panel.result.click" },
        { LivehimePolarisBehaviorEvent::TwoUnityDownloadResultClick, "live.live-zbj.unity-2d-download-panel.result.click" },
        { LivehimePolarisBehaviorEvent::TwoUnityStartLive, "live.live-zbj.startlive.0.click"},
        { LivehimePolarisBehaviorEvent::PluginSceneClick, "live.live-zbj.plug-in.0.click" },
        { LivehimePolarisBehaviorEvent::VideoConnHistoryClick, "live.live-zbj.connection-panel.history.click" },
        { LivehimePolarisBehaviorEvent::VideoConnInvitationClick, "live.live-zbj.connection-panel.rec-anchor.click" },
        { LivehimePolarisBehaviorEvent::VideoConnWaitingPanelShow, "live.live-zbj.connection-waiting-panel.0.show" },
        { LivehimePolarisBehaviorEvent::VideoConnWaitingPanelCancelClick, "live.live-zbj.connection-waiting-panel.cancel.click" },
        { LivehimePolarisBehaviorEvent::VideoConnInvitationPanelShow, "live.live-zbj.connection-invitation.0.show" },
        { LivehimePolarisBehaviorEvent::VideoConnInvitationPanelClick, "live.live-zbj.connection-invitation.0.click" },
        { LivehimePolarisBehaviorEvent::VideoConnFollowClick, "live.live-zbj.follow.all.click" },
        { LivehimePolarisBehaviorEvent::VideoConnFollowStatusChange, "live.live-zbj.follow.status-change.show" },
        { LivehimePolarisBehaviorEvent::PluginEntranceShow, "live.live-zbj.plug-in-entrance.0.show" },
        { LivehimePolarisBehaviorEvent::PluginSetDlgShow, "live.live-zbj.plug-in-set.0.show" },
        { LivehimePolarisBehaviorEvent::PluginSetSureClick, "live.live-zbj.plug-in-set.submit.click" },
        { LivehimePolarisBehaviorEvent::VoiceLinkPanelShow, "live.live-zbj.voice-join-panel.0.show" },
        { LivehimePolarisBehaviorEvent::VoiceLinkPanelSwitchClick, "live.live-zbj.voice-join-panel.switch.click" },
        { LivehimePolarisBehaviorEvent::VoiceLinkPanelConditionClick, "live.live-zbj.voice-join-panel.condition.click" },
        { LivehimePolarisBehaviorEvent::VoiceLinkWaitingPanelShow, "live.live-zbj.voice-join-waiting-panel.0.show" },
        { LivehimePolarisBehaviorEvent::VoiceLinkwaitingPanelClick, "live.live-zbj.voice-join-waiting-panel.0.click" },
        { LivehimePolarisBehaviorEvent::VoiceLinkCardEndClick, "live.live-zbj.voice-join-card.end.click" },
        { LivehimePolarisBehaviorEvent::VoiceLinkJoinPanelAudienceClick, "live.live-zbj.voice-join-panel.audience.click" },
        { LivehimePolarisBehaviorEvent::ActivityPopupsWindowShow, "live.live-zbj.popups.0.show" },
        { LivehimePolarisBehaviorEvent::ActivityPopupsWindowClick, "live.live-zbj.popups.0.click" },
        { LivehimePolarisBehaviorEvent::VtuberMatchPanelShow, "live.live-zbj.connection-panel.virtual-connection.show" },
        { LivehimePolarisBehaviorEvent::VtuberMatchBtnClick, "live.live-zbj.connection-panel.virtual-connection.click" },
        { LivehimePolarisBehaviorEvent::VtuberConnWaitShow, "live.live-zbj.connection-waiting-time.0.show" },
        { LivehimePolarisBehaviorEvent::PlayTogetherHelpClick, "live.my-live-room-show.play-together-panel.rule.click" },
        { LivehimePolarisBehaviorEvent::StreamingQualityDetailPanelShow, "live.live-zbj.heartbeat-detail.0.show" },
        { LivehimePolarisBehaviorEvent::StreamingQualityDetailPanelHelpClick, "live.live-zbj.heartbeat-detail.help.click" },
        { LivehimePolarisBehaviorEvent::DmjSettingEnterEffectsClick, "live.live-zbj.danmu-setting.enter-effects.click" },
        { LivehimePolarisBehaviorEvent::FaceFffectSwitchClick, "live.live-zbj.danmu-setting.face-special-effects.click" },
        { LivehimePolarisBehaviorEvent::FaceSourceDownLoadTracker, "live.live-zbj.face-source-download.0.track" },
        { LivehimePolarisBehaviorEvent::FaceEffectPlayTracker, "live.live-zbj.face-effect-play.0.track" },
        { LivehimePolarisBehaviorEvent::JumpAgreementTrack, "blink.my-live-camera.jump-agreement.track" },
        { LivehimePolarisBehaviorEvent::EffectSettingTotalSwitch, "live.live-zbj.animation-setting.switch.click" },
        { LivehimePolarisBehaviorEvent::EffectSettingDetialSwitch, "live.live-zbj.animation-setting.set.click" },
		    { LivehimePolarisBehaviorEvent::SettingDowanloadClick, "live.live-zbj.setting.download-seeting.click" },
		    { LivehimePolarisBehaviorEvent::CachePathSetClick, "live.live-zbj.other-cache.alter-path.click" },
        { LivehimePolarisBehaviorEvent::MultiVCRoomShow, "live.live-zbj.multi-connection-room.0.show" },
        { LivehimePolarisBehaviorEvent::MultiVCRoomInviteClick, "live.live-zbj.multi-connection-room.invite.click" },
        { LivehimePolarisBehaviorEvent::MultiVCInvitePanelShow, "live.live-zbj.multi-connection-panel.0.show" },
        { LivehimePolarisBehaviorEvent::MultiVCApplyPanelJoinClick, "live.live-zbj.multi-connection-panel.join.click" },
        { LivehimePolarisBehaviorEvent::MultiVCApplyPanelCancelClick, "live.live-zbj.multi-connection-panel.cancel-application.click" },
        { LivehimePolarisBehaviorEvent::MultiVCInvitePanelInviteClick, "live.live-zbj.multi-connection-panel.invite.click" },
        { LivehimePolarisBehaviorEvent::MultiVCInvitePanelCancelClick, "live.live-zbj.multi-connection-panel.cancel-invitation.click" },
        { LivehimePolarisBehaviorEvent::MultiVCApplyDialogShow, "live.live-zbj.multi-connection-application.0.show" },
		    { LivehimePolarisBehaviorEvent::MultiVCApplyDialogClick, "live.live-zbj.multi-connection-application.0.click" },
		    { LivehimePolarisBehaviorEvent::MultiVCInviteDialogShow, "live.live-zbj.multi-connection-invitation.0.show" },
		    { LivehimePolarisBehaviorEvent::MultiVCInviteDialogClick, "live.live-zbj.multi-connection-invitation.0.click" },
		    { LivehimePolarisBehaviorEvent::MP4EffectAdded, "live.live-zbj.mp4-effect-add.0.track" },
        { LivehimePolarisBehaviorEvent::MP4EffectPlayed, "live.live-zbj.mp4-effect-play.0.track" },
        { LivehimePolarisBehaviorEvent::MP4EffectDXFailed, "live.live-zbj.mp4-effect-dx-failed.0.track" },
        { LivehimePolarisBehaviorEvent::InteractPlayDownload, "live.live-zbj.interact-play-download.0.track" },
        { LivehimePolarisBehaviorEvent::InteractPlayOpen, "live.live-zbj.interact-play-open.0.track" },
        { LivehimePolarisBehaviorEvent::VtuberJoinTrack, "live.live-zbj.vtuber-join.0.track" },
        { LivehimePolarisBehaviorEvent::FigmaUploadTrack, "live.live-zbj.figma_upload.0.track" },
        { LivehimePolarisBehaviorEvent::StartVtuberJoinTrack, "live.live-zbj.start_join_err.0.track" },
        { LivehimePolarisBehaviorEvent::CefJsBridgeCallTrack, "live.live-zbj.cef-jsbridge.call.track" },
        { LivehimePolarisBehaviorEvent::DanmuCoreUserSettingClick, "live.live-zbj.danmu-setting.ruser.click" },
        { LivehimePolarisBehaviorEvent::DanmuCoreUserModuleClick, "live.live-zbj.dmj.ruser-module.click" },
        { LivehimePolarisBehaviorEvent::DanmuCoreUserMessageShow, "live.live-zbj.dmj.ruser-message.show" },
		    { LivehimePolarisBehaviorEvent::VtuberEntranceClick, "live.live-zbj.preview.icon.click" },

        { LivehimePolarisBehaviorEvent::AVlinkChannelBaseInfo, "blink.live.channel_base_info.track" },
		    { LivehimePolarisBehaviorEvent::CDNSendPacketFail, "blink.push.send_failed_3_times.track" },



        { LivehimePolarisBehaviorEvent::QuestionnaireShow, "live.live-zbj.floating-ball.0.show" },
        { LivehimePolarisBehaviorEvent::QuestionnaireClick, "live.live-zbj.floating-ball.0.click" },
        { LivehimePolarisBehaviorEvent::AppUseTimeTracker, "blink.live.apptime.track" },
        { LivehimePolarisBehaviorEvent::CloseLiveTracker, "blink.close.live.tracker" },
        { LivehimePolarisBehaviorEvent::LaunchDurationTracker, "blink.live.launch.duration.track" },

		    { LivehimePolarisBehaviorEvent::MultiVoicePreviewShow, "live.live-zbj.multi-voice-preview.0.show"},
        { LivehimePolarisBehaviorEvent::MultiVoicePreviewClick, "live.live-zbj.multi-voice-preview.0.click"},
        { LivehimePolarisBehaviorEvent::MultiVoiceJoinApplyPanelShow, "live.live-zbj.multi-voice-join-apply-panel.0.show"},
        { LivehimePolarisBehaviorEvent::MultiVoiceApplyPanelInviteClick, "live.live-zbj.multi-voice-join-apply-panel.invite.click" },
        { LivehimePolarisBehaviorEvent::MultiVoiceJoinApplyPanelClick, "live.live-zbj.multi-voice-join-apply-panel.0.click"},
        { LivehimePolarisBehaviorEvent::MultiVoiceJoinInvitePanelShow, "live.my-live-room-show.multi-voice-join-invite-panel.0.show"},
        { LivehimePolarisBehaviorEvent::MultiVoiceJoinInvitePanelClick, "live.my-live-room-show.multi-voice-join-invite-panel.0.click"},
        { LivehimePolarisBehaviorEvent::MulitiVoiceJoinQuitClick, "live.live-zbj.multi-voice-join.quit.click" },
        { LivehimePolarisBehaviorEvent::MulitiVoicePositionClick, "	live.live-zbj.multi-voice-join.wheat-position.click" },
        { LivehimePolarisBehaviorEvent::MulitiVoiceManageClick, "live.live-zbj.multi-voice-join.wheat-manage.click" },
        { LivehimePolarisBehaviorEvent::MulitiVoiceErrorTrack, "live.live-zbj.chat_room_error.0.track" },
        { LivehimePolarisBehaviorEvent::MulitiVoiceClosedTechTrack, "live.live-zbj.chat_room_closed.0.track" },
		    { LivehimePolarisBehaviorEvent::AnchorTaskCenterBubbleShow, "live.anchor-task-center.bubble.0.show" },
        { LivehimePolarisBehaviorEvent::HotRankShow, "live.live-zbj.ranking-icon.0.show" },
        { LivehimePolarisBehaviorEvent::HotRankClick, "live.live-zbj.ranking-icon.0.click" },
        { LivehimePolarisBehaviorEvent::FansRankClick, "live.live-zbj.fans-attend.0.click" },
	    	{ LivehimePolarisBehaviorEvent::DanmuGiftPanelClick, "live.live-zbj.infor-tab.0.click" },
        { LivehimePolarisBehaviorEvent::RTCExceptionInfoTrack, "blink.live.channel_base_live_exception_info.track" },
        { LivehimePolarisBehaviorEvent::DanmuCoreUserTagShow, "live.live-zbj.core-user-tag.0.show" },
		    { LivehimePolarisBehaviorEvent::WebrtcRealtimeTrack, "live.blink.webrtc.realtime.track" },
    };
    
    // 按照event-id来确定这个埋点事件数据什么类型（当前只有 点击、曝光、日志）
    EventCategory DeterminEventCategory(const std::string& event_id)
    {
        std::string::size_type pos = event_id.rfind('.');
        if (pos != std::string::npos && pos < event_id.length() - 1)
        {
            std::string substr = event_id.substr(pos + 1);
            if (stricmp(substr.c_str(), "click") == 0)
            {
                return EventCategory::CLICK;
            }
            else if (stricmp(substr.c_str(), "show") == 0)
            {
                return EventCategory::EXPOSURE;
            }
            else if (stricmp(substr.c_str(), "pv") == 0)
            {
                return EventCategory::PAGEVIEW;
            }
            else if (stricmp(substr.c_str(), "tracker") == 0||
                stricmp(substr.c_str(), "track") == 0)
            {
                return EventCategory::TRACKER;
            }
        }

        NOTREACHED();
        return EventCategory::OTHER;
    }

    EventCategory DeterminEventCategory(LivehimePolarisBehaviorEvent event_id)
    {
        return DeterminEventCategory(kLivehimeBehaviorEventPolarisTable.at(event_id));
    }

    // 埋点常规处理
    using EventTrackingResult = std::tuple<bool>;
    using EventTrackingParser = std::function<EventTrackingResult(ResponseInfo, const std::string&)>;

    EventTrackingResult ParsePolarisEventTrackingResponse(ResponseInfo info, const std::string& data)
    {
        if (info.response_code != 200)
        {
            LOG(WARNING) << "Parse polaris_event_tracking response failure: invalid status!\n"
                << "response info: " << info;
            return EventTrackingResult(false);
        }

        return EventTrackingResult(true);
    }

    // We now do nothing even if the report fails.
    using DefaultHandler = std::function<void(bool)>;
    void HandleReportResponse(bool)
    {
    }

}   // namespace

namespace secret
{
    RequestProxy EventTrackingServiceIntlImpl::ReportLivehimeBehaviorEventPolaris(
        LivehimePolarisBehaviorEvent event_id,
        const base::StringPairs& event_ext)
    {
        secret::LivehimeIntlBehaviorEvent intl_event;
        if (CheckInCommonBehaviorEventTable("LivehimePolarisBehaviorEvent",
            base::IntToString(static_cast<int>(event_id)), intl_event)) {
            return ReportLivehimeIntlBehaviorEvent(intl_event, event_ext);
        }
        else 
        {
            return RequestProxy(nullptr);
        }
       
    }
    RequestProxy EventTrackingServiceIntlImpl::ReportLivehimeBehaviorEventPolaris(
        const std::string& event_id,
        const base::StringPairs& event_ext)
    {
       
        return  ReportLivehimeIntlBehaviorEvent(event_id, event_ext);
    }
    //RequestProxy EventTrackingServiceIntlImpl::ReportLivehimeBehaviorEventPolaris(
    //    const std::string& event_id,
    //    const base::StringPairs& event_ext)
    //{
    //    static AppEvent ae;

    //    static bool common_info_inited = false;

    //    // app_info
    //    auto app_info = ae.mutable_app_info();

    //    if (!common_info_inited)
    //    {
    //        // 冷启动点击时还无法获取mid
    //        if (event_id != "live.live-zbj.startup.0.show") {
    //            common_info_inited = true;
    //        }

    //        app_info->set_app_id(102);  // 产品编号，由数据平台分配，PC直播姬用102
    //        app_info->set_platform(4);  // 平台编号，iphone=1，ipad=2，android=3，wp=4，PC直播姬和wp共用4
    //        app_info->set_buvid(GetSecretCore()->network_info().buvid());

    //        auto& osver = GetSecretCore()->network_info().os_ver();
    //        app_info->set_osver(osver.empty() ? "1" : osver);
    //        app_info->set_fts(base::Time::Now().ToTimeT() * 1000);

    //        // runtime_info
    //        auto runtime_info = ae.mutable_runtime_info();
    //        runtime_info->set_network((RuntimeNetWork)GetSecretCore()->network_info().network_type());
    //        runtime_info->set_oid(base::UTF16ToUTF8(GetSecretCore()->network_info().communications_operator()));
    //        runtime_info->set_version(BililiveContext::Current()->GetExecutableVersionAsASCII());
    //        runtime_info->set_version_code(std::to_string(BililiveContext::Current()->GetExecutableBuildNumber()));

    //        ae.set_log_id(kLivehimeTechLogIDPolaris);
    //        ae.set_page_type(1);

    //        // PC端拿不到或者对PC没有意义的必填字段要给填充值
    //        app_info->set_chid(kRequireValueDummy);
    //        app_info->set_brand(kRequireValueDummy);
    //        app_info->set_model(kRequireValueDummy);
    //        runtime_info->set_logver(kRequireValueDummy);
    //    }

    //    app_info->set_uid(GetSecretCore()->account_info().mid());
    //    ae.set_mid(std::to_string(GetSecretCore()->account_info().mid()));

    //    static int64 sn = 0;
    //    ae.set_event_id(event_id);
    //    ae.set_ctime(base::Time::Now().ToTimeT() * 1000);
    //    ae.set_sn(sn++);
    //    ae.set_sn_gen_time(base::Time::Now().ToTimeT() * 1000);
    //    ae.set_upload_time(base::Time::Now().ToTimeT() * 1000);
    //    ae.set_event_category(DeterminEventCategory(event_id));

    //    auto extended_fields = ae.mutable_extended_fields();
    //    extended_fields->clear();
    //    for (auto& iter : event_ext)
    //    {
    //        (*extended_fields)[iter.first] = iter.second;
    //    }

    //    int64_t uid = DETERMINE_FIELD_VALUE(GetSecretCore()->account_info().mid());
    //    std::string mid = base::Int64ToString(uid);
    //    int64_t room_id = DETERMINE_FIELD_VALUE(GetSecretCore()->user_info().room_id());
    //    int area_id = DETERMINE_FIELD_VALUE(GetSecretCore()->anchor_info().current_area());
    //    int parent_area_id = DETERMINE_FIELD_VALUE(GetSecretCore()->anchor_info().current_parent_area());
    //    int64_t tag_id = DETERMINE_FIELD_VALUE(GetSecretCore()->anchor_info().tag_id());
    //    std::string tag_name = GetSecretCore()->anchor_info().tag_name();
    //    std::string live_key = DETERMINE_FIELD_VALUE_STR(GetSecretCore()->anchor_info().live_key());
    //    std::string sub_session_key = DETERMINE_FIELD_VALUE_STR(GetSecretCore()->anchor_info().sub_session_key());
    //    std::string screen_status = GetSecretCore()->anchor_info().get_land_scape_model() ? "3" : "2";
    //    int identify_status = GetSecretCore()->anchor_info().identify_status();// 实名认证状态：0 申请中，1 已通过，2 驳回，3 未申请

    //    (*extended_fields)["room_id"] = base::Int64ToString(room_id);
    //    (*extended_fields)["live_status"] = LiveStatus(GetSecretCore()->anchor_info().live_status());
    //    (*extended_fields)["parent_area_id"] = base::IntToString(parent_area_id);
    //    (*extended_fields)["area_id"] = base::IntToString(area_id);
    //    (*extended_fields)["up_id"] = mid;
    //    (*extended_fields)["live_key"] = live_key;
    //    (*extended_fields)["sub_session_key"] = sub_session_key;
    //    (*extended_fields)["screen_status"] = screen_status;
    //    (*extended_fields)["tag_name"] = tag_name;
    //    (*extended_fields)["tag_id"] = base::Int64ToString(tag_id);

    //    // 对于 live.live-zbj.startup.0.show，北极星那边没录入这个参数
    //    if (event_id != "live.live-zbj.startup.0.show") {
    //        (*extended_fields)["is_certify"] = base::IntToString(identify_status);
    //    }

    //    EventList el;
    //    Event* evt = el.add_events();

    //    if (ae.event_category() == EventCategory::TRACKER) {
    //        evt->set_logid(kLivehimeTechLogIDPolaris);
    //    }
    //    else
    //    {
    //        evt->set_logid(kLivehimeLogIDPolaris);
    //    }

    //    evt->set_eventid(event_id);
    //    evt->set_body(ae.SerializeAsString());

    //    std::string content = el.SerializeAsString();

    //    RequestHeaders header;
    //    //header["Content-Encoding"] = "gzip";
    //    //header[net::HttpRequestHeaders::kContentType] = "application/octet-stream";
    //    //header[net::HttpRequestHeaders::kContentLength] = std::to_string(content.length());

    //    RequestRaw body(content);

    //    //RequestUpload upload_body("livehime-evnet-report", content);
    //    //upload_body.mime_type = "application/octet-stream";
    //    //upload_body.extra_params[kArgBFSUploadBucket] = kBFSUploadBucket;

    //    RequestParams params;
    //    AppendCSRF(params);
    //    GURL gurl = AppendQueryStringToGURL(GURL(kURLLancerDataCenter),
    //        params.ToRequestContent().second);

    //    RequestProxy proxy =
    //        conn_manager_->NewRequestConnection<EventTrackingParser, DefaultHandler>(
    //            gurl,
    //            RequestType::POST,
    //            header,
    //            body,
    //            ParsePolarisEventTrackingResponse,
    //            HandleReportResponse);

    //    return proxy;
    //}

}   // namespace secret
