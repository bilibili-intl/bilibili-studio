// Copyright (C) 2019 Beijing Bytedance Network Technology Co., Ltd.
#ifndef _BEF_EFFECT_FACE_DETECT_AI_H_
#define _BEF_EFFECT_FACE_DETECT_AI_H_

#if defined(__ANDROID__) || defined(TARGET_OS_ANDROID)
#include<jni.h>
#endif

#include "bef_effect_ai_public_define.h"


//***************************** begin Create-Config *****************/
// Config when creating handle
#define BEF_DETECT_LARGE_MODEL 0x00100000  // 106ģ�ͳ�ʼ����������׼, ���ѷ���
#define BEF_DETECT_SMALL_MODEL 0x00200000  // 106ģ�ͳ�ʼ������������
#define BEF_DETECT_FACE_240_DETECT_FASTMODE \
0x00300000  // 240ģ�ͳ�ʼ������������
//**************************** end of Create-Config *****************/

//***************************** begin Mode-Config ******************/
#define BEF_DETECT_MODE_VIDEO_SLOW 0x00010000  // ��Ƶ��⣬�ܼ���С������,��ʼ��+Ԥ�����
#define BEF_DETECT_MODE_VIDEO 0x00020000  // ��Ƶ��⣬��ʼ��+Ԥ�����
#define BEF_DETECT_MODE_IMAGE 0x00040000  // ͼƬ��⣬��ʼ��+Ԥ�����
#define BEF_DETECT_MODE_IMAGE_SLOW \
0x00080000  // ͼƬ��⣬�������ģ��Ч�����ã��ܼ���С����������ʼ��+Ԥ�����
//***************************** enf of Mode-Config *****************/

//***************************** Begin Config-106 point and action **/
// for 106 key points detect
// NOTE ��ǰ�汾 ���졢ҡͷ����ͷ����üĬ�϶�������������ص�λ����Ч
#define BEF_FACE_DETECT 0x00000001  // ���106��
// ��������
#define BEF_EYE_BLINK 0x00000002   // գ��
#define BEF_MOUTH_AH 0x00000004    // ����
#define BEF_HEAD_YAW 0x00000008    // ҡͷ
#define BEF_HEAD_PITCH 0x00000010  // ��ͷ
#define BEF_BROW_JUMP 0x00000020   // ��ü
#define BEF_MOUTH_POUT 0x00000040  // ���

#define BEF_DETECT_FULL 0x0000007F  // ����������е���������ʼ��+Ԥ�����

#define BEF_EYE_BLINK_LEFT \
0x00000080  // ����գ�ۣ�ֻ������ȡ��Ӧ��action�����������Ȼ��գ��
#define BEF_EYE_BLINK_RIGHT \
0x00000100  // ����գ�ۣ�ֻ������ȡ��Ӧ��action�����������Ȼ��գ��
#define BEF_INDIAN_HEAD_ROLL \
0x00000200  // ӡ��ʽҡͷ��ֻ������ȡ��Ӧ��action�����������Ȼ��ҡͷ

//**************************** End Config-106 point and action *******/

//******************************* Begin Config-280 point *************/
// for 280 points
// NOTE: ���ڸ��˶������ԣ�üë���۾�����͹ؼ������һ��ģ���г�
#define TT_MOBILE_FACE_240_DETECT \
0x00000100  // �������ؼ���: üë, �۾�, ��ͣ���ʼ��+Ԥ�����
#define AI_BROW_EXTRA_DETECT TT_MOBILE_FACE_240_DETECT   // üë 13*2����
#define AI_EYE_EXTRA_DETECT TT_MOBILE_FACE_240_DETECT    // �۾� 22*2����
#define AI_MOUTH_EXTRA_DETECT TT_MOBILE_FACE_240_DETECT  // ��� 64����
#define AI_MOUTH_MASK_DETECT 0x00000300   // ��� mask
#define AI_IRIS_EXTRA_DETECT 0x00000800   // ��Ĥ 20*2����

#define TT_MOBILE_FACE_280_DETECT \
0x00000900  // �������ؼ���: üë, �۾�, ��ͣ���Ĥ����ʼ��+Ԥ�����
//******************************* End Config-280 point ***************/

#define TT_MOBILE_FORCE_DETECT 0x00001000  // ǿ����֡������⣬����ʾ���

//bef_effect_public_face_define
#define BEF_MAX_FACE_NUM  10



// �۾�,üë,�촽��ϸ�����, 280����
typedef struct bef_ai_face_ext_info_t {
    int eye_count;                  // ��⵽�۾�����
    int eyebrow_count;              // ��⵽üë����
    int lips_count;                 // ��⵽�촽����
    int iris_count;                 // ��⵽��Ĥ����
    
    bef_ai_fpoint eye_left[22];        // ���۹ؼ���
    bef_ai_fpoint eye_right[22];       // ���۹ؼ���
    bef_ai_fpoint eyebrow_left[13];    // ��üë�ؼ���
    bef_ai_fpoint eyebrow_right[13];   // ��üë�ؼ���
    bef_ai_fpoint lips[64];            // �촽�ؼ���
    bef_ai_fpoint left_iris[20];       // ���Ĥ�ؼ���
    bef_ai_fpoint right_iris[20];      // �Һ�Ĥ�ؼ���
    
//    unsigned char* face_mask;  // face_mask
//    int face_mask_size;        // face_mask_size
//    float* warp_mat;          // warp mat data ptr, size 2*3
} bef_ai_face_ext_info;


