#include <chrono>

#include "be_effect_handle.h"
#include "be_resource_context.h"
#include "be_util.h"
#include "be_defines.h"
#include "bef_effect_ai_version.h"
#include <new>

EffectHandle::EffectHandle() {}

EffectHandle::~EffectHandle() {}



bef_effect_result_t EffectHandle::initializeHandle() {
    bef_effect_result_t ret = 0;
    if (m_initialized)
    {
        return ret;
    }
    m_initialized = true;
    ret = bef_effect_ai_create(&m_renderMangerHandle);
    CHECK_BEF_AI_RET_SUCCESS(ret, "EffectHandle::initializeHandle:: create effect handle failed !");

    ret = bef_effect_ai_check_license(m_renderMangerHandle, getResourceContext()->getFeatureContext()->getLicensePath().c_str());
    CHECK_BEF_AI_RET_SUCCESS(ret, "EffectHandle::initializeHandle:: check_license failed !");

    ret = bef_effect_ai_init(m_renderMangerHandle, m_init_width, m__init_height, getResourceContext()->getFeatureContext()->getFeatureModelPath(BE_FEATURE_EFFECT).c_str(), "");
    CHECK_BEF_AI_RET_SUCCESS(ret, "EffectHandle::initializeHandle:: init effect handle failed !");
     
    ret = bef_effect_ai_composer_set_mode(m_renderMangerHandle, 1, 0);
    CHECK_BEF_AI_RET_SUCCESS(ret, "EffectHandle::initializeHandle:: bef_effect_ai_composer_set_mode failed !");

    registerBeautyComposerNodes();
    return ret;
}
bef_effect_result_t EffectHandle::releaseHandle() {
    bef_effect_ai_destroy(m_renderMangerHandle);
    return 0;
}

bef_effect_result_t EffectHandle::process(GLint texture, GLint textureSticker, int width, int height, int timeStamp) {
    bef_effect_result_t ret = 0;
    m_effectMutex.lock();
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    double timestamp = (double)time.count() / 1000.0;

    ScopeTimer t(__FUNCTION__);
    bef_effect_ai_set_width_height(m_renderMangerHandle, width, height);
    auto time_alg_start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    bef_effect_ai_algorithm_texture(m_renderMangerHandle, texture, timestamp);
    auto time_render_start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    bef_effect_result_t iRet = bef_effect_ai_process_texture(m_renderMangerHandle, texture, textureSticker, timestamp);
    auto time_render_end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    m_effectMutex.unlock();
    return ret;
}


void EffectHandle::setInitWidthAndHeight(int width, int height) {
    m_init_width = width;
    m__init_height = height;

}

bef_effect_result_t EffectHandle::setEffectWidthAndHeight(int width, int height) {
    bef_effect_ai_set_width_height(m_renderMangerHandle, width, height);
    return 0;
}


void EffectHandle::updateComposerNode(int subId, int updateStatus, float value) {

    auto itr = m_allComposerNodeMap.find(subId);
    if (itr != m_allComposerNodeMap.end()) {
        BEComposerNode *node = itr->second;
        bool flag = false;
        if (node != nullptr)
        {
            node->setValue(value);
            switch (updateStatus)
            {
            case 0: {
                flag = removeComposerPath(node->isMajor() ? m_currentMajorPaths : m_currentSubPaths, node);
                if (flag)
                {
                    setComposerNodes();
                }
                break;
            }
            case 1: 
            case 2:
            {
                flag = addComposerPath(node->isMajor() ? m_currentMajorPaths : m_currentSubPaths, node);
                if (flag)
                {
                    setComposerNodes();
                }
                updateComposerNodeValue(node);
                break;
            }
            default:
                printf("Invalid update operation!\n");
                break;
            }
        }
    }
    else {
        printf("Invalid effect id: %d\n", subId);
    }
}



