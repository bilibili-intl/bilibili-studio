#ifndef LIVEHIME_LABEL_H
#define LIVEHIME_LABEL_H

#include "bililive/bililive/ui/views/controls/bililive_label.h"

// TitleLabel
class LivehimeTitleLabel : public BililiveLabel
{
public:
    LivehimeTitleLabel() { InitThema(); }
    explicit LivehimeTitleLabel(const string16& text)
        : BililiveLabel(text)
    {
        InitThema();
    }

    static const gfx::Font& GetFont();

private:
    void InitThema();
};

class LivehimeSmallTitleLabel : public BililiveLabel
{
public:
    LivehimeSmallTitleLabel() { InitThema(); }
    explicit LivehimeSmallTitleLabel(const string16& text)
        : BililiveLabel(text)
    {
        InitThema();
    }

private:
    void InitThema();
};


// ContentLabel
class LivehimeContentLabel : public BililiveLabel
{
public:
    LivehimeContentLabel() { InitThema(); }
    explicit LivehimeContentLabel(const string16& text)
        : BililiveLabel(text)
    {
        InitThema();
    }

    static const gfx::Font& GetFont();

private:
    void InitThema();
};

class LivehimeSmallContentLabel : public BililiveLabel
{
public:
    LivehimeSmallContentLabel() { InitThema(); }
    explicit LivehimeSmallContentLabel(const string16& text)
        : BililiveLabel(text)
    {
        InitThema();
    }

private:
    void InitThema();
};


// TipLabel
class LivehimeTipLabel : public BililiveLabel
{
public:
    LivehimeTipLabel();
    explicit LivehimeTipLabel(const string16& text);
    explicit LivehimeTipLabel(const string16& text, SkColor clr);

    static const gfx::Font& GetFont();

private:
    void InitParam();
};

class SplitLineHorizontalLabel : public views::Label
{
public:
    SplitLineHorizontalLabel();
    void SetBackgroundColor(SkColor bg_color) {bg_color_ = bg_color;}
protected:
	// view
	void OnPaintBackground(gfx::Canvas* canvas) override;
	gfx::Size GetPreferredSize() override;
private:
    SkColor bg_color_;
};

class LivehimeStyleLabel : public BililiveLabel
{
public:
    enum class LabelStyle
    {
        Style_Title,
        Style_Content,
        Style_Content12,
        Style_Notes,
        Style_Warning,
    };
    explicit LivehimeStyleLabel(LabelStyle style, const string16& text);
};

#endif
