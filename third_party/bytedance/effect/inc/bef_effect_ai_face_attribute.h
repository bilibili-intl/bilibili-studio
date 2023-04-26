// Copyright (C) 2019 Beijing Bytedance Network Technology Co., Ltd.
#ifndef _BEF_EFFECT_FACE_ATTRIBUTE_H_
#define _BEF_EFFECT_FACE_ATTRIBUTE_H_

#if defined(__ANDROID__) || defined(TARGET_OS_ANDROID)
#include<jni.h>
#endif

#include "bef_effect_ai_public_define.h"
#include "bef_effect_ai_face_detect.h"

typedef enum {
  BEF_FACE_ATTRIBUTE_AGE          = 0x00000001,     //����
  BEF_FACE_ATTRIBUTE_GENDER       = 0x00000002,     //�Ա�
  BEF_FACE_ATTRIBUTE_EXPRESSION   = 0x00000004,     //����
  BEF_FACE_ATTRIBUTE_ATTRACTIVE   = 0x00000008,     //��ֵ
  BEF_FACE_ATTRIBUTE_HAPPINESS    = 0x00000010,     //���ĳ̶�
  BEF_FACE_ATTRIBUTE_RACIAL       = 0x00000020,     //��ɫ
} bef_ai_face_attribute_types;

typedef enum {
  BEF_FACE_ATTRIBUTE_ForceDetect = 0x10000000,     //δ��ƽ���������ݣ����û��棬���л�����ͷʱ������֡���ұ仯ʱʹ��
                                //���ڴ����л�����ͷ�����ٵ�����ID ����������
} bef_ai_face_attribut_config;

/*
 *@brief ��ɫ���ö��
**/
typedef enum {
  BEF_FACE_ATTRIBUTE_WHITE = 0,                   //������
  BEF_FACE_ATTRIBUTE_YELLOW = 1,                  //������
  BEF_FACE_ATTRIBUTE_INDIAN = 2,                  //ӡ����
  BEF_FACE_ATTRIBUTE_BLACK = 3,                   //������
  BEF_FACE_ATTRIBUTE_NUM_RACIAL = 4,              //֧�ֵķ�ɫ����
}bef_ai_face_attribute_racial_type;

/*
 *@brief �������ö��
**/
typedef enum {
  BEF_FACE_ATTRIBUTE_ANGRY = 0,                   //����
  BEF_FACE_ATTRIBUTE_DISGUST = 1,                 //���
  BEF_FACE_ATTRIBUTE_FEAR = 2,                    //����
  BEF_FACE_ATTRIBUTE_HAPPY = 3,                   //����
  BEF_FACE_ATTRIBUTE_SAD = 4,                     //����
  BEF_FACE_ATTRIBUTE_SURPRISE = 5,                //�Ծ�
  BEF_FACE_ATTRIBUTE_NEUTRAL = 6,                 //ƽ��
  BEF_FACE_ATTRIBUTE_NUM_EXPRESSION = 7           //֧�ֵı������
}bef_ai_face_attribute_expression_type;

/*
 *@breif �����������Խṹ��
**/
typedef struct bef_ai_face_attribute_info {
  float age;                          // Ԥ�������ֵ�� ֵ��Χ��0��100��֮��
  float boy_prob;                     // Ԥ��Ϊ���Եĸ���ֵ��ֵ��Χ��0.0��1.0��֮��
  float attractive;                   // Ԥ�����ֵ��������Χ��0��100��֮��
  float happy_score;                  // Ԥ���΢Ц�̶ȣ���Χ��0��100��֮��
  bef_ai_face_attribute_expression_type exp_type;            // Ԥ��ı������
  float exp_probs[BEF_FACE_ATTRIBUTE_NUM_EXPRESSION];    // Ԥ���ÿ������ĸ��ʣ�δ��ƽ������
  bef_ai_face_attribute_racial_type racial_type;             // Ԥ��ķ�ɫ���
  float racial_probs[BEF_FACE_ATTRIBUTE_NUM_RACIAL];     // Ԥ���ÿ������ɫ�ĸ��ʣ�δ��ƽ������
  // ��������Ԥ���ֶ� Ŀǰû��ֵ����
  float real_face_prob;               // Ԥ�������������ĸ��ʣ��������ֵ��ܡ������ȷ���ʵ����
  float quality;                      // Ԥ��������������������Χ��0��100��֮��
  float arousal;                      // ������ǿ�ҳ̶�
  float valence;                      // ���������������̶�
  float sad_score;                    // ���ĳ̶�
  float angry_score;                  // �����̶�
  float surprise_score;               // �Ծ��ĳ̶�
  float mask_prob;                    // Ԥ������ֵĸ���
  float wear_hat_prob;                // ��ñ�ӵĸ���
  float mustache_prob;                // �к��ӵĸ���
  float lipstick_prob;                // Ϳ�ں�ĸ���
  float wear_glass_prob;              // ����ͨ�۾��ĸ���
  float wear_sunglass_prob;           // ��ī���ĸ���
  float blur_score;                   // ģ���̶�
  float illumination;                 // ����
} bef_ai_face_attribute_info;