bool EffectHandle::addComposerPath(std::map<int, std::string> &pathMap, BEComposerNode *node) {
    if (node != nullptr)
    {
        int id = node->isMajor() ? node->getMajorId() : node->getSubId();        
        if (pathMap.find(id) == pathMap.end())
        {
            pathMap.insert(std::make_pair(id, node->getNodeName()));
            return true;
        }  
    }
    return false;
}
bool EffectHandle::removeComposerPath(std::map<int, std::string> &pathMap, BEComposerNode *node) {
    if (node != nullptr)
    {
        int id = node->isMajor() ? node->getMajorId() : node->getSubId();
        if (pathMap.find(id) != pathMap.end())
        {
            pathMap.erase(id);
            return true;
        }
    }
    return false;
}

void EffectHandle::setComposerNodes() {
    int majorCount = m_currentMajorPaths.size();
    int subCount = m_currentSubPaths.size();
    char **nodePaths = nullptr;
    if (majorCount + subCount > 0)
    {
        printf("set composerNodes:\n");
        char **nodePaths = (char **)malloc((majorCount + subCount) * sizeof(char *));
        int i = 0;
        for (auto itr = m_currentMajorPaths.begin(); itr != m_currentMajorPaths.end(); itr++)
        {            
            nodePaths[i] = (char*) itr->second.c_str();
            printf("path: %s\n", nodePaths[i]);
            i++;
        }
        for (auto itr = m_currentSubPaths.begin(); itr != m_currentSubPaths.end(); itr++)
        {
            nodePaths[i] = (char*)itr->second.c_str();
            printf("path: %s\n", nodePaths[i]);
            i++;
        }
        bef_effect_ai_composer_set_nodes(m_renderMangerHandle, (const char**)nodePaths, majorCount + subCount);
        //free(nodePaths);
    }
    else
    {
        bef_effect_ai_composer_set_nodes(m_renderMangerHandle, (const char**)nodePaths, 0);
    }   
}

void EffectHandle::updateComposerNodeValue(BEComposerNode *node) {
    if (node != nullptr)
    {        
        int ret = bef_effect_ai_composer_update_node(m_renderMangerHandle, node->getNodeName().c_str(), node->getKey().c_str(), node->getValue());
        printf("bef_effect_ai_composer_update_node ret %d\n", ret);
    }
}




void EffectHandle::registerBeautyComposerNodes() {
    registerBeautyFaceNodes();
    registerBeautyReshaperNodes();
    registerBeautyMakeupNodes();
}

void EffectHandle::registerComposerNode(int majorId, int subId, bool isMajor, std::string NodeName, std::string key) {
    auto itr = m_allComposerNodeMap.find(subId);
    if (itr == m_allComposerNodeMap.end())
    {
        m_allComposerNodeMap.insert(std::make_pair(subId, new BEComposerNode(majorId, subId, isMajor, NodeName, key)));
    }
}

void EffectHandle::registerBeautyFaceNodes() {
    registerComposerNode(TYPE_BEAUTY_FACE, TYPE_BEAUTY_FACE_SMOOTH, true, NODE_BEAUTY, "smooth");
    registerComposerNode(TYPE_BEAUTY_FACE, TYPE_BEAUTY_FACE_WHITEN, true, NODE_BEAUTY, "whiten");
    registerComposerNode(TYPE_BEAUTY_FACE, TYPE_BEAUTY_FACE_SHARPEN, true, NODE_BEAUTY, "sharp");

    registerComposerNode(TYPE_BEAUTY_FACE_4ITEMS,TYPE_BEAUTY_FACE_4ITEMS_BRIGHTEN_EYE, true, NODE_BEAUTY_4ITEMS, "BEF_BEAUTY_BRIGHTEN_EYE");
    registerComposerNode(TYPE_BEAUTY_FACE_4ITEMS,TYPE_BEAUTY_FACE_4ITEMS_REMOVE_POUCH, true, NODE_BEAUTY_4ITEMS, "BEF_BEAUTY_REMOVE_POUCH");
    registerComposerNode(TYPE_BEAUTY_FACE_4ITEMS,TYPE_BEAUTY_FACE_4ITEMS_SMILE_FOLDS, true, NODE_BEAUTY_4ITEMS, "BEF_BEAUTY_SMILES_FOLDS");
    registerComposerNode(TYPE_BEAUTY_FACE_4ITEMS,TYPE_BEAUTY_FACE_4ITEMS_WHITEN_TEETH, true, NODE_BEAUTY_4ITEMS, "BEF_BEAUTY_WHITEN_TEETH");
}

