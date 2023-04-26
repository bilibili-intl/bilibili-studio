#ifndef BILILIVE_BILILIVE_LIVEHIME_VOLUME_BILILIVE_MIC_RECORD_PRESENTER_IMPL_H_
#define BILILIVE_BILILIVE_LIVEHIME_VOLUME_BILILIVE_MIC_RECORD_PRESENTER_IMPL_H_

#include "bililive/bililive/livehime/volume/bililive_mic_record_contract.h"
#include "bililive/bililive/livehime/sources_properties/source_media_property_contract.h"
class SourceMediaPropertyPresenterImpl;
class BililiveMicRecordPresenterImpl:
    public contracts::BililiveMicRecordContract,
    public contracts::SourceMediaPropertyView
{
public:
    BililiveMicRecordPresenterImpl();
    ~BililiveMicRecordPresenterImpl();
    bool StartRecord() override;
    bool StopRecord() override;
    bool Replay() override;
    void OnMediaStateChanged(contracts::MediaState state) override;

    static std::wstring GetBililiveEchoTestDirectory();

private:
    std::shared_ptr<SourceMediaPropertyPresenterImpl> media_presenter_;
};


#endif  // BILILIVE_BILILIVE_LIVEHIME_VOLUME_BILILIVE_MIC_RECORD_PRESENTER_IMPL_H_