typedef enum {
  // �����ص�����(�Ա����䡢��ɫ)����֡����Ĭ��ֵΪ12;
  BEF_FACE_ATTRIBUTE_IDRelatedDetectInterval = 1,
  // �������ص�����(���顢��ֵ��΢Ц�̶ȣ�����֡����Ĭ��ֵΪ1����ÿ֡��ʶ��
  // �����ֶΣ���ǰ�����裻
  BEF_FACE_ATTRIBUTE_DetectInterval = 2,
  // �������ص�����ʶ�����Ŷ��㹻��ʱ��ֹͣ��������ԣ������SDK�д洢���������أ����ⲻ��֪��
  // Ĭ��ֵΪ1����ʾ�򿪣�����Ϊ0,��ʾ�رգ�
  BEF_FACE_ATTRIBUTE_IDRelatedAccumulateResult = 3,
}bef_ai_face_attribute_param_config_type;

/*
 *@brief ����������Խṹ��
 *@param
 *      face_count ��Ч����������
**/
typedef struct bef_ai_face_attribute_result {
  bef_ai_face_attribute_info  attr_info[BEF_MAX_FACE_NUM];    //����������Խ������
  int face_count;                         //��Ч����������������ʾattr_info�е�ǰface_count����������Ч��
} bef_ai_face_attribute_result;



/**
 * @brief �����������Լ��ľ��
 * @param [in] config Config of face attribute detect algorithm
 *                    �������Լ���㷨������
 * @param [in] strModelPath ģ���ļ�����·��
 * @param [out] handle Created face attribute detect handle
 *                     �������������Լ����
 * @return If succeed return BEF_RESULT_SUC, other value please see bef_effect_base_define.h
 *         �ɹ����� BEF_RESULT_SUC, ʧ�ܷ�����Ӧ������, ������ο� bef_effect_base_define.h
 */
BEF_SDK_API bef_effect_result_t
bef_effect_ai_face_attribute_create(
  unsigned long long config,
  const char * strModelPath,
  bef_effect_handle_t *handle
);

/**
 * @brief �����������Լ��
 * @param [in] handle Created face attribute detect handle
 *                    �Ѵ������������Լ����
 * @param [in] image Image base address
 *                   ����ͼƬ������ָ��
 * @param [in] pixel_format Pixel format of input image
 *                          ����ͼƬ�ĸ�ʽ
 * @param [in] image_width  Image width
 *                          ����ͼ��Ŀ�� (������Ϊ��λ)
 * @param [in] image_height Image height
 *                          ����ͼ��ĸ߶� (������Ϊ��λ)
 * @param [in] image_stride Image stride in each row
 *                          ����ͼ��ÿһ�еĲ��� (������Ϊ��λ)
 * @param [in] ptr_base_info ���������
 * @param [in] detect_config Config of face detect, for example
 *                           ���������ص�����
 * @return If succeed return BEF_RESULT_SUC, other value please see bef_effect_base_define.h
 *         �ɹ����� BEF_RESULT_SUC, ʧ�ܷ�����Ӧ������, ������ο� bef_effect_base_define.h
 */
BEF_SDK_API bef_effect_result_t
bef_effect_ai_face_attribute_detect(
  bef_effect_handle_t handle,
  const unsigned char *image,
  bef_ai_pixel_format pixel_format,
  int image_width,
  int image_height,
  int image_stride,
  const bef_ai_face_106 *ptr_base_info,
  unsigned long long config,
  bef_ai_face_attribute_info *ptr_face_attribute_info
);