void EffectHandle::registerBeautyReshaperNodes() {
    registerComposerNode(TYPE_BEAUTY_RESHAPE, TYPE_BEAUTY_RESHAPE_FACE_OVERALL, true, NODE_RESHAPE, "Internal_Deform_Overall");
    registerComposerNode(TYPE_BEAUTY_RESHAPE, TYPE_BEAUTY_RESHAPE_FACE_CUT, true, NODE_RESHAPE, "Internal_Deform_CutFace");
    registerComposerNode(TYPE_BEAUTY_RESHAPE, TYPE_BEAUTY_RESHAPE_FACE_SMALL, true, NODE_RESHAPE, "Internal_Deform_Face");
    registerComposerNode(TYPE_BEAUTY_RESHAPE, TYPE_BEAUTY_RESHAPE_EYE, true, NODE_RESHAPE, "Internal_Deform_Eye");
    registerComposerNode(TYPE_BEAUTY_RESHAPE, TYPE_BEAUTY_RESHAPE_EYE_ROTATE, true, NODE_RESHAPE, "Internal_Deform_RotateEye");
    registerComposerNode(TYPE_BEAUTY_RESHAPE, TYPE_BEAUTY_RESHAPE_CHEEK, true, NODE_RESHAPE, "Internal_Deform_Zoom_Cheekbone");
    registerComposerNode(TYPE_BEAUTY_RESHAPE, TYPE_BEAUTY_RESHAPE_JAW, true, NODE_RESHAPE, "Internal_Deform_Zoom_Jawbone");
    registerComposerNode(TYPE_BEAUTY_RESHAPE, TYPE_BEAUTY_RESHAPE_NOSE_LEAN, true, NODE_RESHAPE, "Internal_Deform_Nose");
    registerComposerNode(TYPE_BEAUTY_RESHAPE, TYPE_BEAUTY_RESHAPE_NOSE_LONG, true, NODE_RESHAPE, "Internal_Deform_MovNose");
    registerComposerNode(TYPE_BEAUTY_RESHAPE, TYPE_BEAUTY_RESHAPE_CHIN, true, NODE_RESHAPE, "Internal_Deform_Chin");
    registerComposerNode(TYPE_BEAUTY_RESHAPE, TYPE_BEAUTY_RESHAPE_FOREHEAD, true, NODE_RESHAPE, "Internal_Deform_Forehead");
    registerComposerNode(TYPE_BEAUTY_RESHAPE, TYPE_BEAUTY_RESHAPE_MOUTH_ZOOM, true, NODE_RESHAPE, "Internal_Deform_ZoomMouth");
    registerComposerNode(TYPE_BEAUTY_RESHAPE, TYPE_BEAUTY_RESHAPE_MOUTH_SMILE, true, NODE_RESHAPE, "Internal_Deform_MouthCorner");
    registerComposerNode(TYPE_BEAUTY_RESHAPE, TYPE_BEAUTY_RESHAPE_EYE_SPACING, true, NODE_RESHAPE, "Internal_Eye_Spacing");
    registerComposerNode(TYPE_BEAUTY_RESHAPE, TYPE_BEAUTY_RESHAPE_EYE_MOVE, true, NODE_RESHAPE, "Internal_Deform_Eye_Move");
    registerComposerNode(TYPE_BEAUTY_RESHAPE, TYPE_BEAUTY_RESHAPE_MOUTH_MOVE, true, NODE_RESHAPE, "Internal_Deform_MovMouth");
}

