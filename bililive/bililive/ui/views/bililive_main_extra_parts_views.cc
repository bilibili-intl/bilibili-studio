#include "bililive_main_extra_parts_views.h"

#include "base/command_line.h"
#include "base/prefs/pref_service.h"

#include "ui/base/ui_base_switches.h"

#include "bililive/bililive/livehime/common_pref/common_pref_service.h"
#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/ui/views/bililive_views_delegate.h"
#include "bililive/public/bililive/bililive_process.h"

#include "obs/obs_proxy/public/common/pref_names.h"


BililiveMainExtraPartsViews::BililiveMainExtraPartsViews()
{}

void BililiveMainExtraPartsViews::ToolkitInitialized()
{
    //CommandLine::ForCurrentProcess()->AppendSwitch(switches::kDisableDwmComposition);

    if (!views::ViewsDelegate::views_delegate)
    {
        views::ViewsDelegate::views_delegate = new BililiveViewsDelegate;
    }
}

void BililiveMainExtraPartsViews::PostBililiveStart()
{
    CommonPrefService::RegisterBililiveHotkey();
}