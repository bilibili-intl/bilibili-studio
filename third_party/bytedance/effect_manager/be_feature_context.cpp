#include "be_feature_context.h"

BEFeatureContext::BEFeatureContext() {

}

BEFeatureContext::~BEFeatureContext() {
    releaeseContext();
}

void BEFeatureContext::initializeContext() {
    if (m_initialized) {
        return;
    }
    m_initialized = true;
    m_modelDir = m_resourceDir + "model/";
    m_materialDir = m_resourceDir + "material/";
    m_licensePath = m_resourceDir + "license/labcv_test_20191225_20200115_com.bytedance.labcv.demo_labcv_test_pc1.0.0.licbag"; 
    m_licensePath = m_resourceDir + "license/license.bag";
    m_stickerDir = m_materialDir + "stickers/";
    m_filterDir = m_materialDir + "FilterResource.bundle/1/";
    m_comopserMakeupDir = m_materialDir + "ComposeMakeup.bundle/ComposeMakeup/composer/";
    m_composerDir = m_materialDir + "ComposeMakeup.bundle/ComposeMakeup/";
    registerAllModelPath();
}

void BEFeatureContext::releaeseContext() {
    m_feature2ModelMap.clear(); 
}

std::string BEFeatureContext::getFeatureModelPath(BE_FEATURE beFeat) const {
    std::string model_path = "";

    auto itr = m_feature2ModelMap.find(beFeat);
    if (itr != m_feature2ModelMap.end())
    {
        model_path = itr->second;
    }
    return model_path;
}

const std::string &BEFeatureContext::getLicensePath() const{
    return m_licensePath;
}

const std::string &BEFeatureContext::getMaterialDir() const {
    return m_materialDir;
}

const std::string &BEFeatureContext::getStickerDir() const {
    return m_stickerDir;
}
const std::string &BEFeatureContext::getFilterDir() const {
    return m_filterDir;
}

const std::string &BEFeatureContext::getComposerDir() const {
    return m_composerDir;
}

const std::string &BEFeatureContext::getComposerMakeupDir() const {
    return m_comopserMakeupDir;
}

void BEFeatureContext::setResourceDir(const std::string &resDir) {
    m_resourceDir = resDir;
    if (m_resourceDir.length() == 0)
    {
        printf("Error: invalid model dir : %s \n", m_resourceDir.c_str());
        return;
    }
    if (m_resourceDir.rfind("/") != m_resourceDir.length() - 1)
    {
        m_resourceDir.append("/");
    }  
}

void BEFeatureContext::registerAllModelPath() {
    if (!m_modelDir.length()) 
    {
        return;
    }  
    m_feature2ModelMap.insert(std::make_pair(BE_FEATURE_EFFECT, m_modelDir));
    m_feature2ModelMap.insert(std::make_pair(BE_FEATURE_FACE, m_modelDir + "ttfacemodel/tt_face_v6.0.model"));
    m_feature2ModelMap.insert(std::make_pair(BE_FEATURE_FACE_EXTRA, m_modelDir + "ttfacemodel/tt_face_extra_v9.0.model"));
    m_feature2ModelMap.insert(std::make_pair(BE_FEATURE_FACE_ATTRIBUTE, m_modelDir + "ttfaceattrmodel/tt_face_attribute_v4.1.model"));
    m_feature2ModelMap.insert(std::make_pair(BE_FEATURE_SKELETON, m_modelDir + "skeleton_model/tt_skeleton_v5.0.model"));
    m_feature2ModelMap.insert(std::make_pair(BE_FEATURE_HAND_DETECT, m_modelDir + "handmodel/tt_hand_det_v7.0.model"));
    m_feature2ModelMap.insert(std::make_pair(BE_FEATURE_HAND_GESTURE, m_modelDir + "handmodel/tt_hand_gesture_v8.0.model"));
    m_feature2ModelMap.insert(std::make_pair(BE_FEATURE_HAND_SEG, m_modelDir + "handmodel/tt_hand_seg_v1.0.model"));
    m_feature2ModelMap.insert(std::make_pair(BE_FEATURE_HAND_KEY_POINT, m_modelDir + "handmodel/tt_hand_kp_v5.0.model"));
    m_feature2ModelMap.insert(std::make_pair(BE_FEATURE_HAND_BOX, m_modelDir + "handmodel/tt_hand_box_reg_v8.0.model"));
    m_feature2ModelMap.insert(std::make_pair(BE_FEATURE_HAIRPARSE, m_modelDir + "hairparser/tt_hair_v6.2.model"));
    m_feature2ModelMap.insert(std::make_pair(BE_FEATURE_PORTRAIT_MATTING, m_modelDir + "mattingmodel/tt_matting_v9.0.model"));    
    m_feature2ModelMap.insert(std::make_pair(BE_FEATURE_LIGHT_CLASSIFY, m_modelDir + "lightcls_model/tt_lightcls_v1.0.model"));
}
