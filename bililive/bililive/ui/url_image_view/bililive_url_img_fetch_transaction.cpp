#include "bililive/bililive/ui/url_image_view/bililive_url_img_fetch_transaction.h"

#include "base/bind.h"
#include "base/threading/thread.h"

#include "net/url_request/url_fetcher.h"
#include "ui/views/controls/image_view.h"
#include "url/gurl.h"

#include "bililive/public/bililive/bililive_thread.h"

#include "bililive/bililive/ui/url_image_view/bililive_url_fetcher_context_getter.h"
#include "bililive/bililive/ui/url_image_view/bililive_url_fetcher_delegate.h"
#include "bililive/bililive/ui/url_image_view/bililive_url_img_fetch_globals.h"

BililiveUrlImgFetchTransaction::BililiveUrlImgFetchTransaction(
        const std::string &url,
        base::Callback<void(double)> process_cb,
        //base::Callback<void(const gfx::ImageSkia&)> s_img_cb,
        base::Callback<void(scoped_ptr<std::vector<char>>)> img_cb)
        : process_cb_(process_cb),
        //static_img_cb_(s_img_cb),
        img_data_cb_(img_cb)
{
    DCHECK(!img_data_cb_.is_null());

    DCHECK(::BililiveThread::CurrentlyOn(::BililiveThread::UI));

    bililive_main_dll::GetUrlImgFetchMainThreadProxy()
            ->PostTask(FROM_HERE,
                       base::Bind(AsyncStartTransaction,
                                  url,
                                  AsWeakPtr()));
}

BililiveUrlImgFetchTransaction::~BililiveUrlImgFetchTransaction()
{
    DCHECK(::BililiveThread::CurrentlyOn(::BililiveThread::UI));

    if (url_fetcher_) {
        DCHECK(url_fetcher_delegate_);

        bililive_main_dll::GetUrlImgFetchMainThreadProxy()
                        ->PostTask(
                                FROM_HERE,
                                base::Bind(ReleaseUrlFetcherObject,
                                           url_fetcher_.release(),
                                           url_fetcher_delegate_.release()));
    }
}

//static
void BililiveUrlImgFetchTransaction::AsyncStartTransaction(
        const std::string &url,
        base::WeakPtr<BililiveUrlImgFetchTransaction> ui_weak_object)
{
    DCHECK(bililive_main_dll::GetUrlImgFetchMainThreadProxy()
            ->BelongsToCurrentThread());


    bililive_main_dll::UrlImgFethcerDelegate *delegate
            = new ::bililive_main_dll::UrlImgFethcerDelegate(
                    base::Bind(BililiveUrlImgFetchTransaction::AsyncOnProcess,
                               ui_weak_object),
                    //base::Bind(BililiveUrlImgFetchTransaction::AsyncOnStaticImageFetched,
                    //           ui_weak_object),
                    base::Bind(BililiveUrlImgFetchTransaction::AsyncOnImageDataFetched,
                               ui_weak_object));

    net::URLFetcher *url_fetcher = net::URLFetcher::Create(
            GURL(url),
            net::URLFetcher::GET,
            delegate);
    url_fetcher->SetRequestContext(
        new ::bililive_main_dll::UrlRequestcontextGetterImpl);

    url_fetcher->Start();

    BililiveThread::PostTask(BililiveThread::UI, 
                             FROM_HERE,
                             base::Bind(OnStartTranactionFinished,
                                        ui_weak_object,
                                        url_fetcher,
                                        delegate));

}

