// Copyright (C) 2019 Beijing Bytedance Network Technology Co., Ltd.

#ifndef BEF_EFFECT_AI_LIGHT_CLASSIFY_H
#define BEF_EFFECT_AI_LIGHT_CLASSIFY_H

#if defined(__ANDROID__) || defined(TARGET_OS_ANDROID)
#include<jni.h>
#endif

#include "bef_effect_ai_public_define.h"

#define BEF_LIGHT_CLASSES 7

typedef enum {
	BEF_LIGHT_TYPE_INDOOR_YELLOW = 0,
	BEF_LIGHT_TYPE_INDOOR_WHITE,
	BEF_LIGHT_TYPE_INDOOR_WEAK,
	BEF_LIGHT_TYPE_SUNNY,
	BEF_LIGHT_TYPE_CLOUDY,
	BEF_LIGHT_TYPE_NIGHT,
	BEF_LIGHT_TYPE_BLACKLIGHT,
}bef_light_classify_type;

typedef struct bef_ai_light_classify_info_st {
	int selected_index;
	float prob;
	std::string name;
}bef_ai_light_classify_info;


BEF_SDK_API bef_effect_result_t
bef_effect_ai_light_classify_create_handle(bef_effect_handle_t *handle, const char* model_path, int fps = 5);

BEF_SDK_API bef_effect_result_t
bef_effect_ai_light_classify(bef_effect_handle_t handle,
	const unsigned char *image,
	bef_ai_pixel_format pixel_format,
	int image_width,
	int image_height,
	int image_stride,
	bef_ai_rotate_type orientation,
	bef_ai_light_classify_info *light_cls_info);


BEF_SDK_API void
bef_effect_ai_light_classify_destroy(bef_effect_handle_t handle);


/**
 * @brief ���߷�����Ȩ
 * @param [in] handle Created light classify handle
 *                    �Ѵ����Ĺ��߷�����
 * @param [in] license ��Ȩ�ļ��ַ���
 * @param [in] length  ��Ȩ�ļ��ַ�������
 * @return If succeed return BEF_RESULT_SUC, other value please refer bef_effect_ai_public_define.h
 *         �ɹ����� BEF_RESULT_SUC, ��Ȩ��Ƿ����� BEF_RESULT_INVALID_LICENSE ������ʧ�ܷ�����Ӧ������, ������ο� bef_effect_ai_public_define.h
 */
#if defined(__ANDROID__) || defined(TARGET_OS_ANDROID)
BEF_SDK_API bef_effect_result_t bef_effect_ai_light_classify_check_license(JNIEnv *env, jobject context,
	bef_effect_handle_t handle,
	const char *licensePath);
#elif defined(__APPLE__)
BEF_SDK_API bef_effect_result_t bef_effect_ai_light_classify_check_license(bef_effect_handle_t handle, const char *licensePath);
#else
BEF_SDK_API bef_effect_result_t bef_effect_ai_light_classify_check_license(bef_effect_handle_t handle, const char *licensePath);
#endif



#endif //BEF_EFFECT_AI_LIGHT_CLASSIFY_H

