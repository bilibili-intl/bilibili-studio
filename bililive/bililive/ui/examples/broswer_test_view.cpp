#include "broswer_test_view.h"

BroswerTestView::BroswerTestView() :
   LivehimeWebBrowserViewOSR(cef_proxy::client_handler_type::unspecified,
        { 0, 0, 500, 500 },
        60,
        "https://www.baidu.com",
        this)
{
}

BroswerTestView::~BroswerTestView()
{
}
