#pragma once

#include "shared/livehime_cef_app.h"


// Implement application-level callbacks for the browser process.
class LivehimeCefAppOther
    : public LivehimeCefApp
{
public:
    LivehimeCefAppOther();
    virtual ~LivehimeCefAppOther();

private:
    // Include the default reference counting implementation.
    IMPLEMENT_REFCOUNTING(LivehimeCefAppOther);
    DISALLOW_COPY_AND_ASSIGN(LivehimeCefAppOther);
};