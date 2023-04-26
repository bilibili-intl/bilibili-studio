// Copyright (C) 2019 Beijing Bytedance Network Technology Co., Ltd.
#ifndef BEF_EFFECT_AI_PORTRAT_MATTING_H
#define BEF_EFFECT_AI_PORTRAT_MATTING_H

#include "bef_effect_ai_public_define.h"

#if defined(__ANDROID__) || defined(TARGET_OS_ANDROID)
#include<jni.h>
#endif

/*
 * @brief �Ƽ�����
 **/
struct bef_ai_recommend_config {
    int OutputMinSideLen = 128;
    int FrashEvery = 15;
    int EdgeMode = 1;
};
/*
 * @brief SDK����
 * edge_mode:
 *    �㷨�������������ñ߽��ģʽ
 *    - 0: ���ӱ߽�
 *    - 1: �ӱ߽�
 *    - 2: �ӱ߽�, ����, 2 �� 3 ���Բ�̫һ������Ч���ϲ�𲻴󣬿�����ȡһ��
 * fresh_every:
 *    �㷨���������õ��ö��ٴ�ǿ����Ԥ�⣬Ŀǰ���� 15 ����
 * MP_OutputMinSideLen:
 *    ���ض̱ߵĳ���, Ĭ��ֵΪ128, ��ҪΪ16�ı�����
 * MP_OutputWidth �����ã�ֻ��GetParam ����֮ǰ�ĵ���
 * MP_OutputHeight �����ã�ֻ��GetParam ����֮ǰ�Ľӿڣ�
 **/
enum bef_ai_matting_param_type {
    BEF_MP_EdgeMode = 0,
    BEF_MP_FrashEvery = 1,
    BEF_MP_OutputMinSideLen = 2,
    BEF_MP_OutputWidth = 3,
    BEF_MP_OutputHeight = 4,
};

/*
 * @brief ģ������ö��
 **/
enum bef_ai_matting_model_type {
    BEF_MP_LARGE_MODEL = 0,
    BEF_MP_SMALL_MODEL = 1,
};


/*
 * @brief ���ؽṹ�壬alpha �ռ���Ҫ���÷���������ڴ���ͷţ���֤��Ч�Ŀس����ڵ���widht*height
 * @note ��������Ĵ�С���̱߹̶���MP_OutputMinSideLen����ָ���Ĵ�С�����߱��ֳ�������ţ�
 *       ��������image_height > image_width: ��
 *                width = MP_OutputMinSideLen,
 *                height = (int)(1.0*MP_OutputMinSideLen/image_width*image_height);
 *                //������Ȳ�Ϊ16�ı�����������ȡ����16�ı���
 *                   if(height %16 > 0){
 *                      height = 16*(height/16)+16;
 }
 **/
struct bef_ai_matting_ret {
    unsigned char* alpha;  // alpha[i, j] ��ʾ�� (i, j) ��� mask Ԥ��ֵ��ֵλ��[0, 255] ֮��
    int width;             // alpha �Ŀ��
    int height;            // alpha �ĸ߶�
};

/**
 * @brief ���������ľ��
 * @param [out] handle    Created portrait_matting handle
 *                        �����Ĺ������
 * @return If succeed return BEF_RESULT_SUC, other value please see bef_effect_ai_public_define.h
 *         �ɹ����� BEF_RESULT_SUC, ʧ�ܷ�����Ӧ������, ������ο� bef_effect_ai_public_define.h
 */
BEF_SDK_API bef_effect_result_t
bef_effect_ai_portrait_matting_create(
                             bef_effect_handle_t *handle
                             );

/*
 * @brief ���ļ���ʼ��ģ�Ͳ���
 **/

BEF_SDK_API bef_effect_result_t
bef_effect_ai_portrait_matting_init_model(
                                      bef_effect_handle_t handle,
                                      bef_ai_matting_model_type type,
                                      const char* param_path);

/*
 * @brief ����SDK����
 **/
BEF_SDK_API bef_effect_result_t
bef_effect_ai_portrait_matting_set_param(
                                        bef_effect_handle_t handle,
                                        bef_ai_matting_param_type type,
                                        int value);

/*
 * @brief ��ȡSDK����
 **/
BEF_SDK_API bef_effect_result_t
bef_effect_ai_portrait_matting_get_param(
                                         bef_effect_handle_t handle,
                                         bef_ai_matting_param_type type,
                                         int *value);
/**
 *  @brief ��ȡ���mask�ĳߴ�
 *  @param handle ����ָ���
 *  @param width ����ͼ�Ŀ��
 *  @param height ����ͼ�ĸ߶�
 *  @param output_width[out] ���mask�Ŀ��
 *  @param output_height[out] ���mask�ĸ߶�
 *
 */
BEF_SDK_API bef_effect_result_t
bef_effect_ai_portrait_get_output_shape(bef_effect_handle_t handle,
                                        int width,
                                        int height,
                                        int *output_width,
                                        int *output_height);
/*
 * @brief ���п�ͼ����
 * @note ret �ṹͼ�ռ���Ҫ�ⲿ����
 **/
BEF_SDK_API bef_effect_result_t
bef_effect_ai_portrait_matting_do_detect(
                                         bef_effect_handle_t handle,
                                         const unsigned char *src_image_data,
                                         bef_ai_pixel_format pixel_format,
                                         int width,
                                         int height,
                                         int image_stride,
                                         bef_ai_rotate_type orient,
                                         bool need_flip_alpha,
                                         bef_ai_matting_ret *ret);


/*
 * @brief �ͷž��
 **/
BEF_SDK_API bef_effect_result_t
bef_effect_ai_portrait_matting_destroy(bef_effect_handle_t handle);


/**
 * @brief ����ָ���Ȩ
 * @param [in] handle Created portrait_matting  handle
 *                    �Ѵ����Ĺ��������
 * @param [in] license ��Ȩ�ļ��ַ���
 * @param [in] length  ��Ȩ�ļ��ַ�������
 * @return If succeed return BEF_RESULT_SUC, other value please refer bef_effect_ai_public_define.h
 *         �ɹ����� BEF_RESULT_SUC, ��Ȩ��Ƿ����� BEF_RESULT_INVALID_LICENSE ������ʧ�ܷ�����Ӧ������, ������ο� bef_effect_ai_public_define.h
 */

#if defined(__ANDROID__) || defined(TARGET_OS_ANDROID)
BEF_SDK_API bef_effect_result_t bef_effect_ai_matting_check_license(JNIEnv* env, jobject context, bef_effect_handle_t handle, const char *licensePath);
#elif defined(__APPLE__)
BEF_SDK_API bef_effect_result_t bef_effect_ai_matting_check_license(bef_effect_handle_t handle, const char *licensePath);
#else
BEF_SDK_API bef_effect_result_t bef_effect_ai_matting_check_license(bef_effect_handle_t handle, const char *licensePath);
#endif

#endif // BEF_EFFECT_AI_SKELETON_H

