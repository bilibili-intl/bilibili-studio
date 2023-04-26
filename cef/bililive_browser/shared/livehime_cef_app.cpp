#include "stdafx.h"
#include "livehime_cef_app.h"

namespace
{
    // These flags must match the Chromium values.
    const char kProcessType[] = "type";
    const char kRendererProcess[] = "renderer";
    const char kGpuProcess[] = "gpu-process";
    const char kPPAPIProcess[] = "ppapi";

}  // namespace

LivehimeCefApp::LivehimeCefApp()
{
}

LivehimeCefApp::~LivehimeCefApp()
{
}

LivehimeCefApp::ProcessType LivehimeCefApp::GetProcessType(CefRefPtr<CefCommandLine> command_line)
{
    // The command-line flag won't be specified for the browser process.
    if (!command_line->HasSwitch(kProcessType))
    {
        return BrowserProcess;
    }

    const std::string& process_type = command_line->GetSwitchValue(kProcessType);
    if (process_type == kRendererProcess)
    {
        return RendererProcess;
    }
    else if(process_type == kGpuProcess)
    {
        return GpuProcess;
    }
    else if (process_type == kPPAPIProcess)
    {
        return PPAPIProcess;
    }

    return OtherProcess;
}
