#include "pinyin-search.h"
#include <cstdint>
#include <cctype>
#include <algorithm>
#include <list>

namespace SoraPinyinTable
{
    extern std::uint64_t s_pyTable[];
};

using SoraPinyinTable::s_pyTable;

class PinYinSearcherImpl : public PinYinSearcher
{
    std::vector<std::wstring*> strlist_;

    struct Index {
        int strIndex;
        int charIndex;

        Index(int si, int ci) : strIndex(si), charIndex(ci) { }
    };

    std::list<Index> indexes_[128];

public:
    PinYinSearcherImpl(std::vector<std::wstring*> strlist)
    {
        strlist_ = std::move(strlist);

        for (int strindex = 0; strindex < static_cast<int>(strlist_.size()); ++strindex)
        {
            std::wstring& str = *strlist_[strindex];
            for (int charindex = 0; charindex < static_cast<int>(str.size()); ++charindex)
            {
                wchar_t ch = str[charindex];

                uint64_t py = s_pyTable[(unsigned short)ch];

                if (py != 0)
                {
                    while (py > 0)
                    {
                        int letter = (py & 0xff);
                        indexes_[letter].emplace_back(Index(strindex, charindex));

                        py >>= 8;
                    }
                }
            }
        }
    }

    virtual ~PinYinSearcherImpl(){}

    std::vector<std::wstring*> Search(std::wstring& s) override
    {
        std::list<Index> cand;

        std::transform(s.begin(), s.end(), s.begin(), &std::tolower);

        if (s.empty())
            return{};

        if (s[0] >= 128)
        {
            std::vector<std::wstring*> result;
            for (auto x : strlist_)
            {
                std::wstring& str = *x;
                if (std::search(str.begin(), str.end(), s.begin(), s.end()) != str.end())
                    result.push_back(x);
            }
            return result;
        }

        cand = indexes_[s[0]];

        int pos = 0;
        while (pos < static_cast<int>(s.size()))
        {
            auto i = cand.begin();
            while (i != cand.end())
            {
                if (strlist_[i->strIndex]->size() - i->charIndex < s.size())
                {
                    i = cand.erase(i);
                    continue;
                }

                wchar_t ch = strlist_[i->strIndex]->at(i->charIndex + pos);
                uint64_t py = s_pyTable[(unsigned short)ch];

                bool matched = false;

                if (py == 0 && s[pos] == ch)
                {
                    matched = true;
                }
                else
                {
                    while (py > 0)
                    {
                        int tocomp = py & 0xff;
                        if (tocomp == s[pos])
                        {
                            matched = true;
                            break;
                        }
                        py >>= 8;
                    }
                }

                if (!matched)
                    i = cand.erase(i);
                else
                    ++i;
            }
            ++pos;
        }

        std::vector<std::wstring*> r;
        for (auto x : cand)
        {
            std::wstring* t = strlist_[x.strIndex];
            if (std::find(r.begin(), r.end(), t) == r.end())
                r.emplace_back(t);
        }

        return r;
    }
};

PinYinSearcher* PinYinSearcher::Build(std::vector<std::wstring*>& strlist)
{
    return new PinYinSearcherImpl(std::move(strlist));
}
