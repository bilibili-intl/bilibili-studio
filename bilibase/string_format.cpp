/*
 @ 0xCCCCCCCC
*/

#include "bilibase/string_format.h"

namespace {

using bilibase::FormatError;
using bilibase::NotReached;
using bilibase::internal::FormatTraits;
using bilibase::internal::Placeholder;
using bilibase::internal::PlaceholderList;
using bilibase::internal::IsDigit;
using bilibase::internal::StrToUL;

enum class FormatParseState {
    IN_TEXT,
    IN_FORMAT
};

const char kEscapeBegin = '{';
const char kEscapeEnd = '}';
const char kSpecifierDelimeter = ':';
const char kPlaceholderMark = '@';

template<typename CharT>
size_t ExtractPlaceholderIndex(const CharT* first_digit, CharT*& last_digit)
{
    auto index = StrToUL(first_digit, last_digit);
    --last_digit;

    return index;
}

template<typename CharT>
typename FormatTraits<CharT>::String AnalyzeFormatT(const CharT* fmt, PlaceholderList<CharT>& placeholders)
{
    const size_t kInitialCapacity = 32;
    typename FormatTraits<CharT>::String analyzed_fmt;
    analyzed_fmt.reserve(kInitialCapacity);

    placeholders.clear();
    Placeholder<CharT> placeholder;

    auto state = FormatParseState::IN_TEXT;
    for (auto ptr = fmt; *ptr != '\0'; ++ptr) {
        if (*ptr == kEscapeBegin) {
            // `{` is an invalid token for in-format state.
            ENSURE(state != FormatParseState::IN_FORMAT).Require<FormatError>();
            if (*(ptr + 1) == kEscapeBegin) {
                // Use `{{` to represent literal `{`.
                analyzed_fmt += kEscapeBegin;
                ++ptr;
            } else if (IsDigit(*(ptr + 1))) {
                CharT* last_digit;
                placeholder.index = ExtractPlaceholderIndex(ptr + 1, last_digit);
                ptr = last_digit;
                ENSURE((*(ptr + 1) == kEscapeEnd) ||
                       (*(ptr + 1) == kSpecifierDelimeter)).Require<FormatError>();
                if (*(ptr + 1) == kSpecifierDelimeter) {
                    ++ptr;
                }

                // Turn into in-format state.
                state = FormatParseState::IN_FORMAT;
            } else {
                ENSURE(NotReached()).Require<FormatError>();
            }
        } else if (*ptr == kEscapeEnd) {
            if (state == FormatParseState::IN_TEXT) {
                ENSURE(*(ptr + 1) == kEscapeEnd).Require<FormatError>();
                analyzed_fmt += kEscapeEnd;
                ++ptr;
            } else {
                placeholder.pos = analyzed_fmt.length();
                analyzed_fmt += kPlaceholderMark;
                placeholders.push_back(placeholder);
                placeholder.format_specifier.clear();

                // Now we turn back into in-text state.
                state = FormatParseState::IN_TEXT;
            }
        } else {
            if (state == FormatParseState::IN_TEXT) {
                analyzed_fmt += *ptr;
            } else {
                placeholder.format_specifier += *ptr;
            }
        }
    }

    ENSURE(state == FormatParseState::IN_TEXT).Require<FormatError>();

    std::sort(std::begin(placeholders), std::end(placeholders),
              [](const Placeholder<CharT>& lhs, const Placeholder<CharT>& rhs) {
                  return lhs.index < rhs.index;
              });

    return analyzed_fmt;
}

} // namespace

namespace bilibase {

namespace internal {

std::string AnalyzeFormat(const char* fmt, PlaceholderList<char>& placeholders)
{
    return AnalyzeFormatT(fmt, placeholders);
}

std::wstring AnalyzeFormat(const wchar_t* fmt, PlaceholderList<wchar_t>& placeholders)
{
    return AnalyzeFormatT(fmt, placeholders);
}

}   // namespace internal

}   // namespace bilibase
