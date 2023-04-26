#ifndef BILILIVE_URL_IMG_FETCH_TRANSACTION_H
#define BILILIVE_URL_IMG_FETCH_TRANSACTION_H

#include <string>
#include <vector>

#include "base/callback.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"

namespace gfx {
    class ImageSkia;
}
namespace net {
    class URLFetcher;
    class URLFetcherDelegate;
}
namespace bililive_main_dll {
    class UrlImgFethcerDelegate;
}


class BililiveUrlImgFetchTransaction final 
        : public base::SupportsWeakPtr<BililiveUrlImgFetchTransaction> {
public:
    BililiveUrlImgFetchTransaction(
            const std::string &url,
            base::Callback<void(double)> process_cb,
            //base::Callback<void(const gfx::ImageSkia&)> static_img_cb,
            base::Callback<void(scoped_ptr<std::vector<char>>)> dyn_img_cb);

    ~BililiveUrlImgFetchTransaction();

private:
    static void AsyncStartTransaction(
            const std::string &url,
            base::WeakPtr<BililiveUrlImgFetchTransaction> ui_weak_object);
    static void OnStartTranactionFinished(
            base::WeakPtr<BililiveUrlImgFetchTransaction> ui_weak_object,
            net::URLFetcher *url_fetcher,
            bililive_main_dll::UrlImgFethcerDelegate *delegate);

    //static void AsyncOnStaticImageFetched(
    //        base::WeakPtr<BililiveUrlImgFetchTransaction> ui_weak_object,
    //        gfx::ImageSkia image,
    //        const net::URLFetcher *source);
    static void AsyncOnImageDataFetched(
            base::WeakPtr<BililiveUrlImgFetchTransaction> ui_weak_object,
            scoped_ptr<std::vector<char>> data,
            const net::URLFetcher *source);
    static void AsyncOnProcess(
            base::WeakPtr<BililiveUrlImgFetchTransaction> ui_weak_object,
            double process,
            const net::URLFetcher *source);
    

    //void OnStaticImageFetched(gfx::ImageSkia image,
    //                          const net::URLFetcher *source);
    void OnDynImageDataFetched(scoped_ptr<std::vector<char>> data,
                           const net::URLFetcher *source);
    void OnProcess(double process,
                   const net::URLFetcher *source);

    static void ReleaseUrlFetcherObject(
            net::URLFetcher *url_fetcher,
            bililive_main_dll::UrlImgFethcerDelegate *delegate);
    


private:
    /*the two scoped_ptrs followed need constructing manually on other thread*/
    /*scoped_ptr only for not forgetting release ration then releasing*/
    scoped_ptr<net::URLFetcher> url_fetcher_;
    scoped_ptr<bililive_main_dll::UrlImgFethcerDelegate> url_fetcher_delegate_;

    base::Callback<void(double)> process_cb_;
    //base::Callback<void(const gfx::ImageSkia&)> static_img_cb_;
    base::Callback<void(scoped_ptr<std::vector<char>>)> img_data_cb_;

};

#endif