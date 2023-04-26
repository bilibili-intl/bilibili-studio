/*
 @ 0xCCCCCCCC
*/

#include <unordered_map>

#include "base/basictypes.h"
#include "base/logging.h"

class BililiveFeatures {
public:
    enum Feature : size_t {
        TTSDanmaku,
        NativeLogin,
    };

    // (keyname, enabled)
    using FeatureInfo = std::pair<std::string, bool>;
    using FeatureMap = std::unordered_map<Feature, FeatureInfo>;

    ~BililiveFeatures() = default;

    // Be sure to call after the BililiveContext has been initialized.
    static void Init();

    static BililiveFeatures* current()
    {
        DCHECK(current_features_);
        return current_features_;
    }

    bool Enabled(Feature feature) const;

private:
    BililiveFeatures();

    void UpdateFeatureInfoFromFile(const std::string& content);

    DISALLOW_COPY_AND_ASSIGN(BililiveFeatures);

private:
    FeatureMap features_;

    static BililiveFeatures* current_features_;
};