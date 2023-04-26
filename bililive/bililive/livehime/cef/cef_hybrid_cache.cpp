#include "cef_hybrid_cache.h"

#include "base\memory\singleton.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/prefs/pref_service.h"
#include "base/strings/utf_string_conversions.h"

#include "bililive/bililive/livehime/common_pref/common_pref_names.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/common/pref_names.h"


CefHybridCache::CefHybridCache()
{
    ReadHybridCache();
}

CefHybridCache::~CefHybridCache()
{
}

CefHybridCache* CefHybridCache::GetInstance()
{
    return Singleton<CefHybridCache>::get();
}

void CefHybridCache::ReadHybridCache()
{
    PrefService* pref = GetBililiveProcess()->profile()->GetPrefs();
    auto root_dict = pref->GetDictionary(prefs::kLiveCefCache);
    if (!root_dict) {
        return;
    }

    for (auto it = root_dict->begin(); it != root_dict->end(); it++) {
        CefCache cef_cache;
        cef_cache.key = it->first;
        auto value = it->second;
        value->GetAsString(&cef_cache.cef_value);
        cache_vet_.push_back(std::move(cef_cache));
    }
}

void CefHybridCache::WriteHybridCache()
{
    std::unique_ptr<base::DictionaryValue> root(new base::DictionaryValue());
    for (auto it = cache_vet_.begin(); it != cache_vet_.end(); it++) {
        root->SetString(it->key,it->cef_value);
    }

    auto pref = GetBililiveProcess()->profile()->GetPrefs();
    pref->Set(prefs::kLiveCefCache, *root.get());
}

bool CefHybridCache::AddHybridCacheByKey(const std::string& key, const base::string16& value)
{
    if (key.empty() || value.empty()) {
        return false;
    }

    std::string key_tmp;
    if (!ChangeCacheKey(key, key_tmp)) {
        return false;
    }

    bool change = true;
    auto it = std::find_if(cache_vet_.begin(), cache_vet_.end(), [key_tmp](const CefCache& cef_cache) {
        return cef_cache.key == key_tmp;
    });
    if (it != cache_vet_.end()) {
        if (it->cef_value == value) {
            change = false;
        }
        else {
            it->cef_value = value;
        }
    }
    else {
        CefCache cef_cache;
        cef_cache.key = key_tmp;
        cef_cache.cef_value = value;
        cache_vet_.push_back(std::move(cef_cache));
    }

    if (change) {
        WriteHybridCache();
    }

    return true;
}

bool CefHybridCache::RemoveHybridCacheByKey(const std::string& key)
{
    std::string key_tmp;
    if (!ChangeCacheKey(key, key_tmp)) {
        return false;
    }

    auto it = std::find_if(cache_vet_.begin(), cache_vet_.end(), [key_tmp](const CefCache& cef_cache) {
        return cef_cache.key == key_tmp;
    });
    if (it != cache_vet_.end()) {
        cache_vet_.erase(it);
        WriteHybridCache();
    }

    return true;
}

base::string16 CefHybridCache::GetHybridCacheByKey(const std::string& key)
{
    std::string key_tmp;
    if (!ChangeCacheKey(key, key_tmp)) {
        return L"";
    }

    auto it = std::find_if(cache_vet_.begin(), cache_vet_.end(), [key_tmp](const CefCache& cef_cache) {
        return cef_cache.key == key_tmp;
    });
    if (it != cache_vet_.end()) {
        return it->cef_value;
    }

    return L"";
}

bool CefHybridCache::ChangeCacheKey(const std::string& old_key, std::string& key)
{
    if (old_key.empty()) {
        return false;
    }

    key = old_key;
    if (key.find(".") != std::string::npos) {
        std::replace(key.begin(), key.end(), '.', '_');
    }

    return true;
}

void CefHybridCache::ClearHybridCache()
{
    if (!cache_vet_.empty()) {
        cache_vet_.clear();
        WriteHybridCache();
    }
}