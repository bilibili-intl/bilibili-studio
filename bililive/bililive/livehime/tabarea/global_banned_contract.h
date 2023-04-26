#ifndef BILILIVE_BILILIVE_LIVEHIME_TABAREA_GLOBAL_BANNED_CONTRACT_H_
#define BILILIVE_BILILIVE_LIVEHIME_TABAREA_GLOBAL_BANNED_CONTRACT_H_

namespace contracts {

class GlobalBannedPresenter {
public:
    virtual ~GlobalBannedPresenter() {}

    virtual void GetBannedInfo() = 0;

    virtual void GlobalBanned(bool banned, int minute, const std::string& type, int level) = 0; //true ¿ªÆô½ûÑÔ false Í£Ö¹½ûÑÔ
};

class GlobalBannedView {
public:
    virtual ~GlobalBannedView() {}

    virtual void OnBannedInfo(bool valid_response, int code, int second) = 0;

    virtual void OnGlobalBanned(bool banned, bool success) = 0; //banned true ¿ªÆô½ûÑÔ false Í£Ö¹½ûÑÔ success true ²Ù×÷³É¹¦ false ²Ù×÷Ê§°Ü
};

}   // namespace contracts

#endif  // BILILIVE_BILILIVE_LIVEHIME_TABAREA_GLOBAL_BANNED_CONTRACT_H_