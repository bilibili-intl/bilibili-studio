#include "bililive/bililive/ui/url_image_view/bililive_url_fetcher_context_getter.h"

#include "base/logging.h"
#include "base/message_loop/message_loop_proxy.h"
#include "base/threading/thread.h"
#include "base/threading/thread.h"

#include "bililive/bililive/ui/url_image_view/bililive_url_img_fetch_globals.h"

namespace bililive_main_dll {
    net::URLRequestContext* UrlRequestcontextGetterImpl::GetURLRequestContext()
    {
        return ::bililive_main_dll::GetUrlImgFetchUrlContext();
    }

    scoped_refptr<base::SingleThreadTaskRunner>
            UrlRequestcontextGetterImpl::GetNetworkTaskRunner() const
    {
        return ::bililive_main_dll::GetUrlImgFetchNetworkThread()
                ->message_loop_proxy();
    }
}