/******************************************************************************
 copy from obs-browser
 ******************************************************************************/

#pragma once

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif



#include <include/cef_app.h>
#include <include/cef_base.h>
#include <include/cef_task.h>
#include <include/cef_client.h>
#include <include/cef_parser.h>
#include <include/cef_scheme.h>
#include <include/cef_version.h>
#include <include/cef_render_process_handler.h>
#include <include/cef_request_context_handler.h>

#if CHROME_VERSION_BUILD < 3507
#define ENABLE_WASHIDDEN 1
#else
#define ENABLE_WASHIDDEN 0
#endif

#if CHROME_VERSION_BUILD >= 3770
#define SendBrowserProcessMessage(browser, pid, msg) \
	browser->GetMainFrame()->SendProcessMessage(pid, msg);
#else
#define SendBrowserProcessMessage(browser, pid, msg) \
	browser->SendProcessMessage(pid, msg);
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#else
#pragma GCC diagnostic pop
#endif
