#ifndef BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_RECEIVER_PROPERTY_PRESENTER_IMPL_H_
#define BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_RECEIVER_PROPERTY_PRESENTER_IMPL_H_

#include "bililive/bililive/livehime/sources_properties/source_receiver_property_contract.h"
#include "bililive/bililive/livehime/sources_properties/source_property_snapshot.h"
#include "bililive/bililive/livehime/obs/srcprop_ui_util.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"

#include "obs/obs-studio/libobs/obs.hpp"

class SourceReceiverPropertyPresenterImpl
    : public contracts::SourceReceiverPropertyPresenter
{
public:
    explicit SourceReceiverPropertyPresenterImpl(obs_proxy::SceneItem* scene_item,
        contracts::SourceReceiverCallback* callback = nullptr);

    ~SourceReceiverPropertyPresenterImpl();

    bool Initialize() override;

    int GetRtmpPort() override;

    std::string GetEncoderInfo() override;

    float GetVolumeValue() override;

    void SetVolumeValue(float value) override;

    bool IsMuted() override;

    void SetMuted(bool muted) override;

    //用来记录是使用电脑还是手机麦克风
    bool IsComputerMic() override;

    void SeComputerMic(bool muted) override;

    void Snapshot() override;

    void Restore() override;

    void ReconnectIOSSignal() override;

    static void OnReceiverSignal(void* data, calldata_t* params);
    static void OnIOSReceiverSignal(void* data, calldata_t* params);

    void SetLandscapeModel(bool is_landscape_model)override;

    void SetRecvType(ReceiverItemHelper::ReceiverType type) override;

    ReceiverItemHelper::ReceiverType GetRecvType() override;

private:
    void RegistSignal();

    bool muted_ = false;

    ReceiverItemHelper receiver_scene_item_;

    obs_source_t* source_;

    OBSSignal receiver_signal_;
    OBSSignal ios_receiver_signal_;




    contracts::SourceReceiverCallback* callback_;

    static bool is_first_server_normal_post_ ;
    static bool is_first_connect_success_post_;
    static bool is_first_connect_break_post_;
    std::unique_ptr<livehime::ReceiverPropertySnapshot> snapshot_;
    std::shared_ptr<int> canary_;

    DISALLOW_COPY_AND_ASSIGN(SourceReceiverPropertyPresenterImpl);
};

#endif  // BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_RECEIVER_PROPERTY_PRESENTER_IMPL_H_