/**
 * @brief ����������Լ��
 * @param [in] handle Created face attribute detect handle
 *                    �Ѵ������������Լ����
 * @param [in] image Image base address
 *                   ����ͼƬ������ָ��
 * @param [in] pixel_format Pixel format of input image
 *                          ����ͼƬ�ĸ�ʽ
 * @param [in] image_width  Image width
 *                          ����ͼ��Ŀ�� (������Ϊ��λ)
 * @param [in] image_height Image height
 *                          ����ͼ��ĸ߶� (������Ϊ��λ)
 * @param [in] image_stride Image stride in each row
 *                          ����ͼ��ÿһ�еĲ��� (������Ϊ��λ)
 * @param [in] ptr_base_info ���������
 * @param [in] face_count �������������
 * @param [in] detect_config Config of face detect, for example
 *                           ���������ص�����
 * @return If succeed return BEF_RESULT_SUC, other value please see bef_effect_base_define.h
 *         �ɹ����� BEF_RESULT_SUC, ʧ�ܷ�����Ӧ������, ������ο� bef_effect_base_define.h
 */
BEF_SDK_API bef_effect_result_t
bef_effect_ai_face_attribute_detect_batch(
  bef_effect_handle_t handle,
  const unsigned char *image,
  bef_ai_pixel_format pixel_format,
  int image_width,
  int image_height,
  int image_stride,
  const bef_ai_face_106 *ptr_base_info,
  int face_count,
  unsigned long long config,
  bef_ai_face_attribute_result *ptr_face_attribute_result
);


/**
 * @brief Set face attribute detect parameter based on type 
 *     �����������Լ�����ز���
 * @param [in] handle Created face detect handle
 *                    �Ѵ��������������
 * @param [in] type Face detect type that needs to be set, check bef_face_detect_type for the detailed
 *                  ��Ҫ���õ����������ͣ��ɲο� bef_face_detect_type
 * @param [in] value Type value, check bef_face_detect_type for the detailed
 *                   ����ֵ, ��������� bef_face_detect_type ö���е�˵��
 * @return If succeed return BEF_RESULT_SUC, other value please refer bef_effect_base_define.h
 *         �ɹ����� BEF_RESULT_SUC, ʧ�ܷ�����Ӧ������, ������ο� bef_effect_base_define.h
 */
BEF_SDK_API bef_effect_result_t
bef_effect_ai_face_attribute_setparam(
  bef_effect_handle_t handle,
  bef_ai_face_attribute_param_config_type type,
  float value
);


/**
 * @param [in] handle Destroy the created face attribute detect handle
 *                    ���ٴ������������Լ����
 */
BEF_SDK_API void
bef_effect_ai_face_attribute_destroy(
  bef_effect_handle_t handle
);

/**
 * @brief �������Լ����Ȩ
 * @param [in] handle Created face attribute detect handle
 *                    �Ѵ��������������
 * @param [in] license ��Ȩ�ļ��ַ���
 * @param [in] length  ��Ȩ�ļ��ַ�������
 * @return If succeed return BEF_RESULT_SUC, other value please refer bef_effect_ai_public_define.h
 *         �ɹ����� BEF_RESULT_SUC, ��Ȩ��Ƿ����� BEF_RESULT_INVALID_LICENSE ������ʧ�ܷ�����Ӧ������, ������ο� bef_effect_ai_public_define.h
 */
#if defined(__ANDROID__) || defined(TARGET_OS_ANDROID)
BEF_SDK_API bef_effect_result_t bef_effect_ai_face_attribute_check_license(JNIEnv* env,
        jobject context, bef_effect_handle_t handle, const char *licensePath);
#elif defined(__APPLE__)
BEF_SDK_API bef_effect_result_t bef_effect_ai_face_attribute_check_license(bef_effect_handle_t handle,
        const char *licensePath);
#else
BEF_SDK_API bef_effect_result_t bef_effect_ai_face_attribute_check_license(bef_effect_handle_t handle,
        const char *licensePath);
#endif


#endif // _BEF_EFFECT_FACE_DETECT_AI_H_
