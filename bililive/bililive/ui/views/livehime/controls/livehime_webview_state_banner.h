#ifndef LIVEHIME_WEBVIEW_STATE_BANNER_H
#define LIVEHIME_WEBVIEW_STATE_BANNER_H

#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/controls/bililive_label.h"
#include "bililive/secret/public/danmaku_hime_service.h"

#include "base/memory/weak_ptr.h"


namespace livehime
{
    enum class WebViewState
    {
        StartLoad,
        LoadFaild,
    };

    void DrawWebViewStateImage(gfx::Canvas *canvas, const gfx::Rect &bounds,
                               WebViewState state, const base::string16 &text);
}


class LivehimeWebViewStateBannerView : public views::View
{
public:
    LivehimeWebViewStateBannerView();
    virtual ~LivehimeWebViewStateBannerView();

    void SetState(livehime::WebViewState state, bool repaint = true);
    void SetStateText(livehime::WebViewState state, const base::string16 &text,
                      bool repaint = false);

    // View
    gfx::Size GetPreferredSize() override;
protected:
    // View
    void OnPaintBackground(gfx::Canvas* canvas) override;

private:
    void CalculateCurrentStateStringWidth();

private:
    std::map<livehime::WebViewState, base::string16> state_texts_;
    livehime::WebViewState state_;
    int state_text_width_;

    DISALLOW_COPY_AND_ASSIGN(LivehimeWebViewStateBannerView);
};




#endif
