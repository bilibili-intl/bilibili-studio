#ifndef BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_TEXT_PROPERTY_CONTRACT_H_
#define BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_TEXT_PROPERTY_CONTRACT_H_

#include "bililive/bililive/livehime/sources_properties/source_public_property_presenter.h"

namespace contracts {

class SourceTextPropertyPresenter
    : public SourcePublicPropertyPresenter
{
public:
    explicit SourceTextPropertyPresenter(obs_proxy::SceneItem* scene_item)
        : SourcePublicPropertyPresenter(scene_item){}

    virtual ~SourceTextPropertyPresenter() {}

    virtual string16 GetText() = 0;
    virtual void SetText(const string16& text) = 0;

    virtual bool GetIsFromFile() = 0;
    virtual void SetIsFromFile(bool value) = 0;

    virtual string16 GetTextFilePath() = 0;
    virtual void SetTextFilePath(const string16& path) = 0;

    virtual string16 GetFontName() = 0;
    virtual void SetFontName(const string16& font) = 0;

    virtual string16 GetFontSize() = 0;
    virtual void SetFontSize(const string16& size) = 0;

    virtual int GetFontStyle() = 0;
    virtual void SetFontStyle(int index) = 0;

    virtual SkColor GetFontColor() = 0;
    virtual void SetFontColor(SkColor clr) = 0;

    virtual float GetTransparent() = 0;
    virtual void SetTransparent(float value) = 0;

    virtual float GetHorizontalScrollSpeed() = 0;
    virtual void SetHorizontalScrollSpeed(float value) = 0;

    virtual float GetVerticalScrollSpeed() = 0;
    virtual void SetVerticalScrollSpeed(float value) = 0;

    virtual bool GetOutline() = 0;
    virtual void SetOutline(bool value) = 0;

    virtual float GetOutlineSize() = 0;
    virtual void SetOutlineSize(float value) = 0;

    virtual SkColor GetOutlineColor() = 0;
    virtual void SetOutlineColor(SkColor value) = 0;

    virtual const std::vector<string16>& GetSystemFontFamilies() const = 0;

    virtual void InitFontSizeFromString(
        const string16& font_size_str) = 0;

    virtual const std::vector<string16>& GetInitFontSize() const = 0;

    virtual std::vector<string16> SearchString(const string16& search_text) = 0;
};

}   // namespace contracts

#endif  // BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_TEXT_PROPERTY_CONTRACT_H_