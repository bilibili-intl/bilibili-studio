#pragma once

#include <vector>

#include "base\strings\string16.h"


struct CefCache {
    std::string key;
    base::string16 cef_value;
};

class CefHybridCache {
public:
    CefHybridCache();
    ~CefHybridCache();

    static CefHybridCache* GetInstance();

    bool AddHybridCacheByKey(const std::string& key, const base::string16& value);
    bool RemoveHybridCacheByKey(const std::string& key);
    void ClearHybridCache();
    base::string16 GetHybridCacheByKey(const std::string& key);
private:
    void ReadHybridCache();
    void WriteHybridCache();

    bool ChangeCacheKey(const std::string& old_key,std::string& key);
private:
    std::vector<CefCache> cache_vet_;
};