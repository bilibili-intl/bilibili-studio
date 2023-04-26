#include "bililive/bililive/ui/url_image_view/bililive_url_fetcher_delegate.h"

#include <string>

#include "base/bind.h"
#include "base/message_loop/message_loop.h"
#include "base/message_loop/message_loop_proxy.h"
#include "base/threading/sequenced_worker_pool.h"

#include "base/threading/thread.h"

#include "net/url_request/url_fetcher.h"

#include "net/url_request/url_request_status.h"

#include "ui/gfx/image/image_skia.h"

#include "bililive/bililive/ui/url_image_view/bililive_url_img_fetch_globals.h"
#include "bililive/bililive/utils/bililive_image_util.h"


namespace bililive_main_dll {
    static void MakeImageTask(
            const net::URLFetcher* source,
            scoped_ptr<std::vector<char>> download_data,
            base::Callback <void(gfx::ImageSkia,
                            const net::URLFetcher* source)> s_img_cb,
            scoped_refptr<base::MessageLoopProxy> cb_loop)
    {

        unsigned char* data_p
                = reinterpret_cast<unsigned char*>(download_data->data());
        size_t data_s = download_data->size();

        gfx::ImageSkia image = ::bililive::MakeSkiaImage(
                data_p, data_s);

        cb_loop->PostTask(FROM_HERE, base::Bind(s_img_cb, image, source));
    }

    UrlImgFethcerDelegate::~UrlImgFethcerDelegate()
    {
        CHECK(::bililive_main_dll::GetUrlImgFetchMainThreadProxy()
                ->BelongsToCurrentThread());
    }

    UrlImgFethcerDelegate::UrlImgFethcerDelegate(
            base::Callback<void(double,
                                const net::URLFetcher* source)> process_cb,
            /*base::Callback<void(gfx::ImageSkia,
                                const net::URLFetcher* source)> s_cb,*/
            base::Callback <
                    void(scoped_ptr<std::vector<char>>,
                         const net::URLFetcher* source)> img_cb)
            : process_cb_(process_cb),
            //static_img_finish_cb_(s_cb),
            img_data_cb_(img_cb)
    {
        DCHECK(!img_data_cb_.is_null());
        //DCHECK(!static_img_finish_cb_.is_null());

        DCHECK(::bililive_main_dll::GetUrlImgFetchMainThreadProxy()
                ->BelongsToCurrentThread());
    }

    void UrlImgFethcerDelegate::OnURLFetchComplete(
            const net::URLFetcher* source)
    {
        DCHECK(::bililive_main_dll::GetUrlImgFetchMainThreadProxy()
                ->BelongsToCurrentThread());

        net::URLRequestStatus status = source->GetStatus();
        if (status.status() != net::URLRequestStatus::SUCCESS)
        {
            //base::MessageLoopProxy::current()->PostTask(
            //        FROM_HERE,
            //        base::Bind(static_img_finish_cb_, gfx::ImageSkia(), source));

            download_data_.reset();
            base::MessageLoopProxy::current()->PostTask(
                    FROM_HERE,
                    base::Bind(img_data_cb_, base::Passed(&download_data_), source));

            return;
        }

        if (!img_data_cb_.is_null()) {
            img_data_cb_.Run(download_data_.Pass(), source);
        }

        //unsigned char* data_p
        //        = reinterpret_cast<unsigned char*>(download_data_->data());
        //size_t data_s = download_data_->size();
        //if (::bililive_secret::ImageFormat::GIF
        //    == ::bililive_secret::ImageDataFormat(data_p, data_s)) {

        //    dyn_img_finish_cb_.Run(download_data_.Pass(), source);

        //    return;
        //}

        //::bililive_main_dll::GetUrlImgFetcherDelegateTasksThreadPool()
        //        ->PostTask(FROM_HERE,
        //                   base::Bind(MakeImageTask,
        //                              source,
        //                              base::Passed(&download_data_),
        //                              static_img_finish_cb_,
        //                              dyn_img_finish_cb_,
        //                              base::MessageLoopProxy::current()));
    }

    void UrlImgFethcerDelegate::OnURLFetchDownloadProgress(
            const net::URLFetcher* source,
            int64 current, int64 total)
    {
        CHECK(::bililive_main_dll::GetUrlImgFetchMainThreadProxy()
                ->BelongsToCurrentThread());
        if (total <= 0)
            return;

        if (!download_data_) {
            download_data_.reset(new std::vector<char>());
            download_data_->reserve(total);
        } else if (static_cast<int64>(download_data_->capacity()) < total) {
            download_data_->reserve(total);
        }

        if (process_cb_.is_null())
            return;

        double ratio
                = static_cast<double>(current) / static_cast<double>(total);
        if (1 >= ratio)
            return;

        process_cb_.Run(ratio, source);
    }

    void UrlImgFethcerDelegate::OnURLFetchDownloadData(
            const net::URLFetcher* source,
            scoped_ptr<std::string> download_data)
    {
        DCHECK(::bililive_main_dll::GetUrlImgFetchMainThreadProxy()
                ->BelongsToCurrentThread());

        if (!download_data_.get())
            download_data_.reset(new std::vector<char>(
                    download_data->begin(), download_data->end()));
        else
            download_data_->insert(download_data_->end(),
                                   download_data->begin(),
                                   download_data->end());

    }

    bool UrlImgFethcerDelegate::ShouldSendDownloadData()
    {
        return true;
    }

    void UrlImgFethcerDelegate::OnURLFetchUploadProgress(
            const net::URLFetcher* source,
            int64 current, int64 total)
    {
        CHECK(::bililive_main_dll::GetUrlImgFetchMainThreadProxy()
                ->BelongsToCurrentThread());
    }
}