#ifndef BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_LABEL_H_
#define BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_LABEL_H_

#include "base/basictypes.h"
#include "ui/views/controls/label.h"
#include "ui/views/controls/image_view.h"

#include "bililive/bililive/ui/views/controls/bililive_theme_common.h"


class BililiveLabel : public views::Label
{
public:
    BililiveLabel();
    explicit BililiveLabel(const string16& text);
    explicit BililiveLabel(const string16& text, const gfx::Font& font);
    virtual ~BililiveLabel(){}

    void SetImage(const gfx::ImageSkia& img);
    void SetImage(const gfx::ImageSkia* image_skia);
    void SetImageSize(const gfx::Size &size);
    void SetTextColor(SkColor clr);
    void SetVerticalAlignment(gfx::VerticalAlignment alignment){ vertical_alignment_ = alignment; }
    void SetPreferredSize(const gfx::Size& size);
    void set_interactive(bool interactive){ interactive_ = interactive; }
    void SetAlpha(U8CPU a);
    void SetLimitText(unsigned int max_words);
    void SetLimitSize(const gfx::Size& size);

    // view
    gfx::Size GetPreferredSize() override;
protected:
    // view
    void Layout() OVERRIDE;
    void PaintText(gfx::Canvas* canvas,
        const string16& text,
        const gfx::Rect& text_bounds,
        int flags) OVERRIDE;

    bool HitTestRect(const gfx::Rect& rect) const {
        return interactive_ ? View::HitTestRect(rect) : false;
    }

private:
    void InitParam();

private:
    views::ImageView* image_;
    gfx::VerticalAlignment vertical_alignment_;
    bool interactive_;
    gfx::Size user_preferred_size_;
    gfx::Size limit_size_;
    SkColor text_color_;
    unsigned int max_words_ = 0;

    DISALLOW_COPY_AND_ASSIGN(BililiveLabel);
};


class BililiveBackgroundLabel : public views::Label
{
public:
    BililiveBackgroundLabel();
    BililiveBackgroundLabel(const SkColor &bg_color,int height = 1);
    // view
    gfx::Size GetPreferredSize() override;
protected:
	// view
	void OnPaintBackground(gfx::Canvas* canvas) override;	
private:
    SkColor bg_color_ = SkColorSetARGB(0x19, 0x00, 0x00, 0x00);
    int height_ = 1;
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_LABEL_H_