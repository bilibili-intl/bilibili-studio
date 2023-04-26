#ifndef BE_DETECTOR_H
#define BE_DETECTOR_H

#include "bef_effect_ai_public_define.h"
#include "bef_effect_ai_face_detect.h"
#include "bef_effect_ai_skeleton.h"
#include "bef_effect_ai_face_attribute.h"
#include "bef_effect_ai_hand.h"
#include "bef_effect_ai_portrait_matting.h"
#include "bef_effect_ai_light_classify.h"

#include "be_feature_context.h"

class BEDectector
{
public:
    BEDectector();
    ~BEDectector();

    void setWidthAndHeight(int width, int height, int bytePerRow);
    void initializeDetector();
    void releaseDetector();

    bef_effect_result_t detectFace(bef_ai_face_info *faceInfo, unsigned char *buffer, bef_ai_pixel_format format, bef_ai_rotate_type ori);
    bef_effect_result_t detectFace280(bef_ai_face_info *faceInfo, unsigned char *buffer, bef_ai_pixel_format format, bef_ai_rotate_type ori);
    bef_effect_result_t detectFaceAttribute(bef_ai_face_attribute_result *faceAttrResult, unsigned char *buffer, bef_ai_face_106 *faceInfo, int faceCount, bef_ai_pixel_format format);
    bef_effect_result_t detectSkeleton(bef_skeleton_info * skeletonInfo, unsigned char* buffer, int *validCount,  bef_ai_pixel_format format, bef_ai_rotate_type ori);
    bef_effect_result_t detectHand(bef_ai_hand_info *handInfo, unsigned char *buffer, bef_ai_pixel_format format, bef_ai_rotate_type ori);
   /* unsigned char * detectHairParse( unsigned char *buffer, bef_ai_pixel_format format, bef_ai_rotate_type ori, int *size);*/
    bef_effect_result_t detectPortraitMatting(unsigned char *buffer, bef_ai_pixel_format format, bef_ai_rotate_type ori, unsigned char **mask, int *size);

    bef_effect_result_t  classifyLight(unsigned char *buffer, bef_ai_pixel_format format, bef_ai_rotate_type ori, bef_ai_light_classify_info *lightClassifyInfo);
private:
    void initializeFaceDetectHandle();
    void initializeSkeletonHandle();
    void initializeHandDetectHandle();
    //void initializeHairParseHandle();
    void initializePortraitMattingHandle();
	void initializeLightClassifyHandle();
private:
    int m_width;
    int m_height;
    int m_bytePerRow;


    bef_effect_handle_t m_faceDetectHandle = 0;
    bef_effect_handle_t m_faceAttributeHandle = 0;
    bef_effect_handle_t m_skeletonHandle = 0;
//    bef_effect_handle_t m_hairParseHandle = 0;
    bef_effect_handle_t m_portraitMattingHandle = 0;
    bef_effect_handle_t m_lightClassifyHandle = 0;

    bef_ai_hand_sdk_handle m_handDetectHandle = 0;

    BEFeatureContext *m_featureContext = nullptr;    
};

#endif //BE_DETECTOR_H
