#ifndef BILILIVE_BILILIVE_BILILIVE_MAIN_EXTRA_PARTS_LIVEHIME_INTL_H_
#define BILILIVE_BILILIVE_BILILIVE_MAIN_EXTRA_PARTS_LIVEHIME_INTL_H_

#include "bililive/public/bililive/bililive_main_extra_parts.h"
#include "bililive/public/secret/bililive_secret.h"


class MainExtraPartsLivehimeIntl : public BililiveMainExtraParts {

public:
    MainExtraPartsLivehimeIntl();

    ~MainExtraPartsLivehimeIntl();

    void PreEarlyInitialization() override;

    void PreProfileInit() override;

    void PostProfileInit() override;

    void PreBililiveStart() override;

    void PostBililiveStart() override;

    void BililiveTearDown() override;

private:
    DISALLOW_COPY_AND_ASSIGN(MainExtraPartsLivehimeIntl);
};

#endif  // BILILIVE_BILILIVE_BILILIVE_MAIN_EXTRA_PARTS_LIVEHIME_INTL_H_
