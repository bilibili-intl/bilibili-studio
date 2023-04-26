#pragma once

#include "base/prefs/pref_registry_simple.h"
#include "bililive/bililive/livehime/common_pref/common_pref_names.h"

//
//namespace prefs {
//    extern const char kHotkeyMicSwitch[];
//    extern const char kHotkeySysVolSwitch[];
//    extern const char kHotkeyScene1[];
//    extern const char kHotkeyScene2[];
//    extern const char kHotkeyScene3[];
//    extern const char kHotkeyLiveSwitch[];
//    extern const char kHotkeyRecordSwitch[];
//    extern const char kHotkeyClearDanmakuTTSQueue[];
//}
//


class CommonPrefService
{
public:
    CommonPrefService()
    {
    }

    ~CommonPrefService()
    {
    }

    static void RegisterProfilePrefs(PrefRegistrySimple *registry);

    static bool RegisterBililiveHotkey();
    static bool RegisterBililiveHotkey(int id);
    static bool RegisterBililiveHotkey(int id, UINT nModify, UINT nVk);
    static void UnregisterBililiveHotKey(int id);

    static bool IsBililiveHotKey(UINT nModify, UINT nVk);

private:

};
