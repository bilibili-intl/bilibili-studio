#ifndef BILILIVE_BILILIVE_UI_BILILIVE_COMMAND_HANDLER_LIVEHIME_H_
#define BILILIVE_BILILIVE_UI_BILILIVE_COMMAND_HANDLER_LIVEHIME_H_

#include "bililive/bililive/command_updater_delegate.h"
#include "bililive/bililive/ui/bililive_obs.h"

namespace bililive {

void EnableSceneHotkey(bool enable);
bool IsSceneHotkeyEnabled();

void ExecuteCommandWithParamsLivehime(BililiveOBS* receiver,
                                      int command,
                                      const CommandParamsDetails& params);
void* PresetMaterialAddSource(const CommandParamsDetails& params);//Ԥ���ز����Դ
void PresetMaterialAddSourceEnd(int result, void* data);//Ԥ���ز����Դ���
}   // namespace bililive

#endif  // BILILIVE_BILILIVE_UI_BILILIVE_COMMAND_HANDLER_LIVEHIME_H_