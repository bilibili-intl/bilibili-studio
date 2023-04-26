#include "bililive/bililive/ui/url_image_view/bililive_url_img_fetch_globals.h"

#include "base/memory/ref_counted.h"

#include "base/path_service.h"
#include "base/threading/sequenced_worker_pool.h"
#include "base/threading/thread.h"
#include "base/threading/thread_checker.h"

#include "net/url_request/url_fetcher.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_builder.h"

#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/public/common/bililive_paths.h"

#define MAIN_DLL_IMG_URLFETCHER_DELEGATE_TASKS_THREADS_NUM 4
const base::FilePath::StringType url_cache_dir_name(L"url_cache");

namespace bililive_main_dll {
    static scoped_ptr<::base::Thread> 
            g_urlfetcher_network_thread;
    static ::base::Thread
            *g_urlfetcher_network_thread_value = 0;


    static scoped_ptr<::net::URLRequestContext>
            g_urlfetcher_fetch_context;
    static scoped_refptr<::base::SequencedWorkerPool> 
            g_urlfetcher_fetch_delegate_tasks_thread_pool;

    static void MakeRequestContentOnNetThread(void)
    {
        CHECK(g_urlfetcher_network_thread->message_loop_proxy()
                ->BelongsToCurrentThread());

        ::net::URLRequestContextBuilder builder;
        ::net::URLRequestContextBuilder::HttpCacheParams params;
        

        base::FilePath cache_dir;
        if (PathService::Get(bililive::DIR_USER_DATA, &cache_dir)) {
            params.type = 
                    ::net::URLRequestContextBuilder::HttpCacheParams::DISK;

            cache_dir = cache_dir.Append(url_cache_dir_name);
            params.path = ::base::FilePath(cache_dir);
        } else {
            params.type =
                    ::net::URLRequestContextBuilder::HttpCacheParams::IN_MEMORY;
        }

        
        builder.EnableHttpCache(params);

        g_urlfetcher_fetch_context.reset(builder.Build());
    }

    static void AsyncInit()
    {
        CHECK(::BililiveThread::CurrentlyOn(BililiveThread::IO));

        CHECK(NULL == g_urlfetcher_network_thread.get());
        CHECK(NULL == g_urlfetcher_fetch_context.get());
        CHECK(NULL == g_urlfetcher_fetch_delegate_tasks_thread_pool.get());


        g_urlfetcher_network_thread.reset(
            new ::base::Thread("Main DLL UrlFetcher Network Thread"));
        g_urlfetcher_network_thread_value = g_urlfetcher_network_thread.get();


        ::base::Thread::Options options;
        options.message_loop_type = ::base::MessageLoop::TYPE_IO;

        if (!g_urlfetcher_network_thread->StartWithOptions(options))
        {
            CHECK(false) << "Main DLL UrlFetcher Network Thread Init error";
        }
        g_urlfetcher_network_thread->message_loop()->PostTask(
            FROM_HERE,
            base::Bind(MakeRequestContentOnNetThread));

        g_urlfetcher_fetch_delegate_tasks_thread_pool
            = new ::base::SequencedWorkerPool(
            MAIN_DLL_IMG_URLFETCHER_DELEGATE_TASKS_THREADS_NUM,
            "Main DLL UrlFetcher Delegate Tasks Thread");
    }
    static void AsyncUninit()
    {
        CHECK(::BililiveThread::CurrentlyOn(BililiveThread::IO));

        CHECK(g_urlfetcher_network_thread);
        CHECK(g_urlfetcher_fetch_context) << "supposing that the "
            "thread has completed all tasks had be posted";

        g_urlfetcher_network_thread->message_loop_proxy()->PostTask(
            FROM_HERE,
            ::base::Bind(::net::URLFetcher::CancelAll));

        g_urlfetcher_network_thread->message_loop_proxy()->PostTask(
            FROM_HERE,
            ::base::Bind(&scoped_ptr<::net::URLRequestContext>::reset,
            ::base::Unretained(&g_urlfetcher_fetch_context),
            static_cast<::net::URLRequestContext*>(0)));

        g_urlfetcher_network_thread.reset();
        g_urlfetcher_network_thread_value = 0;
        CHECK(0 == g_urlfetcher_fetch_context.get());

        //g_urlfetcher_main_thread.reset();

        g_urlfetcher_fetch_delegate_tasks_thread_pool->Shutdown();
        g_urlfetcher_fetch_delegate_tasks_thread_pool = NULL;
    }
    
    void UrlImgFetchInit(void)
    {
        CHECK(::BililiveThread::CurrentlyOn(BililiveThread::UI));

        ::BililiveThread::PostTask(::BililiveThread::IO, FROM_HERE,
                base::Bind(AsyncInit));
    }


    scoped_refptr<base::MessageLoopProxy> GetUrlImgFetchMainThreadProxy(void)
    {
        //CHECK(::BililiveThread::CurrentlyOn(BililiveThread::UI));

        //CHECK(g_urlfetcher_main_thread);
        //return g_urlfetcher_main_thread.get();

        return BililiveThread::GetMessageLoopProxyForThread(
                ::BililiveThread::IO);
    }

    ::base::Thread *GetUrlImgFetchNetworkThread(void)
    {
        CHECK(g_urlfetcher_network_thread_value);
        return g_urlfetcher_network_thread_value;
    }
    ::net::URLRequestContext *GetUrlImgFetchUrlContext(void)
    {
        CHECK(g_urlfetcher_network_thread
              && g_urlfetcher_network_thread->thread_id()
                         == base::PlatformThread::CurrentId());

        CHECK(g_urlfetcher_fetch_context);
        return g_urlfetcher_fetch_context.get();
    }
    ::base::SequencedWorkerPool *GetUrlImgFetcherDelegateTasksThreadPool(void)
    {
        CHECK(g_urlfetcher_fetch_delegate_tasks_thread_pool);
        return g_urlfetcher_fetch_delegate_tasks_thread_pool.get();
    }

    void UrlImgFetchUnInit(void)
    {
        CHECK(::BililiveThread::CurrentlyOn(BililiveThread::UI));

        ::BililiveThread::PostTask(::BililiveThread::IO, FROM_HERE,
                base::Bind(AsyncUninit));
    }

}