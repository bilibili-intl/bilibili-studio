/*
 @ 0xCCCCCCCC
*/

#include "bililive/common/bililive_features.h"

#include "base/file_util.h"
#include "base/ini_parser.h"
#include "base/strings/string_number_conversions.h"

#include "bililive/common/bililive_context.h"

namespace {

using Feature = BililiveFeatures::Feature;

const wchar_t kFeatureFileName[] = L"features";

BililiveFeatures::FeatureMap BuildFeatures()
{
    return {
        { Feature::TTSDanmaku, {"livehime.tts_danmaku", false} },
        { Feature::NativeLogin, {"livehime.pw_login", false} }
    };
}

// Returns true if value was parsed successfully, and `enabled` contains the parsed value.
// Returns false otherwise, and `enabled` won't be touched.
bool ParseFeatureEnabled(const std::string& value, bool& enabled)
{
    int number = -1;
    if (!base::StringToInt(value, &number) || number < 0) {
        NOTREACHED() << "Suspicious feature value detected! value: " << value;
        return false;
    }

    enabled = number != 0;

    return true;
}

}   // namespace

// The instance is leaked on purpose.
BililiveFeatures* BililiveFeatures::current_features_ = nullptr;

void BililiveFeatures::Init()
{
    DCHECK(!current_features_);

    current_features_ = new BililiveFeatures();
}

BililiveFeatures::BililiveFeatures()
    : features_(std::move(BuildFeatures()))
{
    auto feature_path = base::MakeAbsoluteFilePath(
        BililiveContext::Current()->GetMainDirectory().Append(kFeatureFileName));

    std::string content;
    file_util::ReadFileToString(feature_path, &content);
    if (content.empty()) {
        return;
    }

    UpdateFeatureInfoFromFile(content);
}

bool BililiveFeatures::Enabled(Feature feature) const
{
    auto it = features_.find(feature);

    if (it == features_.cend()) {
        NOTREACHED() << "Unknown feature: " << feature;
        return false;
    }

    return it->second.second;
}

void BililiveFeatures::UpdateFeatureInfoFromFile(const std::string& content)
{
    base::DictionaryValueINIParser ini;
    ini.Parse(content);

    for (auto& feature_info : features_) {
        const std::string& key = feature_info.second.first;
        std::string value;
        bool enabled = false;
        if (ini.root().GetString(key, &value) && ParseFeatureEnabled(value, enabled)) {
            feature_info.second.second = enabled;

            LOG(INFO) << "[Feature] " << key << "=" << value;
        }
    }
}
