#ifndef BILILIVE_BILILIVE_LIVEHIME_GIFT_IMAGE_IMAGE_FETCHER_ASYNC_H_
#define BILILIVE_BILILIVE_LIVEHIME_GIFT_IMAGE_IMAGE_FETCHER_ASYNC_H_

#include "base/memory/weak_ptr.h"


class ImageFetcherAsyncDelegate;

class ImageFetcherAsync {
public:
    struct GiftInfo {
        int type;
        bool is_gold;
        std::string icon_data;
    };

    explicit ImageFetcherAsync(ImageFetcherAsyncDelegate* d);

    void RequestGiftInfo(int64_t gift_id);

    void Reset();

private:
    static GiftInfo GetGiftInfoSync(int64_t gift_id);

    void OnFetchGiftConfigInfo(const GiftInfo& info);

    ImageFetcherAsyncDelegate* delegate_;
    base::WeakPtrFactory<ImageFetcherAsync> weakptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(ImageFetcherAsync);
};


class ImageFetcherAsyncDelegate {
public:
    virtual ~ImageFetcherAsyncDelegate() = default;

    virtual void OnGetGiftInfo(const ImageFetcherAsync::GiftInfo& info) = 0;
};

#endif  // BILILIVE_BILILIVE_LIVEHIME_GIFT_IMAGE_IMAGE_FETCHER_ASYNC_H_