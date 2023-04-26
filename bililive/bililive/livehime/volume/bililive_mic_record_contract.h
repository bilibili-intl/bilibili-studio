#ifndef BILILIVE_BILILIVE_LIVEHIME_VOLUME_BILILIVE_MIC_RECORD_CONTRACT_H_
#define BILILIVE_BILILIVE_LIVEHIME_VOLUME_BILILIVE_MIC_RECORD_CONTRACT_H_
namespace contracts
{
    class BililiveMicRecordContract
    {
    public:
        virtual ~BililiveMicRecordContract() = default;
        virtual bool StartRecord() = 0;
        virtual bool StopRecord() = 0;
        virtual bool Replay() = 0;
    };
}
#endif  // BILILIVE_BILILIVE_LIVEHIME_VOLUME_BILILIVE_MIC_RECORD_CONTRACT_H_