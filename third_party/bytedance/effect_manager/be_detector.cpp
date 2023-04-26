#include "be_util.h"
#include "be_resource_context.h"
#include "be_detector.h"
#include "be_defines.h"

BEDectector::BEDectector()
{
}


BEDectector::~BEDectector()
{
}

void BEDectector::setWidthAndHeight(int width, int height, int bytePerRow) {
    m_width = width;
    m_height = height;
    m_bytePerRow = bytePerRow;
}

void BEDectector::initializeDetector() {
    m_featureContext = getResourceContext()->getFeatureContext();
    initializeSkeletonHandle();
    initializeFaceDetectHandle();
    initializeHandDetectHandle();
    //initializeHairParseHandle();
    initializePortraitMattingHandle();
    initializeLightClassifyHandle();
}

void BEDectector::releaseDetector() {
    bef_effect_ai_skeleton_destroy(m_skeletonHandle);
    bef_effect_ai_face_detect_destroy(m_faceDetectHandle);
    bef_effect_ai_face_attribute_destroy(m_faceAttributeHandle);
    bef_effect_ai_hand_detect_destroy(m_handDetectHandle);
    //bef_effect_ai_hairparser_destroy(m_hairParseHandle);
    bef_effect_ai_portrait_matting_destroy(m_portraitMattingHandle);
    bef_effect_ai_light_classify_destroy(m_lightClassifyHandle);
}

bef_effect_result_t BEDectector::detectFace(bef_ai_face_info *faceInfo, unsigned char *buffer, bef_ai_pixel_format format, bef_ai_rotate_type ori) {
    ScopeTimer t(__FUNCTION__);
    bef_effect_result_t ret = bef_effect_ai_face_detect(m_faceDetectHandle, buffer, format, m_width, m_height, m_bytePerRow, ori, BEF_DETECT_MODE_VIDEO | BEF_DETECT_FULL, faceInfo);
    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::detectFace bef_effect_ai_face_detect");
    return ret;
}

bef_effect_result_t BEDectector::detectFace280(bef_ai_face_info *faceInfo, unsigned char *buffer, bef_ai_pixel_format format, bef_ai_rotate_type ori) {
    ScopeTimer t(__FUNCTION__);
    bef_effect_result_t ret = bef_effect_ai_face_detect(m_faceDetectHandle, buffer, format, m_width, m_height, m_bytePerRow, ori, BEF_DETECT_MODE_VIDEO | BEF_DETECT_FULL | TT_MOBILE_FACE_280_DETECT, faceInfo);
    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::detectFace bef_effect_ai_face_detect");
    return ret;
}

bef_effect_result_t BEDectector::detectFaceAttribute(bef_ai_face_attribute_result *faceAttrResult, unsigned char *buffer, bef_ai_face_106 *faceInfo, int faceCount, bef_ai_pixel_format format) {
    ScopeTimer t(__FUNCTION__);
    bef_effect_result_t ret;
    unsigned long long attriConfig = BEF_FACE_ATTRIBUTE_AGE | BEF_FACE_ATTRIBUTE_HAPPINESS | BEF_FACE_ATTRIBUTE_EXPRESSION | BEF_FACE_ATTRIBUTE_GENDER
        | BEF_FACE_ATTRIBUTE_RACIAL | BEF_FACE_ATTRIBUTE_ATTRACTIVE;
    if (faceCount == 1) {
        ret = bef_effect_ai_face_attribute_detect(m_faceAttributeHandle, buffer, format, m_width, m_height, m_bytePerRow, faceInfo, attriConfig, &faceAttrResult->attr_info[0]);
        CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::detectFaceAttribute bef_effect_ai_face_attribute_detect");
    }
    else
    {
        ret = bef_effect_ai_face_attribute_detect_batch(m_faceAttributeHandle, buffer, format, m_width, m_height, m_bytePerRow, faceInfo, faceCount, attriConfig, faceAttrResult);
        CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::detectFaceAttribute bef_effect_ai_face_attribute_detect_batch");
    }
    return ret;
}

