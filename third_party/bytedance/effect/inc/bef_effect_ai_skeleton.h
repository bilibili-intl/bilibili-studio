// Copyright (C) 2019 Beijing Bytedance Network Technology Co., Ltd.
#ifndef BEF_EFFECT_AI_SKELETON_H
#define BEF_EFFECT_AI_SKELETON_H

#include "bef_effect_ai_public_define.h"

#if defined(__ANDROID__) || defined(TARGET_OS_ANDROID)
#include<jni.h>
#endif

#define BEF_AI_MAX_SKELETON_POINT_NUM 18
#define BEF_AI_MAX_SKELETON_NUM  2


typedef struct bef_ai_skeleton_point_info_st {
    float x; // ��Ӧ cols, ��Χ�� [0, width] ֮��
    float y; // ��Ӧ rows, ��Χ�� [0, height] ֮��
    bool is_detect; // �����ֵΪ 0, �� x,y ������
} bef_ai_skeleton_point_info;

typedef struct bef_ai_skeleton_info_st {
    bef_ai_skeleton_point_info      keyPointInfos[BEF_AI_MAX_SKELETON_POINT_NUM];     // ��⵽�Ĺ�����Ϣ
    bef_ai_rect  skeletonRect; // ����rect
} bef_ai_skeleton_info;

typedef struct bef_ai_skeleton_result_st {
    bef_ai_rotate_type orient;
    int body_count;
    bef_ai_skeleton_info body[BEF_AI_MAX_SKELETON_NUM];
} bef_ai_skeleton_result;

/**
 * @brief ���������ľ��
 * @param [in] model_path ģ���ļ�·��
 * @param [out] handle    Created skeleton handle
 *                        �����Ĺ������
 * @return If succeed return BEF_RESULT_SUC, other value please see bef_effect_ai_public_define.h
 *         �ɹ����� BEF_RESULT_SUC, ʧ�ܷ�����Ӧ������, ������ο� bef_effect_ai_public_define.h
 */
BEF_SDK_API bef_effect_result_t
bef_effect_ai_skeleton_create(
                             const char *model_path,
                             bef_effect_handle_t *handle
                             );

/**
 * @brief ����ؼ�����
 * @param [in] handle Created skeleton handle
 *                    �Ѵ����Ĺ������
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
 * @param [in] orientation  Image orientation
 *                          ����ͼ���ת�򣬾�����ο� bef_effect_ai_public_define.h �е� bef_rotate_type
 * @param [in, out] skeleton_info_num ����Ϊ NULL�� �������*skeleton_infoΪNULL���������������壬�᷵�ؼ��Ĺ�������
 *                                ���*skeleton_info��ΪNULL����ʾskeleton_info�ĳ��ȣ�
 *                                ��� skeleton_info_num ���� *skeleton_info���ȣ��� ����*skeleton_info����
 * @param [in, out] skeleton_info skeleton_info����Ϊnullptr,�������*skeleton_infoΪNULL�����ڲ�������ڴ棬��Ҫ������ʹ��delete[] (*skeleton_info)�ͷ�
 *                                ���*skeleton_info��ΪNULL���򳤶�Ϊ*skeleton_info_num��
 *                                ����ݼ����������*skeleton_info_num����������
 * @return If succeed return BEF_RESULT_SUC, other value please see bef_effect_ai_public_define.h
 *         �ɹ����� BEF_RESULT_SUC, ʧ�ܷ�����Ӧ������, ������ο� bef_effect_ai_public_define.h
 */
BEF_SDK_API bef_effect_result_t
bef_effect_ai_skeleton_detect(
                             bef_effect_handle_t handle,
                             const unsigned char *image,
                             bef_ai_pixel_format pixel_format,
                             int image_width,
                             int image_height,
                             int image_stride,
                             bef_ai_rotate_type orientation,
                             int *skeleton_info_num,
                             bef_ai_skeleton_info **skeleton_info
                             );


