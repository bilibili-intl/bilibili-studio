#ifndef BILILIVE_URL_FETCHER_DELEGATE_H
#define BILILIVE_URL_FETCHER_DELEGATE_H

#include <vector>

#include "base/callback.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"


#include "net/url_request/url_fetcher_delegate.h"

namespace gfx {
    class ImageSkia;
}
namespace net {
    class URLFetcher;
}

namespace bililive_main_dll {

    class UrlImgFethcerDelegate 
            : public net::URLFetcherDelegate,
            public ::base::SupportsWeakPtr<UrlImgFethcerDelegate> {
    public:
        explicit UrlImgFethcerDelegate(
                base::Callback<
                        void(double,
                             const net::URLFetcher* source)> process_cb,
                //base::Callback<
                //        void(gfx::ImageSkia, 
                //             const net::URLFetcher* source)> s_img_finish_cb,
                base::Callback <
                        void(scoped_ptr<std::vector<char>>,
                             const net::URLFetcher* source)> img_data_cb);
        ~UrlImgFethcerDelegate();

        void OnURLFetchComplete(const net::URLFetcher* source) override;

        void OnURLFetchDownloadProgress(const net::URLFetcher* source,
                int64 current, int64 total);

        void OnURLFetchDownloadData(const net::URLFetcher* source,
                scoped_ptr<std::string> download_data) override;

        bool ShouldSendDownloadData() override;

        void OnURLFetchUploadProgress(const net::URLFetcher* source,
                int64 current, int64 total) override;

    private:
        scoped_ptr<std::vector<char>> download_data_;

        base::Callback<void(double,
                            const net::URLFetcher* source)> process_cb_;
        //base::Callback <void(gfx::ImageSkia,
        //                     const net::URLFetcher* source)> static_img_finish_cb_;

        base::Callback <void(scoped_ptr<std::vector<char>>,
                             const net::URLFetcher* source)> img_data_cb_;

    };
}


#endif