// ��106��ʹ��
typedef struct bef_ai_face_106_st {
    bef_ai_rect rect;                // �����沿�ľ�������
    float score;                  // ���Ŷ�
    bef_ai_fpoint points_array[106]; // ����106�ؼ��������
    float visibility_array[106];  // ��Ӧ����ܼ��ȣ���δ���ڵ�1.0, ���ڵ�0.0
    float yaw;                    // ˮƽת��,��ʵ������������
    float pitch;                  // ������,��ʵ�������ϸ�����
    float roll;                   // ��ת��,��ʵ������������
    float eye_dist;               // ���ۼ��
    int ID;                       // faceID: ÿ����⵽������ӵ��Ψһ��faceID.�������ٶ�ʧ�Ժ����±���⵽,����һ���µ�faceID
    unsigned int action;          // ����, ������ bef_ai_effect_face_detect.h ��
    unsigned int tracking_cnt;
} bef_ai_face_106, *p_bef_ai_face_106;


// @brief �����
typedef struct bef_ai_face_info_st {
    bef_ai_face_106 base_infos[BEF_MAX_FACE_NUM];          // ��⵽��������Ϣ
    bef_ai_face_ext_info extra_infos[BEF_MAX_FACE_NUM];    // �۾���üë���촽�ؼ���ȶ������Ϣ
    int face_count;                                     // ��⵽��������Ŀ
} bef_ai_face_info, *p_bef_ai_face_info;

// brief �㷨�����������
typedef struct bef_ai_face_image_st {
    bef_ai_face_106 base_info;         // ��⵽��������Ϣ
    bef_ai_face_ext_info extra_info;   // �۾���üë���촽�ؼ���ȶ������Ϣ
    unsigned int texture_id;          // ��������λ�õĽ�ͼ���Ѳ����ͷ���֡���������
    bef_ai_pixel_format pixel_format;  // ��ͼ��ʽ��Ŀǰ��ΪRGBA
    int image_width;                // ��ͼ���ؿ��
    int image_height;               // ��ͼ���ظ߶�
    int image_stride;               // ��ͼ�п��
} bef_ai_face_image_st, *p_bef_ai_face_image_st;



/**
 * @brief �����������ľ��
 * @param [in] config Config of face detect algorithm ��������㷨������
 * ͼƬģʽ�� BEF_DETECT_SMALL_MODEL| BEF_DETECT_MODE_IMAGE | BEF_DETECT_FULL
 * ��Ƶģʽ�� BEF_DETECT_SMALL_MODEL| BEF_DETECT_MODE_VIDEO | BEF_DETECT_FULL
 * ͼƬ����ģʽ�� BEF_DETECT_SMALL_MODEL| BEF_DETECT_MODE_IMAGE_SLOW | BEF_DETECT_FULL
 *
 * @param [in] strModelPath ģ���ļ�����·��
 * @param [out] handle Created face detect handle
 *                     ���������������
 * @return If succeed return BEF_RESULT_SUC, other value please see bef_effect_ai_public_define.h
 *         �ɹ����� BEF_RESULT_SUC, ʧ�ܷ�����Ӧ������, ������ο� bef_effect_ai_public_define.h
 */
BEF_SDK_API bef_effect_result_t
bef_effect_ai_face_detect_create(
                                 unsigned long long config,
                                 const char * strModelPath,
                                 bef_effect_handle_t *handle
                                 );

/**
 * @brief �������
 * @param [in] handle Created face detect handle
 *                    �Ѵ��������������
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
 * @param [in] orientation Image orientation
 *                         ����ͼ���ת�򣬾�����ο� bef_effect_ai_public_define.h �е� bef_rotate_type
 * @param [in] detect_config Config of face detect, for example, BEF_FACE_DETECT | BEF_DETECT_EYEBALL | BEF_BROW_JUMP
 *                           ���������ص�����
 * @return If succeed return BEF_RESULT_SUC, other value please see bef_effect_ai_public_define.h
 *         �ɹ����� BEF_RESULT_SUC, ʧ�ܷ�����Ӧ������, ������ο� bef_effect_ai_public_define.h
 */
BEF_SDK_API bef_effect_result_t
bef_effect_ai_face_detect(
                          bef_effect_handle_t handle,
                          const unsigned char *image,
                          bef_ai_pixel_format pixel_format,
                          int image_width,
                          int image_height,
                          int image_stride,
                          bef_ai_rotate_type orientation,
                          unsigned long long detect_config,
                          bef_ai_face_info *p_face_info
                          );