void EffectHandle::registerBeautyMakeupNodes() {
    //case TYPE_MAKEUP_LIP:
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_LIP_FUGUHONG, false, "lip/fuguhong", "Internal_Makeup_Lips");
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_LIP_SHAONVFEN, false, "lip/shaonvfen", "Internal_Makeup_Lips");
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_LIP_YUANQIJU, false, "lip/yuanqiju", "Internal_Makeup_Lips");
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_LIP_XIYOUSE, false, "lip/xiyouse", "Internal_Makeup_Lips");
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_LIP_XIGUAHONG, false, "lip/xiguahong", "Internal_Makeup_Lips");
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_LIP_SIRONGHONG, false, "lip/sironghong", "Internal_Makeup_Lips");
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_LIP_ZANGJUSE, false, "lip/zangjuse", "Internal_Makeup_Lips");
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_LIP_MEIZISE, false, "lip/meizise", "Internal_Makeup_Lips");
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_LIP_SHANHUSE, false, "lip/shanhuse", "Internal_Makeup_Lips");
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_LIP_DOUSHAFEN, false, "lip/doushafen", "Internal_Makeup_Lips");

    //case TYPE_MAKEUP_BLUSH:
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_BLUSH_WEIXUN, false, "blush/weixun", "Internal_Makeup_Blusher");
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_BLUSH_RICHANG, false, "blush/richang", "Internal_Makeup_Blusher");
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_BLUSH_MITAO, false, "blush/mitao", "Internal_Makeup_Blusher");
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_BLUSH_TIANCHENG, false, "blush/tiancheng", "Internal_Makeup_Blusher");
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_BLUSH_QIAOPI, false, "blush/qiaopi", "Internal_Makeup_Blusher");
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_BLUSH_XINJI, false, "blush/xinji", "Internal_Makeup_Blusher");
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_BLUSH_SHAISHANG, false, "blush/shaishang", "Internal_Makeup_Blusher");


    //case TYPE_MAKEUP_PUPIL:
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_PUPIL_HUNXUEZONG, false, "pupil/hunxuezong", "Internal_Makeup_Pupil");
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_PUPIL_KEKEZONG, false, "pupil/kekezong", "Internal_Makeup_Pupil");
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_PUPIL_MITAOFEN, false, "pupil/mitaofen", "Internal_Makeup_Pupil");
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_PUPIL_SHUIGUANGHEI, false, "pupil/shuiguanghei", "Internal_Makeup_Pupil");
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_PUPIL_XINGKONGLAN, false, "pupil/xingkonglan", "Internal_Makeup_Pupil");
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_PUPIL_CHUJIANHUI, false, "pupil/chujianhui", "Internal_Makeup_Pupil");

    //case TYPE_MAKEUP_HAIRDYE:
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_HAIRDYE_ANLAN, false, "hair/anlan", "");
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_HAIRDYE_MOLV, false, "hair/molv", "");
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_HAIRDYE_SHENZONG, false, "hair/shenzong", "");

    //case TYPE_MAKEUP_EYESHADOW:
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_EYESHADOW_WANXIAHONG, false, "eyeshadow/ziranlan", "Internal_Makeup_Eye");
    //registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_EYESHADOW_DADIZONG, false, "eyeshadow/dadizong", "Internal_Makeup_Eye");
    //registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_EYESHADOW_WANXIAHONG, false, "eyeshadow/wanxiahong", "Internal_Makeup_Eye");
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_EYESHADOW_SHAONVFEN, false, "eyeshadow/shaonvfen", "Internal_Makeup_Eye");
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_EYESHADOW_QIZHIFEN, false, "eyeshadow/qizhifen", "Internal_Makeup_Eye");
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_EYESHADOW_MEIZIHONG, false, "eyeshadow/meizihong", "Internal_Makeup_Eye");
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_EYESHADOW_JIAOTANGZONG, false, "eyeshadow/jiaotangzong", "Internal_Makeup_Eye");
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_EYESHADOW_YUANQIJU, false, "eyeshadow/yuanqiju", "Internal_Makeup_Eye");
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_EYESHADOW_NAICHASE, false, "eyeshadow/naichase", "Internal_Makeup_Eye");

    //case TYPE_MAKEUP_BROW:
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_BROW_BR01, false, "eyebrow/BR01", "Internal_Makeup_Brow");
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_BROW_BK01, false, "eyebrow/BK01", "Internal_Makeup_Brow");
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_BROW_BK02, false, "eyebrow/BK02", "Internal_Makeup_Brow");
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_BROW_BK03, false, "eyebrow/BK03", "Internal_Makeup_Brow");


    //case TYPE_MAKEUP_TRIM:
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_TRIM_TRIM01, false, "facial/xiurong01", "Internal_Makeup_Facial");
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_TRIM_TRIM02, false, "facial/xiurong02", "Internal_Makeup_Facial");
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_TRIM_TRIM03, false, "facial/xiurong03", "Internal_Makeup_Facial");
    registerComposerNode(TYPE_MAKEUP_OPTION, TYPE_MAKEUP_TRIM_TRIM04, false, "facial/xiurong04", "Internal_Makeup_Facial");
}