/**
 * @brief set detection inputSize ���ü���㷨������ߴ�
 * ��������ã�Ĭ��width = 128 height = 224
 * @param [in] handle Created skeleton handle
 *                      �Ѵ����Ĺ������
 * @param [in] width Skeleton detect  width
 *                      ���
 * @param [in] height Skeleton detect network height
 *                      �߶�
 * @return If succeed return BEF_RESULT_SUC, other value please refer bef_effect_ai_public_define.h
 *         �ɹ����� BEF_RESULT_SUC, ʧ�ܷ�����Ӧ������, ������ο� bef_effect_ai_public_define.h
 */
BEF_SDK_API bef_effect_result_t
bef_effect_ai_skeleton_set_detection_inputsize(
        bef_effect_handle_t handle,
        int width,
        int height
);
/**
 * @brief set tracking inputsize ���ø����㷨������ߴ�
 * ��������ã�Ĭ��width = 144 height = 192
 * @param [in] handle Created skeleton handle
 *                      �Ѵ����Ĺ������
 * @param [in] width Skeleton detect  width
 *                      ���
 * @param [in] height Skeleton detect network height
 *                      �߶�
 * @return If succeed return BEF_RESULT_SUC, other value please refer bef_effect_ai_public_define.h
 *         �ɹ����� BEF_RESULT_SUC, ʧ�ܷ�����Ӧ������, ������ο� bef_effect_ai_public_define.h
 */
BEF_SDK_API bef_effect_result_t
bef_effect_ai_skeleton_set_tracking_inputsize(
        bef_effect_handle_t handle,
        int width,
        int height
);

/**
 * @brief ������������ ���������Ϊ BEF_MAX_SKELETON_NUM Ϊ 2
 * @param [in] handle Created skeleton handle
 *                      �Ѵ����Ĺ������
 * @param [in] max_skeleton_num ���ü�����g������ range in [1, 2]
 * @return If succeed return BEF_RESULT_SUC, other value please refer bef_effect_ai_public_define.h
 *         �ɹ����� BEF_RESULT_SUC, ʧ�ܷ�����Ӧ������, ������ο� bef_effect_ai_public_define.h
 */
BEF_SDK_API bef_effect_result_t
bef_effect_ai_skeleton_set_targetnum(
                                bef_effect_handle_t handle,
                                int max_skeleton_num
                                );


/**
 * @param [in] handle Destroy the created skeleton handle
 *                    ���ٴ����Ĺ������
 */
BEF_SDK_API void
bef_effect_ai_skeleton_destroy(
                              bef_effect_handle_t handle
                              );





/**
 * @brief ����ؼ�����Ȩ
 * @param [in] handle Created skeleton detect handle
 *                    �Ѵ����Ĺ��������
 * @param [in] license ��Ȩ�ļ��ַ���
 * @param [in] length  ��Ȩ�ļ��ַ�������
 * @return If succeed return BEF_RESULT_SUC, other value please refer bef_effect_ai_public_define.h
 *         �ɹ����� BEF_RESULT_SUC, ��Ȩ��Ƿ����� BEF_RESULT_INVALID_LICENSE ������ʧ�ܷ�����Ӧ������, ������ο� bef_effect_ai_public_define.h
 */

#if defined(__ANDROID__) || defined(TARGET_OS_ANDROID)
BEF_SDK_API bef_effect_result_t bef_effect_ai_skeleton_check_license(JNIEnv* env, jobject context, bef_effect_handle_t handle, const char *licensePath);
#elif defined(__APPLE__)
BEF_SDK_API bef_effect_result_t bef_effect_ai_skeleton_check_license(bef_effect_handle_t handle, const char *licensePath);
#else
BEF_SDK_API bef_effect_result_t bef_effect_ai_skeleton_check_license(bef_effect_handle_t handle, const char *licensePath);
#endif

#endif // BEF_EFFECT_AI_SKELETON_H

