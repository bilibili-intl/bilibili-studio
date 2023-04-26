#ifndef BILILIVE_BILILIVE_LIVEHIME_MAIN_VIEW_LIVEHIME_MAIN_CLOSE_CONTRACT_H_
#define BILILIVE_BILILIVE_LIVEHIME_MAIN_VIEW_LIVEHIME_MAIN_CLOSE_CONTRACT_H_

namespace contracts {

class LivehimeClosePresenter {
public:
    virtual ~LivehimeClosePresenter() {}

    virtual int GetExitStyle() = 0;

    virtual bool GetIsRemember() = 0;

    virtual void SaveExitStyle(int exit_style) = 0;

    virtual void SaveIsRemember(bool is_remember) = 0;
};

}   // namespace contracts

#endif  // BILILIVE_BILILIVE_LIVEHIME_MAIN_VIEW_LIVEHIME_MAIN_CLOSE_CONTRACT_H_