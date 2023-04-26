#include "bililive/bililive/livehime/gift_image/image_fetcher_async.h"

#include "bililive/bililive/livehime/gift_image/image_fetcher.h"
#include "bililive/public/bililive/bililive_thread.h"


ImageFetcherAsync::ImageFetcherAsync(ImageFetcherAsyncDelegate* d)
    :delegate_(d),
    weakptr_factory_(this) {
}


ImageFetcherAsync::GiftInfo ImageFetcherAsync::GetGiftInfoSync(int64_t gift_id) {
    auto icon = ImageFetcher::GetInstance()->FetchGiftImageOnlyOnMRU(gift_id);
    if (icon.empty()) {
        icon = ImageFetcher::GetInstance()->FetchGiftImageSync(gift_id);
    }

    int type = ImageFetcher::GetInstance()->FetchGiftType(gift_id);
    //auto coin_type = ImageFetcher::GetInstance()->FetchGiftCoinType(gift_id);

    GiftInfo info;
    info.type = type;
    //info.is_gold = (coin_type == GiftCoinType::GCT_GOLD);
    info.icon_data = icon;

    return info;
}

void ImageFetcherAsync::OnFetchGiftConfigInfo(const GiftInfo& info) {
    delegate_->OnGetGiftInfo(info);
}

void ImageFetcherAsync::RequestGiftInfo(int64_t gift_id) {
    // 通过IO线程去取图
    BililiveThread::PostTaskAndReplyWithResult(BililiveThread::IO, FROM_HERE,
        base::Bind(&ImageFetcherAsync::GetGiftInfoSync, gift_id),
        base::Bind(&ImageFetcherAsync::OnFetchGiftConfigInfo, weakptr_factory_.GetWeakPtr()));
}

void ImageFetcherAsync::Reset() {
    weakptr_factory_.InvalidateWeakPtrs();
}