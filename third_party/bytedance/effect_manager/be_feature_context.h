#ifndef BE_REATURE_CONTEXT_H
#define BE_REATURE_CONTEXT_H


#include <stdio.h>
#include <string>
#include <map>



enum BE_FEATURE : int {
	BE_FEATURE_UNKONWN = 0,
	BE_FEATURE_EFFECT = 1,
	BE_FEATURE_FACE = 2,
	BE_FEATURE_FACE_EXTRA = 3,
	BE_FEATURE_FACE_ATTRIBUTE = 4,
	BE_FEATURE_SKELETON = 5,
	BE_FEATURE_HAND_DETECT = 6,
	BE_FEATURE_HAND_GESTURE = 7,
	BE_FEATURE_HAND_SEG = 8,
	BE_FEATURE_HAND_KEY_POINT = 9,
	BE_FEATURE_HAND_BOX = 10,
	BE_FEATURE_HAIRPARSE = 11,
	BE_FEATURE_PORTRAIT_MATTING = 12,
	BE_FEATURE_FACE_VERIFY = 13,
    BE_FEATURE_LIGHT_CLASSIFY = 14,
};

class BEFeatureContext {
public:
    BEFeatureContext();
    ~BEFeatureContext();
    std::string  getFeatureModelPath(BE_FEATURE be_feat) const;
    void initializeContext();
    void releaeseContext();    
    void setResourceDir(const std::string &resDir);
    const std::string &getLicensePath() const;
    const std::string &getMaterialDir() const;
public:
    const std::string &getStickerDir() const;
    const std::string &getFilterDir() const;
    const std::string &getComposerDir() const;
    const std::string &getComposerMakeupDir() const;
private:
    void registerAllModelPath();
private:
    std::map<BE_FEATURE, std::string> m_feature2ModelMap;
    bool m_initialized = false;
    std::string m_resourceDir = "";
    std::string m_licensePath = "";
    std::string m_modelDir = "";
    
    std::string m_materialDir = "";

    std::string m_stickerDir = "";
    std::string m_filterDir = "";
    std::string m_composerDir = "";
    std::string m_comopserMakeupDir = "";
};

#endif // !BE_REATURE_CONTEXT_H
