#include "bililive/bililive/ui/url_image_view/bililive_url_image_view.h"

#include "base/bind.h"

#include "ui/views/layout/fill_layout.h"
#include "ui/gfx/image/image.h"

BililiveUrlImageView::BililiveUrlImageView(views::ButtonListener* listener, BililiveImageButtonConstructor fn)
    : weak_factory_(this)
{
    SetLayoutManager(new views::FillLayout());

    image_button_ = fn.Run();
    image_button_->SetButtonListener(listener);
    image_button_->SetStretchPaint(true);
    AddChildView(image_button_);
}

BililiveImageButton* BililiveUrlImageView::CreateBililiveImageButton()
{
    return new BililiveImageButton(NULL);
}

void BililiveUrlImageView::OnEnabledChanged()
{
    image_button_->SetEnabled(enabled());
}

void BililiveUrlImageView::SetImageUrl(const std::string &url, base::Closure completed)
{
    completed_ = completed;
    SetImageUrl(url);
}

void BililiveUrlImageView::SetImageUrl(const std::string &url)
{
    image_button_->SetAllStateImage(&default_image_);
    if (url.empty()) {
        if (url_fetcher_transaction_)
            url_fetcher_transaction_.reset();

        image_button_->SetAllStateImage(&default_image_);
    } else {
        url_fetcher_transaction_.reset(
                new BililiveUrlImgFetchTransaction(
                        url,
                        base::Bind(&BililiveUrlImageView::OnFetchProcess, weak_factory_.GetWeakPtr()),
                        //base::Bind(&BililiveUrlImageView::OnFetchStaticImage, 
                        //           base::Unretained(this)),
                        base::Bind(&BililiveUrlImageView::OnFetchImageData, weak_factory_.GetWeakPtr())
                        )
                        );
    }
}

void BililiveUrlImageView::set_default_image(const gfx::ImageSkia &def)
{
    if (image_button_->GetImage(views::Button::ButtonState::STATE_NORMAL).isNull()) {
        image_button_->SetAllStateImage(&def);
    }

    default_image_ = def;
}

//void BililiveUrlImageView::OnFetchStaticImage(const gfx::ImageSkia &image)
//{
//    
//    if (image.isNull()) {
//        image_view_->SetImage(default_image_);
//    } else {
//        image_view_->SetImage(image);
//        image_view_->SetImageSize(image_view_->size());
//    }
//}

void BililiveUrlImageView::OnFetchImageData(scoped_ptr<std::vector<char>> data)
{
    if (data) {
        if (!data->empty()) {
            std::vector<char> *vct = data.get();
            image_button_->SetImageFromBinary((const unsigned char*)&(*vct)[0], vct->size());
        }
    }

    if (!completed_.is_null())
    {
        completed_.Run();
        completed_.Reset();
    }
}

void BililiveUrlImageView::OnFetchProcess(double process_ratio)
{
    (void)process_ratio;
}