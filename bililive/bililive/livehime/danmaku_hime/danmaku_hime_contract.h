#ifndef BILILIVE_BILILIVE_LIVEHIME_DANMAKU_HIME_DANMAKU_HIME_CONTRACT_H_
#define BILILIVE_BILILIVE_LIVEHIME_DANMAKU_HIME_DANMAKU_HIME_CONTRACT_H_

#include "bililive/secret/public/danmaku_hime_service.h"


namespace contracts {

    class DanmakuHimePresenter {
    public:
        //using DmkTitleItem = secret::DanmakuHimeService::DmkTitleItem;

        virtual ~DanmakuHimePresenter() = default;

        virtual bool IsInitialized() = 0;

        //virtual bool RequestGiftConfigListSync() = 0;

        //virtual void RequestMedalConfigListASync(secret::DanmakuHimeService::GetMedalConfigHandler handler) = 0;

        virtual std::string RequestImageSync(const std::string& url) = 0;

        //virtual bool RequestTitleListSync() = 0;

        //virtual const secret::DanmakuHimeService::GiftConfig& GetGiftConfig() = 0;

        //virtual const secret::DanmakuHimeService::EffectConfigInfo& GetEffectConfig() = 0;

        //virtual const std::vector<DmkTitleItem>& GetTitleList() = 0;
    };

}

#endif  // BILILIVE_BILILIVE_LIVEHIME_DANMAKU_HIME_DANMAKU_HIME_CONTRACT_H_