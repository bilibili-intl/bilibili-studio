#ifndef IS_ADMINISTRATOR_REQUIRED_H_
#define IS_ADMINISTRATOR_REQUIRED_H_

#include "base/strings/utf_string_conversions.h"

namespace bililive_utils
{
    int GetProcessIdByName(const wchar_t* processName);
    bool CheckIfSucceedToOpenProcess(int pid);
    void KillProcessByPid(int pid);
    bool CheckProcessIsOpen(int pid,base::string16& process_name,bool check_name = false);
    bool CheckPluginProcess(int timer);
};

#endif
