#include "bililive/bililive/livehime/main_view/livehime_main_close_presenter_impl.h"

#include "base/prefs/pref_service.h"

#include "bililive/bililive/livehime/main_view/livehime_main_close_pref_constants.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/common/pref_names.h"

LivehimeClosePresenterImpl::LivehimeClosePresenterImpl()
{}

int LivehimeClosePresenterImpl::GetExitStyle()
{
    PrefService* global_prefs = GetBililiveProcess()->global_profile()->GetPrefs();

    return global_prefs->GetInteger(prefs::kApplicationExitMode);
}

bool LivehimeClosePresenterImpl::GetIsRemember()
{
    PrefService* global_prefs = GetBililiveProcess()->global_profile()->GetPrefs();

    return global_prefs->GetBoolean(prefs::kApplicationExitRememberChoice);
}

void LivehimeClosePresenterImpl::SaveExitStyle(int exit_style)
{
    PrefService* global_prefs = GetBililiveProcess()->global_profile()->GetPrefs();

    global_prefs->SetInteger(prefs::kApplicationExitMode, exit_style);
}

void LivehimeClosePresenterImpl::SaveIsRemember(bool is_remember)
{
    PrefService* global_prefs = GetBililiveProcess()->global_profile()->GetPrefs();

    global_prefs->SetBoolean(prefs::kApplicationExitRememberChoice, is_remember);
}