typedef enum {
    // ����ÿ������֡����һ���������(Ĭ��ֵ������ʱ24, ������ʱ24/3=8), ֵԽ��,
    // cpuռ����Խ��, ��������������ʱ��Խ��.
    BEF_FACE_PARAM_FACE_DETECT_INTERVAL = 1,  // default 24
    // �����ܼ�⵽�����������Ŀ(Ĭ��ֵ5),
    // �����ٵ������������ڸ�ֵʱ�����ٽ����µļ��. ��ֵԽ��, ����Ӧ��ʱԽ��.
    // ����ֵ���ܴ��� AI_MAX_FACE_NUM
    BEF_FACE_PARAM_MAX_FACE_NUM = 2,  // default 5
    // ��̬�����ܹ���������Ĵ�С����Ƶģʽǿ����4��ͼƬģʽ����ͨ������Ϊ8������С����������⼶��Խ�ߴ����ܼ���С��������ȡֵ��Χ��4��10
    BEF_FACE_PARAM_MIN_DETECT_LEVEL = 3,
    // base �ؼ���ȥ��������[1-30]
    BEF_FACE_PARAM_BASE_SMOOTH_LEVEL = 4,
    // extra �ؼ���ȥ��������[1-30]
    BEF_FACE_PARAM_EXTRA_SMOOTH_LEVEL = 5,
    // ��� mask ȥ���������� [0-1], Ĭ��0�� ƽ��Ч�����ã��ٶȸ���
    BEF_FACE_PARAM_MASK_SMOOTH_TYPE = 6,
} bef_face_detect_type;


/**
 * @brief Set face detect parameter based on type ��������������ز���
 * @param [in] handle Created face detect handle
 *                    �Ѵ��������������
 * @param [in] type Face detect type that needs to be set, check bef_face_detect_type for the detailed
 *                  ��Ҫ���õ����������ͣ��ɲο� bef_face_detect_type
 * @param [in] value Type value, check bef_face_detect_type for the detailed
 *                   ����ֵ, ��������� bef_face_detect_type ö���е�˵��
 * @return If succeed return BEF_RESULT_SUC, other value please refer bef_effect_ai_public_define.h
 *         �ɹ����� BEF_RESULT_SUC, ʧ�ܷ�����Ӧ������, ������ο� bef_effect_ai_public_define.h
 */
BEF_SDK_API bef_effect_result_t
bef_effect_ai_face_detect_setparam(
                                   bef_effect_handle_t handle,
                                   bef_face_detect_type type,
                                   float value
                                   );
/*
 *@brief ��ʼ��handle
 *@param [in] config ָ��240ģ�͵�ģ�Ͳ���������240����280
 *Config-240��TT_MOBILE_FACE_240_DETECT
 *Config-280��TT_MOBILE_FACE_280_DETECT
 *Config-240 ����ģʽ, TT_MOBILE_FACE_240_DETECT | TT_MOBILE_FACE_240_DETECT_FASTMODE
 *Config-280 ����ģʽ, TT_MOBILE_FACE_280_DETECT | TT_MOBILE_FACE_240_DETECT_FASTMODE
 *@param [in] param_path ģ�͵��ļ�·��
 */
BEF_SDK_API bef_effect_result_t
bef_effect_ai_face_detect_add_extra_model(
                                          bef_effect_handle_t handle,
                                          unsigned long long config, // ����config������240����280
                                          // Config-240��TT_MOBILE_FACE_240_DETECT
                                          // Config-280��TT_MOBILE_FACE_280_DETECT
                                          // Config-240 ����ģʽ, TT_MOBILE_FACE_240_DETECT | TT_MOBILE_FACE_240_DETECT_FASTMODE
                                          // Config-280 ����ģʽ, TT_MOBILE_FACE_280_DETECT | TT_MOBILE_FACE_240_DETECT_FASTMODE
                                          const char *param_path
                                          );

/**
 * @param [in] handle Destroy the created face detect handle
 *                    ���ٴ��������������
 */
BEF_SDK_API void
bef_effect_ai_face_detect_destroy(
                                  bef_effect_handle_t handle
                                  );

/**
 * @brief ���������Ȩ
 * @param [in] handle Created face detect handle
 *                    �Ѵ��������������
 * @param [in] license ��Ȩ�ļ��ַ���
 * @param [in] length  ��Ȩ�ļ��ַ�������
 * @return If succeed return BEF_RESULT_SUC, other value please refer bef_effect_ai_public_define.h
 *         �ɹ����� BEF_RESULT_SUC, ��Ȩ��Ƿ����� BEF_RESULT_INVALID_LICENSE ������ʧ�ܷ�����Ӧ������, ������ο� bef_effect_ai_public_define.h
 */
#if defined(__ANDROID__) || defined(TARGET_OS_ANDROID)
BEF_SDK_API bef_effect_result_t bef_effect_ai_face_check_license(JNIEnv *env, jobject context,
                                                                 bef_effect_handle_t handle,
                                                                 const char *licensePath);
#elif defined(__APPLE__)
BEF_SDK_API bef_effect_result_t bef_effect_ai_face_check_license(bef_effect_handle_t handle, const char *licensePath);
#else
BEF_SDK_API bef_effect_result_t bef_effect_ai_face_check_license(bef_effect_handle_t handle, const char *licensePath);
#endif


#endif // _BEF_EFFECT_FACE_DETECT_AI_H_
