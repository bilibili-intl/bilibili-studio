#ifndef BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_RECEIVER_PROPERTY_CONTRACT_H_
#define BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_RECEIVER_PROPERTY_CONTRACT_H_

#include "bililive/bililive/livehime/obs/srcprop_ui_util.h"
#include "bililive/bililive/livehime/sources_properties/source_public_property_presenter.h"


namespace contracts {

    class SourceReceiverPropertyPresenter
        :public SourcePublicPropertyPresenter
    {
    public:
        explicit SourceReceiverPropertyPresenter(obs_proxy::SceneItem* scene_item)
            : SourcePublicPropertyPresenter(scene_item){}

        ~SourceReceiverPropertyPresenter() {}

        virtual bool Initialize() = 0;

        virtual int GetRtmpPort() = 0;

        virtual std::string GetEncoderInfo() = 0;

        virtual float GetVolumeValue() = 0;

        virtual void SetVolumeValue(float value) = 0;

        virtual bool IsMuted() = 0;

        virtual void SetMuted(bool muted) = 0;

        //用来记录是使用电脑还是手机麦克风
        virtual bool IsComputerMic() = 0;

        virtual void SeComputerMic(bool muted) = 0;

        virtual void SetLandscapeModel(bool is_landscape_model) = 0;

        virtual void SetRecvType(ReceiverItemHelper::ReceiverType type) = 0;

        virtual ReceiverItemHelper::ReceiverType GetRecvType() = 0;

        //恢复IOS有线投屏重连信号
        virtual void ReconnectIOSSignal() = 0;
     };

    class SourceReceiverCallback
    {
    public:
        virtual ~SourceReceiverCallback() {}

        virtual void ReceiverStartCallback() = 0;

        virtual void ReceiverStatusCallback(bool running) = 0;

        virtual void  ReceiverResetCallback() = 0;

        
    };

}  // namespace contracts

#endif  // BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_RECEIVER_PROPERTY_CONTRACT_H_