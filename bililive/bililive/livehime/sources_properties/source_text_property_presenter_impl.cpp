#include "bililive/bililive/livehime/sources_properties/source_text_property_presenter_impl.h"

#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"

#include "bilibase/scope_guard.h"

namespace
{

const int kScrollSpeedMin = 0;
const int kScrollSpeedMax = 300;

const int kOutlineSizeMin = 1;
const int kOutlineSizeMax = 20;

int CALLBACK EnumFontNameProc(LOGFONT *log_font, TEXTMETRIC* /*ptm*/, int font_type, LPARAM lparam)
{
    std::vector<string16>* font_name_families = reinterpret_cast<std::vector<string16>*>(lparam);

    DCHECK(font_name_families);

    if (font_type & TRUETYPE_FONTTYPE &&
        !StartsWith(log_font->lfFaceName, L"@", true))
    {
        font_name_families->push_back(log_font->lfFaceName);
    }

    return 1;
}

}   // namespace

using namespace contracts;

SourceTextPropertyPresenterImpl::SourceTextPropertyPresenterImpl(
    obs_proxy::SceneItem* scene_item)
    : contracts::SourceTextPropertyPresenter(scene_item),
      text_item_(scene_item)
{
    HDC hdc = ::GetDC(::GetDesktopWindow());
    ::EnumFontFamilies(
        hdc,
        nullptr,
        reinterpret_cast<FONTENUMPROC>(EnumFontNameProc),
        reinterpret_cast<LPARAM>(&system_font_families_));
    std::vector<std::wstring*> candptrlist;
    for (auto font_it = system_font_families_.begin(); font_it != system_font_families_.end(); font_it++)
    {
        std::wstring *p = &(*font_it);
        candptrlist.push_back(p);
    }
    searcher_.reset(PinYinSearcher::Build(candptrlist));
    ON_SCOPE_EXIT
    {
        ::ReleaseDC(::GetDesktopWindow(), hdc);
    };
}

string16 SourceTextPropertyPresenterImpl::GetText()
{
    return text_item_.Text();
}

void SourceTextPropertyPresenterImpl::SetText(const string16& text)
{
    text_item_.Text(text);
}

bool SourceTextPropertyPresenterImpl::GetIsFromFile()
{
    return text_item_.IsFromFile();
}

void SourceTextPropertyPresenterImpl::SetIsFromFile(bool value)
{
    text_item_.IsFromFile(value);
}

string16 SourceTextPropertyPresenterImpl::GetTextFilePath()
{
    return text_item_.TextFile();
}

void SourceTextPropertyPresenterImpl::SetTextFilePath(
    const string16& path)
{
    text_item_.TextFile(path);
}

string16 SourceTextPropertyPresenterImpl::GetFontName()
{
    return text_item_.FontName();
}

void SourceTextPropertyPresenterImpl::SetFontName(
    const string16& font)
{
    text_item_.FontName(font);
}

string16 SourceTextPropertyPresenterImpl::GetFontSize()
{
    return base::StringPrintf(L"%d", text_item_.FontSize());
}

void SourceTextPropertyPresenterImpl::SetFontSize(
    const string16& size)
{
    int font_height = -1;
    base::StringToInt(size, &font_height);

    text_item_.FontSize(font_height);
}

int SourceTextPropertyPresenterImpl::GetFontStyle()
{
    FontStyle font_bold = FontStyle::FONT_DEFAULT;
    FontStyle font_italic = FontStyle::FONT_DEFAULT;

    if (text_item_.IsBold())
    {
        font_bold = FontStyle::FONT_BOLD;
    }

    if (text_item_.IsItalic())
    {
        font_italic = FontStyle::FONT_ITALIC;
    }

    return font_bold | font_italic;
}

void SourceTextPropertyPresenterImpl::SetFontStyle(
    int index)
{
    text_item_.IsBold(!!(index & FontStyle::FONT_BOLD));
    text_item_.IsItalic(!!(index & FontStyle::FONT_ITALIC));
}

