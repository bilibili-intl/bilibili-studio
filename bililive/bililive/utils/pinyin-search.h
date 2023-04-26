#ifndef BILILIVE_BILILIVE_LIVEHIME_HOST_CENTER_LIVE_PARTITION_PINYIN_SERARCHER_H_
#define BILILIVE_BILILIVE_LIVEHIME_HOST_CENTER_LIVE_PARTITION_PINYIN_SERARCHER_H_

#include <vector>
#include <string>

class PinYinSearcher
{
public:
    virtual ~PinYinSearcher(){}

    static PinYinSearcher* Build(std::vector<std::wstring*>& strlist);
    virtual std::vector<std::wstring*> Search(std::wstring& s) = 0;
};

#endif
