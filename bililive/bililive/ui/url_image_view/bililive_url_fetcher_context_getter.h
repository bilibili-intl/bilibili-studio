#ifndef BILILIVE_URL_FETCHER_CONTEXT_GETTER_H
#define BILILIVE_URL_FETCHER_CONTEXT_GETTER_H

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "net/url_request/url_request_context_getter.h"

namespace base {
    class MessageLoopProxy;
    class Thread;
}

namespace bililive_main_dll {
    class UrlRequestcontextGetterImpl : public net::URLRequestContextGetter {
    public:
        net::URLRequestContext* GetURLRequestContext() override;
        scoped_refptr<base::SingleThreadTaskRunner>
                GetNetworkTaskRunner() const override;
    };
}

#endif