bef_effect_result_t BEDectector::detectSkeleton(bef_skeleton_info * skeletonInfo, unsigned char* buffer, int *validCount, bef_ai_pixel_format format, bef_ai_rotate_type ori) {
    ScopeTimer t(__FUNCTION__);
    bef_effect_result_t ret = bef_effect_ai_skeleton_detect(m_skeletonHandle, buffer, format, m_width, m_height, m_bytePerRow, ori, validCount, &skeletonInfo);
    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::detectSkeleton bef_effect_ai_skeleton_detect");
    return ret;
}


bef_effect_result_t BEDectector::detectHand(bef_ai_hand_info *handInfo, unsigned char *buffer, bef_ai_pixel_format format, bef_ai_rotate_type ori) {
    ScopeTimer t(__FUNCTION__);
    bef_effect_result_t ret;
    ret = bef_effect_ai_hand_detect(m_handDetectHandle, buffer, format, m_width, m_height, m_bytePerRow, ori,
        BEF_HAND_MODEL_DETECT | BEF_HAND_MODEL_BOX_REG | BEF_HAND_MODEL_GESTURE_CLS | BEF_HAND_MODEL_KEY_POINT, handInfo, 0);
    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::detectHand bef_effect_ai_hand_detect");
    return ret;
}

//unsigned char * BEDectector::detectHairParse(unsigned char *buffer, bef_ai_pixel_format format, bef_ai_rotate_type ori, int *size) {
//    ScopeTimer t(__FUNCTION__);
//    bef_effect_result_t ret;
//    ret = bef_effect_ai_hairparser_get_output_shape(m_hairParseHandle, size, size + 1, size + 2);
//    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::detectHairParse bef_effect_ai_hairparser_get_output_shape");
//
//    unsigned char *dstAlpha = new unsigned char[size[0] * size[1] * size[2]];
//    ret = bef_effect_ai_hairparser_do_detect(m_hairParseHandle, buffer, format, m_width, m_height, m_bytePerRow, ori, dstAlpha, false);
//    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::detectHairParse bef_effect_ai_hairparser_do_detect");
//    return dstAlpha;
//}

bef_effect_result_t BEDectector::detectPortraitMatting(unsigned char *buffer, bef_ai_pixel_format format, bef_ai_rotate_type ori, unsigned char **mask, int *size) {
    bef_effect_result_t ret;
    ScopeTimer t(__FUNCTION__);
    *mask = new unsigned char[m_width * m_height];
    bef_ai_matting_ret mattingRet = { *mask, m_width, m_height };

    ret = bef_effect_ai_portrait_matting_do_detect(m_portraitMattingHandle, buffer, format, m_width, m_height, m_bytePerRow, ori, false, &mattingRet);
    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::detectPortraitMatting bef_effect_ai_portrait_matting_do_detect");

    *size = mattingRet.width;
    *(size + 1) = mattingRet.height;
    *(size + 2) = 1;
    return ret;
}

bef_effect_result_t BEDectector::classifyLight(unsigned char *buffer, bef_ai_pixel_format format, bef_ai_rotate_type ori, bef_ai_light_classify_info *lightClassifyInfo) {
    ScopeTimer t(__FUNCTION__);
    if (lightClassifyInfo == nullptr)
    {
        return -1;
    }
    bef_effect_result_t ret;
    ret = bef_effect_ai_light_classify(m_lightClassifyHandle, buffer, format, m_width, m_height, m_bytePerRow, ori, lightClassifyInfo);
    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::classifyLight bef_effect_ai_light_classify");
    return ret;
}

