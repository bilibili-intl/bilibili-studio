/*
 @ 0xCCCCCCCC
*/

#ifndef BILIBASE_STRING_FORMAT_H_
#define BILIBASE_STRING_FORMAT_H_

#include <algorithm>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <vector>

#include "bilibase/error_exception_util.h"

namespace bilibase {

class FormatError : public std::invalid_argument {
public:
    explicit FormatError(const std::string& what)
        : invalid_argument(what)
    {}

    explicit FormatError(const char* what)
        : invalid_argument(what)
    {}
};

namespace internal {

template<typename CharT>
struct FormatTraits {
    using String = std::basic_string<CharT>;
    using Stream = std::basic_ostringstream<CharT>;
};

template<typename CharT>
struct Placeholder {
    using String = typename FormatTraits<CharT>::String;

    size_t index = static_cast<size_t>(-1);
    size_t pos = static_cast<size_t>(-1);
    String format_specifier;
    String formatted;

    // For std::equal_range.
    struct RangeCompare {
        bool operator()(size_t index, const Placeholder& rhs) const
        {
            return index < rhs.index;
        }

        bool operator()(const Placeholder& lhs, size_t index) const
        {
            return lhs.index < index;
        }

        // HACK: VS2013 needs this function for a paranoid pre-sorted check
        // only in debug mode.
#if _DEBUG
        bool operator()(const Placeholder& lhs, const Placeholder& rhs)
        {
            return lhs.index < rhs.index;
        }
#endif
    };
};

template<typename CharT>
using PlaceholderList = std::vector<Placeholder<CharT>>;

// Return a simplified/analyzed format string, and store every specifiers into
// `placeholders`.
// Be aware of that elements in `placeholders` are sorted in the ascending order
// of index.

std::string AnalyzeFormat(const char* fmt, PlaceholderList<char>& placeholders);
std::wstring AnalyzeFormat(const wchar_t* fmt, PlaceholderList<wchar_t>& placeholders);

enum class SpecifierCategory {
    NONE = 0,
    PADDING_ALIGN,
    SIGN,
    WIDTH,
    PRECISION,
    TYPE
};

inline bool IsDigit(char ch)
{
    return isdigit(ch) != 0;
}

inline bool IsDigit(wchar_t ch)
{
    return iswdigit(ch) != 0;
}

inline unsigned long StrToUL(const char* str, char*& end_ptr)
{
    return strtoul(str, &end_ptr, 10);
}

inline unsigned long StrToUL(const wchar_t* str, wchar_t*& end_ptr)
{
    return wcstoul(str, &end_ptr, 10);
}

inline bool IsTypeSpecifier(char ch)
{
    return ch == 'b' || ch == 'x' || ch == 'X' || ch == 'o' || ch == 'e' || ch == 'E';
}

inline bool IsTypeSpecifier(wchar_t ch)
{
    return ch == L'b' || ch == L'x' || ch == L'X' || ch == L'o' || ch == L'e' || ch == L'E';
}

template<typename CharT>
SpecifierCategory GuessNextSpecCategory(const CharT* spec)
{
    // Maybe we have finished parsing.
    if (*spec == '\0') {
        return SpecifierCategory::NONE;
    }

    if (*(spec + 1) == '<' || *(spec + 1) == '>') {
        return SpecifierCategory::PADDING_ALIGN;
    }

    if (*spec == '+') {
        return SpecifierCategory::SIGN;
    }

    if (IsDigit(*spec)) {
        return SpecifierCategory::WIDTH;
    }

    if (*spec == '.') {
        return SpecifierCategory::PRECISION;
    }

    if (IsTypeSpecifier(*spec)) {
        return SpecifierCategory::TYPE;
    }

    ENSURE(NotReached()).Require<FormatError>();
    return SpecifierCategory::NONE;
}

template<typename CharT>
void ApplyPaddingAlignFormat(const CharT* spec,
                             SpecifierCategory last_spec_type,
                             typename FormatTraits<CharT>::Stream& stream,
                             const CharT*& spec_end)
{
    ENSURE(last_spec_type < SpecifierCategory::PADDING_ALIGN).Require<FormatError>();

    typename FormatTraits<CharT>::Stream& os = stream;

    CharT fill_ch = *spec++;
    os << std::setfill(fill_ch);
    if (*spec == '<') {
        os << std::left;
    } else {
        os << std::right;
    }

    spec_end = spec + 1;
}

template<typename CharT>
void ApplySignFormat(const CharT* spec,
                     SpecifierCategory last_spec_type,
                     typename FormatTraits<CharT>::Stream& stream,
                     const CharT*& spec_end)
{
    ENSURE(last_spec_type < SpecifierCategory::SIGN).Require<FormatError>();

    typename FormatTraits<CharT>::Stream& os = stream;

    os << std::showpos;

    spec_end = spec + 1;
}

template<typename CharT>
void ApplyWidthFormat(const CharT* spec,
                      SpecifierCategory last_spec_type,
                      typename FormatTraits<CharT>::Stream& stream,
                      const CharT*& spec_end)
{
    ENSURE(last_spec_type < SpecifierCategory::WIDTH).Require<FormatError>();

    typename FormatTraits<CharT>::Stream& os = stream;

    CharT* digit_end = nullptr;
    auto width = StrToUL(spec, digit_end);
    os << std::setw(width);

    spec_end = digit_end;
}

template<typename CharT>
void ApplyPrecisionFormat(const CharT* spec,
                          SpecifierCategory last_spec_type,
                          typename FormatTraits<CharT>::Stream& stream,
                          const CharT*& spec_end)
{
    ENSURE(last_spec_type < SpecifierCategory::PRECISION).Require<FormatError>();

    typename FormatTraits<CharT>::Stream& os = stream;

    CharT* digit_end = nullptr;
    auto precision_size = StrToUL(spec + 1, digit_end);
    os << std::fixed << std::setprecision(precision_size);

    spec_end = digit_end;
}

template<typename CharT>
void ApplyTypeFormat(const CharT* spec,
                     SpecifierCategory last_spec_type,
                     typename FormatTraits<CharT>::Stream& stream,
                     const CharT*& spec_end)
{
    ENSURE(last_spec_type < SpecifierCategory::TYPE).Require<FormatError>();

    typename FormatTraits<CharT>::Stream& os = stream;

    CharT type_mark = *spec;
    switch (type_mark) {
        case 'b':
            os << std::boolalpha;
            break;

        case 'x':
            os << std::hex;
            break;

        case 'X':
            os << std::hex << std::uppercase;
            break;

        case 'o':
            os << std::oct;
            break;

        case 'e':
            os << std::scientific;
            break;

        case 'E':
            os << std::scientific << std::uppercase;
            break;

        default:
            break;
    }

    spec_end = spec + 1;
}

template<typename CharT, typename Arg>
void FormatArgWithSpecifier(Arg&& arg,
                            const typename FormatTraits<CharT>::String& specifier,
                            typename FormatTraits<CharT>::Stream& stream,
                            typename FormatTraits<CharT>::String& formatted)
{
    if (specifier.empty()) {
        stream << arg;
        formatted = stream.str();
        return;
    }

    auto spec = specifier.data();
    auto last_spec_type = SpecifierCategory::NONE;
    auto next_spec_type = SpecifierCategory::NONE;
    while ((next_spec_type = GuessNextSpecCategory(spec)) != SpecifierCategory::NONE) {
        switch (next_spec_type) {
            case SpecifierCategory::PADDING_ALIGN:
                ApplyPaddingAlignFormat(spec, last_spec_type, stream, spec);
                last_spec_type = SpecifierCategory::PADDING_ALIGN;
                break;

            case SpecifierCategory::SIGN:
                ApplySignFormat(spec, last_spec_type, stream, spec);
                last_spec_type = SpecifierCategory::SIGN;
                break;

            case SpecifierCategory::WIDTH:
                ApplyWidthFormat(spec, last_spec_type, stream, spec);
                last_spec_type = SpecifierCategory::WIDTH;
                break;

            case SpecifierCategory::PRECISION:
                ApplyPrecisionFormat(spec, last_spec_type, stream, spec);
                last_spec_type = SpecifierCategory::PRECISION;
                break;

            case SpecifierCategory::TYPE:
                ApplyTypeFormat(spec, last_spec_type, stream, spec);
                last_spec_type = SpecifierCategory::TYPE;
                break;

            default:
                break;
        }
    }

    stream << arg;
    formatted = stream.str();
}

template<typename CharT>
typename FormatTraits<CharT>::String StringFormatT(const typename FormatTraits<CharT>::String& fmt,
                                                   PlaceholderList<CharT>& placeholders,
                                                   size_t arg_count)
{
    bool all_args_processed = std::all_of(placeholders.begin(), placeholders.end(),
                                          [arg_count](const Placeholder<CharT>& placeholder) {
                                              return placeholder.index < arg_count;
                                          });
    ENSURE(all_args_processed).Require<FormatError>();

    auto formatted_str(fmt);

    // Sort in the decreasing order of pos, since we are going to expand `formatted_str`
    // from right to left.
    std::sort(placeholders.begin(), placeholders.end(),
              [](const Placeholder<CharT>& lhs, const Placeholder<CharT>& rhs) {
                  return lhs.pos > rhs.pos;
              });

    for (const auto& placeholder : placeholders) {
        formatted_str.replace(placeholder.pos, 1, placeholder.formatted);
    }

    return formatted_str;
}

template<typename CharT, typename Arg, typename... Args>
typename FormatTraits<CharT>::String StringFormatT(const typename FormatTraits<CharT>::String& fmt,
                                                   PlaceholderList<CharT>& placeholders,
                                                   size_t arg_processing_index,
                                                   Arg&& arg,
                                                   Args&&... args)
{
    // One argument may associate with multiple placeholders.
    auto associated_placeholders = std::equal_range(placeholders.begin(),
                                                    placeholders.end(),
                                                    arg_processing_index,
                                                    typename Placeholder<CharT>::RangeCompare());
    typename FormatTraits<CharT>::Stream output;
    auto default_fmt_flags = output.flags();
    for (auto it = associated_placeholders.first; it != associated_placeholders.second; ++it) {
        FormatArgWithSpecifier<CharT, Arg>(std::forward<Arg>(arg),
                                           it->format_specifier,
                                           output,
                                           it->formatted);
        // Reset stream for reuse.
        output.str(typename FormatTraits<CharT>::String());
        output.clear();
        output.flags(default_fmt_flags);
    }

    return StringFormatT(fmt, placeholders, arg_processing_index + 1, std::forward<Args>(args)...);
}

}   // namespace internal

// C#-like string format facility.
// For each format-specifier in `fmt`, it can be in the form of
// {index[:[fill|align]|sign|width|.precision|type]}
// index := the 0-based index number of specifier.
// fill := any single character other than `{` and `}` for filling the padding.
// align := `<` for left-alignemnt with fill character and `>` for right-alignment.
// sign := +, prepend `+` with the positive number.
// width := the width of the field.
// .precision := floating-point precision.
// type := can be one of [b, x, X, o, e, E].
// Specifier marks `fill` and `align` **must** be in presence together.
// Although all of these specifier marks are optional, their relative orders, if any
// present, do matter; otherwise, a StringFormatSpecifierError exception would be raised.
// Also, if a specifier mark has no effect on its corresponding argument, this specifier
// mark is simply ignored, and no exception would be raised.

template<typename... Args>
std::string StringFormat(const char* fmt, Args&&... args)
{
    using namespace bilibase::internal;

    PlaceholderList<char> placeholders;
    placeholders.reserve(sizeof...(args));
    auto analyzed_fmt = AnalyzeFormat(fmt, placeholders);

    return StringFormatT(analyzed_fmt, placeholders, 0, std::forward<Args>(args)...);
}

template<typename... Args>
std::wstring StringFormat(const wchar_t* fmt, Args&&... args)
{
    using namespace bilibase::internal;

    PlaceholderList<wchar_t> placeholders;
    placeholders.reserve(sizeof...(args));
    auto analyzed_fmt = AnalyzeFormat(fmt, placeholders);

    return StringFormatT(analyzed_fmt, placeholders, 0, std::forward<Args>(args)...);
}

}   // namespace bilibase

#endif  // BILIBASE_STRING_FORMAT_H_