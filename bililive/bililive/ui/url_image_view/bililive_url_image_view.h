#ifndef bililive_url_image_view_h
#define bililive_url_image_view_h

#include <string>
#include <vector>

#include "base/memory/scoped_ptr.h"

#include "ui/gfx/image/image_skia.h"
#include "ui/views/controls/image_view.h"

#include "bililive/bililive/ui/url_image_view/bililive_url_img_fetch_transaction.h"
#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"


class BililiveUrlImageView : public views::View
{
public:
    typedef base::Callback<BililiveImageButton*(void)> BililiveImageButtonConstructor;

    explicit BililiveUrlImageView(views::ButtonListener* listener, 
        BililiveImageButtonConstructor fn = base::Bind(&BililiveUrlImageView::CreateBililiveImageButton));

    void SetImageUrl(const std::string &url);
    void SetImageUrl(const std::string &url, base::Closure completed);
    void set_default_image(const gfx::ImageSkia &def);

    BililiveImageButton* bililive_image_button() const { return image_button_; }

protected:
    static BililiveImageButton* CreateBililiveImageButton();

    // View
    void OnEnabledChanged() override;

private:
    //void OnFetchStaticImage(const gfx::ImageSkia &image);
    void OnFetchImageData(scoped_ptr<std::vector<char>> data);
    void OnFetchProcess(double process_ratio);

private:
    gfx::ImageSkia default_image_;
    BililiveImageButton *image_button_;
    base::Closure completed_;

    scoped_ptr<BililiveUrlImgFetchTransaction> url_fetcher_transaction_;

    base::WeakPtrFactory<BililiveUrlImageView> weak_factory_;

    DISALLOW_COPY_AND_ASSIGN(BililiveUrlImageView);
};

#endif