void BEDectector::initializeFaceDetectHandle() {
    bef_effect_result_t ret = 0;
    //load face detect model
    ret = bef_effect_ai_face_detect_create(BEF_DETECT_SMALL_MODEL | BEF_DETECT_FULL | BEF_DETECT_MODE_VIDEO, m_featureContext->getFeatureModelPath(BE_FEATURE_FACE).c_str(), &m_faceDetectHandle);
    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializeFaceDetectHandle bef_effect_ai_face_detect_create");
    ret = bef_effect_ai_face_check_license(m_faceDetectHandle, m_featureContext->getLicensePath().c_str());
    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializeFaceDetectHandle bef_effect_ai_face_check_license");
    ret = bef_effect_ai_face_detect_setparam(m_faceDetectHandle, BEF_FACE_PARAM_FACE_DETECT_INTERVAL, 15);
    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializeFaceDetectHandle bef_effect_ai_face_detect_setparam");
    ret = bef_effect_ai_face_detect_setparam(m_faceDetectHandle, BEF_FACE_PARAM_MAX_FACE_NUM, 15);
    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializeFaceDetectHandle bef_effect_ai_face_detect_setparam");

    //load face detect model with 280 keypoints 
    ret = bef_effect_ai_face_detect_add_extra_model(m_faceDetectHandle, TT_MOBILE_FACE_280_DETECT, m_featureContext->getFeatureModelPath(BE_FEATURE_FACE_EXTRA).c_str());
    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializeFaceDetectHandle bef_effect_ai_face_detect_add_extra_model");

    // load face attribute detect model
    ret = bef_effect_ai_face_attribute_create(0, m_featureContext->getFeatureModelPath(BE_FEATURE_FACE_ATTRIBUTE).c_str(), &m_faceAttributeHandle);
    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializeFaceDetectHandle bef_effect_ai_face_attribute_create");
    ret = bef_effect_ai_face_attribute_check_license(m_faceAttributeHandle, m_featureContext->getLicensePath().c_str());
    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializeFaceDetectHandle bef_effect_ai_face_attribute_check_license");
}

void BEDectector::initializeSkeletonHandle() {
    bef_effect_result_t ret;
    ret = bef_effect_ai_skeleton_create(getResourceContext()->getFeatureContext()->getFeatureModelPath(BE_FEATURE_SKELETON).c_str(), &m_skeletonHandle);
    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initialSkeletonHandle bef_effect_ai_skeleton_create");
    ret = bef_effect_ai_skeleton_check_license(m_skeletonHandle, getResourceContext()->getFeatureContext()->getLicensePath().c_str());
    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initialSkeletonHandle bef_effect_ai_skeleton_check_license");
    ret = bef_effect_ai_skeleton_set_targetnum(m_skeletonHandle, 1);
    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initialSkeletonHandle bef_effect_ai_skeleton_set_targetnum");
}


void BEDectector::initializeHandDetectHandle() {
    bef_effect_result_t ret;

    // create hand detect handle
    ret = bef_effect_ai_hand_detect_create(&m_handDetectHandle, 0);
    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializeHandDetectHandle bef_effect_ai_hand_detect_create");

    // check license for hand model
    ret = bef_effect_ai_hand_check_license(m_handDetectHandle, m_featureContext->getLicensePath().c_str());
    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializeHandDetectHandle bef_effect_ai_hand_check_license");

    //init hand detecting  with model
    ret = bef_effect_ai_hand_detect_setmodel(m_handDetectHandle, BEF_HAND_MODEL_DETECT, m_featureContext->getFeatureModelPath(BE_FEATURE_HAND_DETECT).c_str());
    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializeHandDetectHandle bef_effect_ai_hand_detect_setmodel");

    //init hand box detecting  with model
    ret = bef_effect_ai_hand_detect_setmodel(m_handDetectHandle, BEF_HAND_MODEL_BOX_REG, m_featureContext->getFeatureModelPath(BE_FEATURE_HAND_BOX).c_str());
    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializeHandDetectHandle bef_effect_ai_hand_detect_setmodel");

    //init hand gesture detecting with model
    ret = bef_effect_ai_hand_detect_setmodel(m_handDetectHandle, BEF_HAND_MODEL_GESTURE_CLS, m_featureContext->getFeatureModelPath(BE_FEATURE_HAND_GESTURE).c_str());
    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializeHandDetectHandle bef_effect_ai_hand_detect_setmodel");

    // init hand key point detecting with model
    ret = bef_effect_ai_hand_detect_setmodel(m_handDetectHandle, BEF_HAND_MODEL_KEY_POINT, m_featureContext->getFeatureModelPath(BE_FEATURE_HAND_KEY_POINT).c_str());
    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializeHandDetectHandle bef_effect_ai_hand_detect_setmodel");

    // set max hand number for detect
    ret = bef_effect_ai_hand_detect_setparam(m_handDetectHandle, BEF_HAND_MAX_HAND_NUM, 1);
    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializeHandDetectHandle bef_effect_ai_hand_detect_setparam");


    ret = bef_effect_ai_hand_detect_setparam(m_handDetectHandle, BEF_HAND_NARUTO_GESTURE, 1);
    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializeHandDetectHandle bef_effect_ai_hand_detect_setparam");

    //set the enlarge ratio of initial rect for regression model
    ret = bef_effect_ai_hand_detect_setparam(m_handDetectHandle, BEF_HNAD_ENLARGE_FACTOR_REG, 2.0);
    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializeHandDetectHandle bef_effect_ai_hand_detect_setparam");


}

