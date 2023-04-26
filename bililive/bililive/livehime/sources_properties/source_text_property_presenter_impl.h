#ifndef BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_TEXT_PROPERTY_PRESENTER_IMPL_H_
#define BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_TEXT_PROPERTY_PRESENTER_IMPL_H_

#include "bililive/bililive/livehime/sources_properties/source_property_snapshot.h"
#include "bililive/bililive/livehime/sources_properties/source_text_property_contract.h"
#include "bililive/bililive/utils/pinyin-search.h"

class SourceTextPropertyPresenterImpl
    : public contracts::SourceTextPropertyPresenter
{
public:
    enum FontStyle
    {
        FONT_DEFAULT    = 0,
        FONT_BOLD       = 1 << 0,
        FONT_ITALIC     = 1 << 1
    };

    explicit SourceTextPropertyPresenterImpl(obs_proxy::SceneItem* scene_item);

    virtual ~SourceTextPropertyPresenterImpl() {}

    string16 GetText() override;
    void SetText(const string16& text) override;

    bool GetIsFromFile() override;
    void SetIsFromFile(bool value) override;

    string16 GetTextFilePath() override;
    void SetTextFilePath(const string16& path) override;

    string16 GetFontName() override;
    void SetFontName(const string16& font) override;

    string16 GetFontSize() override;
    void SetFontSize(const string16& size) override;

    int GetFontStyle() override;
    void SetFontStyle(int index) override;

    SkColor GetFontColor() override;
    void SetFontColor(SkColor clr) override;

    float GetTransparent() override;
    void SetTransparent(float value) override;

    float GetHorizontalScrollSpeed() override;
    void SetHorizontalScrollSpeed(float value) override;

    float GetVerticalScrollSpeed() override;
    void SetVerticalScrollSpeed(float value) override;

    bool GetOutline() override;
    void SetOutline(bool value) override;

    float GetOutlineSize() override;
    void SetOutlineSize(float value) override;

    SkColor GetOutlineColor() override;
    void SetOutlineColor(SkColor value) override;

    const std::vector<string16>& GetSystemFontFamilies() const override;

    void InitFontSizeFromString(
        const string16& font_size_str) override;

    const std::vector<string16>& GetInitFontSize() const override;

    void Snapshot() override;
    void Restore() override;

    std::vector<string16> SearchString(const string16& search_text) override;
private:
    TextSceneItemHelper text_item_;
    std::unique_ptr<livehime::TextPropertySnapshot> snapshot_;

    std::vector<string16> system_font_families_;
    std::vector<string16> font_size_;
    std::unique_ptr<PinYinSearcher> searcher_;
    DISALLOW_COPY_AND_ASSIGN(SourceTextPropertyPresenterImpl);
};

#endif  // BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_TEXT_PROPERTY_PRESENTER_IMPL_H_