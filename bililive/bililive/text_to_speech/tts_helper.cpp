#include "tts_helper.h"

#include <sapi.h>
#include <process.h>
#include <objbase.h>
#include <algorithm>
#include <functional>
#include <regex>

#include "base/logging.h"
#include "base/win/windows_version.h"

#define SPVOICES_FILTER_PATTERN L"(- Chinese)|(-CN)"
#define SPVOICES_FILTER_JAPANESE_PATTERN L"(- Japanese)|(-JP)"


namespace bililive
{

static GUID BILILIVE_CLSID_SpVoice;
static GUID BILILIVE_CLSID_SpObjectTokenCategory;
static const wchar_t* BILILIVE_SPCAT_VOICES;
static bool s_has_japanese_support = false;

static bool s_last_init_tts_result = false;

static std::unordered_map<std::wstring, CComPtr<IUnknown>> GetVoiceListInternal(const wchar_t* filter_pattern)
{
    std::unordered_map<std::wstring, CComPtr<IUnknown>> result;
    CComPtr<IEnumSpObjectTokens> spEnumTokens;
    std::wregex regPattern(filter_pattern);

    // get category from id
    CComPtr<ISpObjectTokenCategory> cpTokenCategory;

    HRESULT hr;
    hr = cpTokenCategory.CoCreateInstance(BILILIVE_CLSID_SpObjectTokenCategory);

    if (SUCCEEDED(hr))
    {
        hr = cpTokenCategory->SetId(BILILIVE_SPCAT_VOICES, FALSE);
    }

    if (SUCCEEDED(hr))
    {
        hr = cpTokenCategory->EnumTokens(NULL, NULL, &spEnumTokens);
    }

    if (!SUCCEEDED(hr))
        return{};

    ULONG spTokensCount = 0;
    if (spEnumTokens->GetCount(&spTokensCount) != S_OK || spTokensCount == 0)
        return{};
    for (ULONG i = 0; i < spTokensCount; ++i)
    {
        CComPtr<ISpObjectToken> voiceToken;
        if (spEnumTokens->Next(1, &voiceToken, NULL) != S_OK)
            return{};
        LPWSTR name = nullptr;
        if (voiceToken->GetStringValue(L"", &name) == S_OK && name != nullptr)
        {
            if (std::regex_search(name, regPattern))
            {
                if (result.find(name) == result.end())
                {
                    CComPtr<IUnknown> pToken;
                    if (voiceToken.QueryInterface<IUnknown>(&pToken) == S_OK)
                        result[name] = pToken;
                }
            }
        }
    }

    return result;
}

static bool CheckTTSObjects()
{
    CComPtr<ISpVoice> spVoice;
    CComPtr<ISpObjectTokenCategory> spTokenCategory;

    if (spVoice.CoCreateInstance(BILILIVE_CLSID_SpVoice) == S_OK
        && !GetVoiceListInternal(SPVOICES_FILTER_PATTERN).empty()
        )
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool InitTextToSpeech()
{
    bool result = false;

    do {
        // speech onecore for win10 creators update
        CLSIDFromString(L"{9BC773B8-9B6C-400F-8AF0-0DFDD1C43229}", &BILILIVE_CLSID_SpVoice);
        CLSIDFromString(L"{461DED9E-81D5-494F-BC96-6432C8645733}", &BILILIVE_CLSID_SpObjectTokenCategory);
        BILILIVE_SPCAT_VOICES = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech_OneCore\\Voices";

        if (CheckTTSObjects())
        {
            LOG(INFO) << "[TTS] Using Win10 OneCore speech engine";
            result = true;
            break;
        }

        // speech onecore for microsoft speech 11 for windows 7
        if (base::win::OSInfo::GetInstance()->version() <= base::win::VERSION_WIN7)
        {
            CLSIDFromString(L"{D941651C-44E6-4C17-BADF-C36826FC3424}", &BILILIVE_CLSID_SpVoice);
            CLSIDFromString(L"{BF86F6C4-F9CE-441A-821A-415A76B86D79}", &BILILIVE_CLSID_SpObjectTokenCategory);
            BILILIVE_SPCAT_VOICES = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech Server\\v11.0\\Voices";

            if (CheckTTSObjects())
            {
                LOG(INFO) << "[TTS] Using microsoft speech 11 speech engine";
                result = true;
                break;
            }
        }

        // legacy sapi speech object
        CLSIDFromString(L"{96749377-3391-11D2-9EE3-00C04F797396}", &BILILIVE_CLSID_SpVoice);
        CLSIDFromString(L"{A910187F-0C7A-45AC-92CC-59EDAFB77B53}", &BILILIVE_CLSID_SpObjectTokenCategory);
        BILILIVE_SPCAT_VOICES = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\Voices";

        if (CheckTTSObjects())
        {
            LOG(INFO) << "[TTS] Using legacy speech engine";
            result = true;
            break;
        }

        LOG(INFO) << "[TTS] No supported TTS Engine found.";
    } while (false);

    // check if chinese voice exists
    if (result)
    {
        auto chineseList = GetVoiceListInternal(SPVOICES_FILTER_PATTERN);
        if (chineseList.empty())
        {
            LOG(INFO) << "[TTS] No chinese voice found.";
            result = false;
        }
        else
        {
            LOG(INFO) << "[TTS] " << chineseList.size() << " chinese voice found.";
        }
    }

    // check if japanese voice exists
    if (result)
    {
        auto japaneseList = GetVoiceListInternal(SPVOICES_FILTER_JAPANESE_PATTERN);
        if (japaneseList.empty())
        {
            LOG(INFO) << "[TTS] No japanese voice found.";
            s_has_japanese_support = false;
        }
        else
        {
            LOG(INFO) << "[TTS] " << japaneseList.size() << " japanese voice found.";
            s_has_japanese_support = true;
        }
    }

    if (result)
    {
        LOG(INFO) << "[TTS] Enable TTS support.";
    }

    return (s_last_init_tts_result = result);
}

bool GetLastInitTextToSpeechResult()
{
    return s_last_init_tts_result;
}

bool SpVoiceHelper::CreateSpVoiceObject()
{
    if (pSpVoice_.CoCreateInstance(BILILIVE_CLSID_SpVoice) != S_OK)
    {
        DCHECK(0) << "[TTS] Failed to create SpVoice object.";
        return false;
    }
    return true;
}

bool SpVoiceHelper::SetVoice(CComPtr<IUnknown> voiceToken)
{
    CComPtr<ISpObjectToken> token;
    if (voiceToken.QueryInterface(&token) != S_OK)
    {
        DCHECK(0) << "[TTS] Incorrect voice token object";
        return false;
    }
    if (pSpVoice_->SetVoice(token) != S_OK)
    {
        DCHECK(0) << "[TTS] set voice failed";
        return false;
    }

    return true;
}

SpVoiceHelper::SpVoiceHelper()
{
}

SpVoiceHelper::~SpVoiceHelper()
{
}


bool SpVoiceHelper::Init()
{
    if (!CreateSpVoiceObject())
        return false;
    return true;
}

bool SpVoiceHelper::Speak(const std::wstring& text)
{
    HRESULT hr = pSpVoice_->Speak(text.c_str(), SPF_IS_XML | SPF_PARSE_SAPI | SPF_ASYNC, NULL);
    if (SUCCEEDED(hr))
    {
        pSpVoice_->WaitUntilDone(INFINITE);
        return true;
    }
    else
        return false;
}

bool SpVoiceHelper::SetSpeed(int speed)
{
    HRESULT hr = pSpVoice_->SetRate(speed);
    return SUCCEEDED(hr);
}

bool SpVoiceHelper::GetSpeed(int& speed)
{
    long result;
    HRESULT hr = pSpVoice_->GetRate(&result);
    if (SUCCEEDED(hr))
    {
        speed = static_cast<int>(result);
        return true;
    }
    else
        return false;
}

bool SpVoiceHelper::SetVolume(int volume)
{
    HRESULT hr = pSpVoice_->SetVolume(static_cast<USHORT>(volume));
    return SUCCEEDED(hr);
}

bool SpVoiceHelper::GetVolume(int& volume)
{
    USHORT result;
    HRESULT hr = pSpVoice_->GetVolume(&result);
    if (SUCCEEDED(hr))
    {
        volume = static_cast<int>(result);
        return true;
    }
    else
        return false;
}



std::unordered_map<std::wstring, CComPtr<IUnknown>> SpVoiceHelper::GetVoiceList()
{
    return GetVoiceListInternal(SPVOICES_FILTER_PATTERN);
}

SpVoiceHelper* SpVoiceHelper::Create()
{
    std::unique_ptr<SpVoiceHelper> result(new SpVoiceHelper());
    if (!result->Init())
        return nullptr;

    return result.release();
}


VoiceStringFormatter::VoiceStringFormatter()
    : lastLangId_(VF_AUTO)
{
}

VoiceStringFormatter::~VoiceStringFormatter()
{
}

VoiceStringFormatter& VoiceStringFormatter::Append(const std::wstring& text)
{
    std::wstring tmpText(text);
    int lang = VF_CHINESE;

    if (s_has_japanese_support)
    {
        // 检查是否含有日语假名
        bool hasKana = std::any_of(tmpText.begin(), tmpText.end(), [](wchar_t ch)
        {
            //if (ch == L'の')
            //    return false; //の之外还有其他的才算！
            if (ch >= 0x3041 && ch <= 0x3096) // hiragana
                return true;
            if (ch >= 0x30a1 && ch <= 0x30fa) // katakana
                return true;
            return false;
        }
        );

        if (hasKana)
        {
            // 如果有假名，检查是不是合法的shiftjis字符串
            LPCSTR defChar = "?";
            BOOL useDefChar = FALSE;
            WideCharToMultiByte(932, 0, tmpText.c_str(), tmpText.size(), NULL, NULL, defChar, &useDefChar);

            if (!useDefChar)
                lang = VF_JAPANESE;
        }
    }

    // 如果是中文，の 读作 的
    if (lang == VF_CHINESE)
    {
        for (auto& x : tmpText)
        {
            if (x == L'の')
                x = L'之';
        }
    }

    return Append(tmpText, lang);
}

VoiceStringFormatter& VoiceStringFormatter::Append(const std::wstring& text, int lang)
{
    if (lastLangId_ == VF_AUTO)
        lastLangId_ = lang;

    if (lastLangId_ != lang)
        FlushBuffer();

    lastLangId_ = lang;
    curBuf_ << text;
    return *this;
}

void VoiceStringFormatter::FlushBuffer()
{
    if (lastLangId_ == VF_AUTO)
        return;

    CComBSTR nodeName(L"lang");
    CComBSTR langIdAttrName(L"langid");
    CComBSTR langId;
    CComBSTR innerText(curBuf_.str().c_str());

    CComPtr<IXMLDOMDocument> xdoc;
    if (SUCCEEDED(xdoc.CoCreateInstance(L"Microsoft.XMLDOM")))
    {
        if (lastLangId_ == VF_CHINESE)
        {
            langId = L"004";
        }
        else if (lastLangId_ == VF_JAPANESE)
        {
            langId = L"411";
        }
        else if (lastLangId_ == VF_ENGLISH)
        {
            langId = L"409";
        }

        CComPtr<IXMLDOMElement> newElem;
        if (xdoc->createElement(nodeName, &newElem) == S_OK)
        {
            if (newElem->setAttribute(langIdAttrName, CComVariant(langId)) == S_OK)
            {
                CComPtr<IXMLDOMNode> newNode;
                newElem->put_text(innerText);
                xdoc->appendChild(newElem, &newNode);

                CComBSTR xmlStr;
                if (xdoc->get_xml(&xmlStr) == S_OK)
                {
                    size_t len = xmlStr.Length();
                    if (len > 0)
                    {
                        const wchar_t* xmlStrEnd = (const wchar_t*)xmlStr + len;
                        if (len > 2)
                        {
                            if (xmlStr[len - 2] == L'\r' && xmlStr[len - 1] == L'\n')
                                xmlStrEnd -= 2;
                        }
                        buf_ << std::wstring((const wchar_t*)xmlStr, xmlStrEnd);
                    }
                }
            }
        }
    }
    else
    {
        buf_ << curBuf_.str();
    }

    curBuf_ = std::wstringstream();
    lastLangId_ = VF_AUTO;
}

std::wstring VoiceStringFormatter::ToString()
{
    FlushBuffer();
    return buf_.str();
}

};