//void BEDectector::initializeHairParseHandle() {
//    bef_effect_result_t ret;
//    //create hair parse handle 
//    ret = bef_effect_ai_hairparser_create(&m_hairParseHandle);
//    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializeHairParseHandle bef_effect_ai_hairparser_create");
//
//    //check license for hair parse
//    ret = bef_effect_ai_hairparser_check_license(m_hairParseHandle, m_featureContext->getLicensePath().c_str());
//    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializeHairParseHandle bef_effect_ai_hairparser_check_license");
//
//    //init the handle with hair parse model
//    ret = bef_effect_ai_hairparser_init_model(m_hairParseHandle, m_featureContext->getFeatureModelPath(BE_FEATURE_HAIRPARSE).c_str());
//    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializeHairParseHandle bef_effect_ai_hairparser_init_model");
//
//    //set param width = 128, height = 224, the others set true
//    ret = bef_effect_ai_hairparser_set_param(m_hairParseHandle, 128, 224, true, true);
//    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializeHairParseHandle bef_effect_ai_hairparser_set_param");
//}

void BEDectector::initializePortraitMattingHandle() {
    bef_effect_result_t ret;
    // create portrait matting handle
    ret = bef_effect_ai_portrait_matting_create(&m_portraitMattingHandle);
    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializePortraitMattingHandle bef_effect_ai_portrait_matting_create");

    // check the  license of portrait matting
    ret = bef_effect_ai_matting_check_license(m_portraitMattingHandle, m_featureContext->getLicensePath().c_str());
    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializePortraitMattingHandle bef_effect_ai_matting_check_license");

    // initialize the handle with portrait matting model
    ret = bef_effect_ai_portrait_matting_init_model(m_portraitMattingHandle, BEF_MP_LARGE_MODEL, m_featureContext->getFeatureModelPath(BE_FEATURE_PORTRAIT_MATTING).c_str());
    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializePortraitMattingHandle bef_effect_ai_portrait_matting_init_model");

    //set param of model
    ret = bef_effect_ai_portrait_matting_set_param(m_portraitMattingHandle, BEF_MP_EdgeMode, 1);
    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializePortraitMattingHandle bef_effect_ai_portrait_matting_set_param");

    //set param of model
    ret = bef_effect_ai_portrait_matting_set_param(m_portraitMattingHandle, BEF_MP_FrashEvery, 15);
    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializePortraitMattingHandle bef_effect_ai_portrait_matting_set_param");
}


void BEDectector::initializeLightClassifyHandle() {

    bef_effect_result_t ret;
    //create light classify handle 
    ret = bef_effect_ai_light_classify_create_handle(&m_lightClassifyHandle, m_featureContext->getFeatureModelPath(BE_FEATURE_LIGHT_CLASSIFY).c_str(), 5);
    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializeLightClassifyHandle bef_effect_ai_light_classify_create_handle");

    //check license for light classify
    ret = bef_effect_ai_light_classify_check_license(m_lightClassifyHandle, m_featureContext->getLicensePath().c_str());
    CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializeLightClassifyHandle bef_effect_ai_light_classify_check_license");
}