SkColor SourceTextPropertyPresenterImpl::GetFontColor()
{
    return text_item_.TextColor();
}

void SourceTextPropertyPresenterImpl::SetFontColor(
    SkColor clr)
{
    text_item_.TextColor(clr);
}

float SourceTextPropertyPresenterImpl::GetTransparent()
{
    return MapFloatFromInt(text_item_.Transparent());
}

void SourceTextPropertyPresenterImpl::SetTransparent(
    float value)
{
    text_item_.Transparent(MapFloatToInt(value));
}

float SourceTextPropertyPresenterImpl::GetHorizontalScrollSpeed()
{
    return MapFloatFromInt(
        text_item_.HorizontalScrollSpeed(),
        kScrollSpeedMin,
        kScrollSpeedMax);
}

void SourceTextPropertyPresenterImpl::SetHorizontalScrollSpeed(float value)
{
    text_item_.HorizontalScrollSpeed(
        MapFloatToInt(
        value,
        kScrollSpeedMin,
        kScrollSpeedMax));
}

float SourceTextPropertyPresenterImpl::GetVerticalScrollSpeed()
{
    return MapFloatFromInt(
        text_item_.VerticalScrollSpeed(),
        kScrollSpeedMin,
        kScrollSpeedMax);
}

void SourceTextPropertyPresenterImpl::SetVerticalScrollSpeed(float value)
{
    text_item_.VerticalScrollSpeed(
        MapFloatToInt(
        value,
        kScrollSpeedMin,
        kScrollSpeedMax));
}

bool SourceTextPropertyPresenterImpl::GetOutline()
{
    return text_item_.Outline();
}

void SourceTextPropertyPresenterImpl::SetOutline(bool value)
{
    text_item_.Outline(value);
}

float SourceTextPropertyPresenterImpl::GetOutlineSize()
{
    return MapFloatFromInt(
        text_item_.OutlineSize(),
        kOutlineSizeMin,
        kOutlineSizeMax);
}

void SourceTextPropertyPresenterImpl::SetOutlineSize(float value)
{
    text_item_.OutlineSize(
        MapFloatToInt(
        value,
        kOutlineSizeMin,
        kOutlineSizeMax));
}

SkColor SourceTextPropertyPresenterImpl::GetOutlineColor()
{
    return text_item_.OutlineColor();
}

void SourceTextPropertyPresenterImpl::SetOutlineColor(SkColor value)
{
    text_item_.OutlineColor(value);
}

const std::vector<string16>& SourceTextPropertyPresenterImpl::GetSystemFontFamilies() const
{
    return system_font_families_;
}

std::vector<string16> SourceTextPropertyPresenterImpl::SearchString(const string16& search_text)
{
    std::vector<string16> search_item;
    if (searcher_.get())
    {
        auto result = searcher_->Search(const_cast<string16&>(search_text));

        for (const auto& it : result)
        {
            search_item.push_back(*it);
        }
    }
    return search_item;
}

void SourceTextPropertyPresenterImpl::InitFontSizeFromString(
    const string16& font_size_str)
{
    size_t from_pos = 0;
    while (from_pos < font_size_str.size())
    {
        size_t end_pos = font_size_str.find(L'#', from_pos);
        if (end_pos == std::wstring::npos)
        {
            break;
        }

        font_size_.push_back(
            font_size_str.substr(
            from_pos,
            end_pos - from_pos));

        from_pos = end_pos + 1;
    }
}

const std::vector<string16>& SourceTextPropertyPresenterImpl::GetInitFontSize() const
{
    return font_size_;
}

void SourceTextPropertyPresenterImpl::Snapshot()
{
    snapshot_ = livehime::TextPropertySnapshot::NewTake(&text_item_);
}

void SourceTextPropertyPresenterImpl::Restore()
{
    DCHECK(snapshot_);

    snapshot_->Restore();
}