#pragma once

#include "cef_core/include/cef_app.h"


// Implement application-level callbacks for the browser process.
class LivehimeCefApp : public CefApp
{
public:
    LivehimeCefApp();
    virtual ~LivehimeCefApp();

    enum ProcessType
    {
        BrowserProcess,
        RendererProcess,
        GpuProcess,
        PPAPIProcess,
        OtherProcess,// gpu-process/ppapi
    };

    // Determine the process type based on command-line arguments.
    static ProcessType GetProcessType(CefRefPtr<CefCommandLine> command_line);

private:
    // Include the default reference counting implementation.
    DISALLOW_COPY_AND_ASSIGN(LivehimeCefApp);
};