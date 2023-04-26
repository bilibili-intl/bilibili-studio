#ifndef TTS_HELPER_H_
#define TTS_HELPER_H_

#include <atlbase.h>
#include <atlcom.h>
#include <string>
#include <memory>
#include <atomic>
#include <sstream>
#include <unordered_map>
#include <msxml6.h>

struct ISpVoice;

namespace bililive
{

bool InitTextToSpeech();
bool GetLastInitTextToSpeechResult();

class SpVoiceHelper
{
public:
    static std::unordered_map<std::wstring, CComPtr<IUnknown>> GetVoiceList();

    static SpVoiceHelper* Create();

    SpVoiceHelper();
    virtual ~SpVoiceHelper();

    bool SetSpeed(int speed); // -10 ~ 10
    bool GetSpeed(int& speed);
    bool SetVolume(int volume); // 0 ~ 100
    bool GetVolume(int& volume);
    bool SetVoice(CComPtr<IUnknown> voiceToken);

    bool Speak(const std::wstring& text);

private:
    bool CreateSpVoiceObject();

    bool Init();

    SpVoiceHelper(const SpVoiceHelper&) = delete;
    SpVoiceHelper& operator = (const SpVoiceHelper&) = delete;

private:
    CComPtr<ISpVoice> pSpVoice_;
};

class VoiceStringFormatter
{
public:
    enum { VF_AUTO = 0, VF_CHINESE, VF_JAPANESE, VF_ENGLISH };
    VoiceStringFormatter();
    ~VoiceStringFormatter();
    
    VoiceStringFormatter& Append(const std::wstring& text);
    VoiceStringFormatter& Append(const std::wstring& text, int lang);

    std::wstring ToString();

private:
    void FlushBuffer();

    int lastLangId_;
    std::wstringstream curBuf_;
    std::wstringstream buf_;
};

};

#endif