void EffectHandle::setEffectOn(BE_EFFECT_TYPE effectType, bool checked) {
    switch (effectType)
    {
    case EFFECT_TYPE_BEAUTY: {
        setBeautyOn(checked);
        break;
    }
    case EFFECT_TYPE_RESHAPE: {
        setReshapeOn(checked);
        break;
    }
    case EFFECT_TYPE_MAKEUP: {
        setMakeupOn(checked);
    }
    }
}

void EffectHandle::setReshapeOn(bool checked) {
    if (checked)
    {
        std::string materialDir = getResourceContext()->getFeatureContext()->getMaterialDir();
        std::string reshapeDir = materialDir + "/ReshapeResource.bundle/FaceReshape_V2";
        bef_effect_ai_set_reshape_face(m_renderMangerHandle, reshapeDir.c_str());
        bef_effect_ai_update_reshape_face_intensity(m_renderMangerHandle, 0.0f, 0.0f);
    }
    else
    {
        bef_effect_ai_set_reshape_face(m_renderMangerHandle, "");
    }
}

void EffectHandle::setBeautyOn(bool checked) {
    if (checked)
    {
        std::string materialDir = getResourceContext()->getFeatureContext()->getMaterialDir();
        std::string beautyDir = materialDir + "/BeautyResource.bundle/IESBeauty";
        bef_effect_ai_set_beauty(m_renderMangerHandle, beautyDir.c_str());
        bef_effect_ai_set_intensity(m_renderMangerHandle, BEF_INTENSITY_TYPE_BEAUTY_SMOOTH, 0.0f);
        bef_effect_ai_set_intensity(m_renderMangerHandle, BEF_INTENSITY_TYPE_BEAUTY_WHITEN, 0.0f);
        bef_effect_ai_set_intensity(m_renderMangerHandle, BEF_INTENSITY_TYPE_BEAUTY_SHARP, 0.0f);
    }
    else
    {
        bef_effect_ai_set_beauty(m_renderMangerHandle, "");
    }
}

void EffectHandle::setMakeupOn(bool checked) {
    if (checked)
    {
        std::string materialDir = getResourceContext()->getFeatureContext()->getMaterialDir();
        std::string makeupDir = materialDir + "/BuildinMakeup.bundle/Makeup";
        bef_effect_ai_set_buildin_makeup(m_renderMangerHandle, makeupDir.c_str());
        bef_effect_ai_set_intensity(m_renderMangerHandle, BEF_INTENSITY_TYPE_BUILDIN_LIP, 0.0);
        bef_effect_ai_set_intensity(m_renderMangerHandle, BEF_INTENSITY_TYPE_BUILDIN_BLUSHER, 0.0);
    }
    else
    {
        bef_effect_ai_set_buildin_makeup(m_renderMangerHandle, "");
    }
}


void EffectHandle::setIntensity(int key, float val) {
    bef_effect_ai_set_intensity(m_renderMangerHandle, key, val);
}

void EffectHandle::setReshapeIntensity(float val1, float val2) {
    bef_effect_ai_update_reshape_face_intensity(m_renderMangerHandle, val1, val2);
}

void EffectHandle::setSticker(const std::string &stickerPath) {
    std::string tempPath = getResourceContext()->getFeatureContext()->getStickerDir() + stickerPath;
    bef_effect_ai_set_effect(m_renderMangerHandle, tempPath.c_str());
}

void EffectHandle::setFilter(const std::string &filterPath) {
    std::string tempPath = getResourceContext()->getFeatureContext()->getFilterDir() + filterPath;
    bef_effect_ai_set_color_filter_v2(m_renderMangerHandle, tempPath.c_str());
}