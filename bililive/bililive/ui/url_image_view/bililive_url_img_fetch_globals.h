#ifndef BILILIVE_URL_IMG_FETCH_GLOBALS_H
#define BILILIVE_URL_IMG_FETCH_GLOBALS_H

#include "base/memory/ref_counted.h"



namespace base {
    class MessageLoopProxy;
    class SequencedWorkerPool;
    class Thread;
    class ThreadChecker;
}
namespace net {
    class URLRequestContext;
}

namespace bililive_main_dll {
    void UrlImgFetchInit(void);

    scoped_refptr<base::MessageLoopProxy> GetUrlImgFetchMainThreadProxy(void);
    ::base::Thread *GetUrlImgFetchNetworkThread(void);
    ::net::URLRequestContext *GetUrlImgFetchUrlContext(void);
    ::base::SequencedWorkerPool *GetUrlImgFetcherDelegateTasksThreadPool(void);

    void UrlImgFetchUnInit(void);

    class UrlImgFetchObject
    {
    public:
        UrlImgFetchObject()
        {
            UrlImgFetchInit();
        }
        ~UrlImgFetchObject()
        {
            UrlImgFetchUnInit();
        }
    };
}


#endif