//static
void BililiveUrlImgFetchTransaction::OnStartTranactionFinished(
        base::WeakPtr<BililiveUrlImgFetchTransaction> ui_weak_object,
        net::URLFetcher *url_fetcher,
        bililive_main_dll::UrlImgFethcerDelegate *delegate)
{
    DCHECK(::BililiveThread::CurrentlyOn(::BililiveThread::UI));

    if (ui_weak_object) {
        ui_weak_object.get()->url_fetcher_.reset(url_fetcher);
        ui_weak_object.get()->url_fetcher_delegate_.reset(delegate);
    } else {
        bililive_main_dll::GetUrlImgFetchMainThreadProxy()
                ->PostTask(
                        FROM_HERE,
                        base::Bind(ReleaseUrlFetcherObject,
                                   url_fetcher,
                                   delegate));
    }
}
//static
//void BililiveUrlImgFetchTransaction::AsyncOnStaticImageFetched(
//        base::WeakPtr<BililiveUrlImgFetchTransaction> ui_weak_object,
//        gfx::ImageSkia image,
//        const net::URLFetcher *source)
//{
//    DCHECK(::bililive_main_dll::GetUrlImgFetchMainThreadProxy()
//            ->BelongsToCurrentThread());
//
//    BililiveThread::PostTask(
//            BililiveThread::UI,
//            FROM_HERE,
//            base::Bind(&BililiveUrlImgFetchTransaction::OnStaticImageFetched,
//                       ui_weak_object,
//                       image,
//                       source));
//}
//static
void BililiveUrlImgFetchTransaction::AsyncOnImageDataFetched(
        base::WeakPtr<BililiveUrlImgFetchTransaction> ui_weak_object,
        scoped_ptr<std::vector<char>> data,
        const net::URLFetcher *source)
{
    DCHECK(::bililive_main_dll::GetUrlImgFetchMainThreadProxy()
            ->BelongsToCurrentThread());

    BililiveThread::PostTask(
            BililiveThread::UI,
            FROM_HERE,
            base::Bind(&BililiveUrlImgFetchTransaction::OnDynImageDataFetched,
                       ui_weak_object,
                       base::Passed(&data),
                       source));
}
//static
void BililiveUrlImgFetchTransaction::AsyncOnProcess(
        base::WeakPtr<BililiveUrlImgFetchTransaction> ui_weak_object,
        double process,
        const net::URLFetcher *source)
{
    DCHECK(::bililive_main_dll::GetUrlImgFetchMainThreadProxy()
            ->BelongsToCurrentThread());

    BililiveThread::PostTask(
            BililiveThread::UI,
            FROM_HERE,
            base::Bind(&BililiveUrlImgFetchTransaction::OnProcess,
                       ui_weak_object,
                       process,
                       source));
}

//void BililiveUrlImgFetchTransaction::OnStaticImageFetched(
//        gfx::ImageSkia image,
//        const net::URLFetcher *source)
//{
//    DCHECK(::BililiveThread::CurrentlyOn(::BililiveThread::UI));
//
//    DCHECK(source == url_fetcher_.get());
//    
//    static_img_cb_.Run(image);
//}

void BililiveUrlImgFetchTransaction::OnDynImageDataFetched(
        scoped_ptr<std::vector<char>> data,
        const net::URLFetcher *source)
{
    DCHECK(::BililiveThread::CurrentlyOn(::BililiveThread::UI));

    DCHECK(source == url_fetcher_.get());

    if (!img_data_cb_.is_null()) {
        img_data_cb_.Run(data.Pass());
    }
}

void BililiveUrlImgFetchTransaction::OnProcess(
        double process,
        const net::URLFetcher *source)
{
    DCHECK(::BililiveThread::CurrentlyOn(::BililiveThread::UI));

    DCHECK(source == url_fetcher_.get());

    if (!process_cb_.is_null()) {
        process_cb_.Run(process);
    }
}

//static
void BililiveUrlImgFetchTransaction::ReleaseUrlFetcherObject(
        net::URLFetcher *url_fetcher,
        bililive_main_dll::UrlImgFethcerDelegate *delegate)
{
    DCHECK(bililive_main_dll::GetUrlImgFetchMainThreadProxy()
            ->BelongsToCurrentThread());

    delete url_fetcher;
    delete delegate;
}
