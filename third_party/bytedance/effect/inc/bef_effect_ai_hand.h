// Copyright (C) 2019 Beijing Bytedance Network Technology Co., Ltd.
#ifndef _BEF_EFFECT_HAND_DETECT_AI_H_
#define _BEF_EFFECT_HAND_DETECT_AI_H_


#if defined(__ANDROID__) || defined(TARGET_OS_ANDROID)
#include<jni.h>
#endif

#include "bef_effect_ai_public_define.h"

//bef_effect_public_hand_define
#define BEF_TT_HAND_GESTURE_HEART_A 0
#define BEF_TT_HAND_GESTURE_HEART_B 1
#define BEF_TT_HAND_GESTURE_HEART_C 2
#define BEF_TT_HAND_GESTURE_HEART_D 3
#define BEF_TT_HAND_GESTURE_OK 4
#define BEF_TT_HAND_GESTURE_HAND_OPEN 5
#define BEF_TT_HAND_GESTURE_THUMB_UP 6
#define BEF_TT_HAND_GESTURE_THUMB_DOWN 7
#define BEF_TT_HAND_GESTURE_ROCK 8
#define BEF_TT_HAND_GESTURE_NAMASTE 9
#define BEF_TT_HAND_GESTURE_PLAM_UP 10
#define BEF_TT_HAND_GESTURE_FIST 11
#define BEF_TT_HAND_GESTURE_INDEX_FINGER_UP 12
#define BEF_TT_HAND_GESTURE_DOUBLE_FINGER_UP 13
#define BEF_TT_HAND_GESTURE_VICTORY 14
#define BEF_TT_HAND_GESTURE_BIG_V 15
#define BEF_TT_HAND_GESTURE_PHONECALL 16
#define BEF_TT_HAND_GESTURE_BEG 17
#define BEF_TT_HAND_GESTURE_THANKS 18
#define BEF_TT_HAND_GESTURE_UNKNOWN 19
#define BEF_TT_HAND_GESTURE_CABBAGE 20
#define BEF_TT_HAND_GESTURE_THREE 21
#define BEF_TT_HAND_GESTURE_FOUR 22
#define BEF_TT_HAND_GESTURE_PISTOL 23
#define BEF_TT_HAND_GESTURE_ROCK2 24
#define BEF_TT_HAND_GESTURE_SWEAR 25
#define BEF_TT_HAND_GESTURE_HOLDFACE 26
#define BEF_TT_HAND_GESTURE_SALUTE 27
#define BEF_TT_HAND_GESTURE_SPREAD 28
#define BEF_TT_HAND_GESTURE_PRAY 29
#define BEF_TT_HAND_GESTURE_QIGONG 30
#define BEF_TT_HAND_GESTURE_SLIDE 31
#define BEF_TT_HAND_GESTURE_PALM_DOWN 32
#define BEF_TT_HAND_GESTURE_PISTOL2 33
#define BEF_TT_HAND_GESTURE_NARUTO1 34
#define BEF_TT_HAND_GESTURE_NARUTO2 35
#define BEF_TT_HAND_GESTURE_NARUTO3 36
#define BEF_TT_HAND_GESTURE_NARUTO4 37
#define BEF_TT_HAND_GESTURE_NARUTO5 38
#define BEF_TT_HAND_GESTURE_NARUTO7 39
#define BEF_TT_HAND_GESTURE_NARUTO8 40
#define BEF_TT_HAND_GESTURE_NARUTO9 41
#define BEF_TT_HAND_GESTURE_NARUTO10 42
#define BEF_TT_HAND_GESTURE_NARUTO11 43
#define BEF_TT_HAND_GESTURE_NARUTO12 44
#define BEF_TT_HAND_GESTURE_RAISE    47

#define BEF_MAX_HAND_NUM 2
#define BEF_HAND_KEY_POINT_NUM 22
#define BEF_HAND_KEY_POINT_NUM_EXTENSION 2


typedef struct bef_ai_hand_st {
    int id;                           ///< �ֵ�id
    bef_ai_rect rect;                      ///< �ֲ����ο�
    unsigned int action;              ///< �ֲ����� bef_hand_types[]��index [0--20)
    float rot_angle;                  ///< �ֲ���ת�Ƕȣ� �����ſ��ǱȽ�׼ȷ
    float score;                      ///< �ֲ��������Ŷ�
    float rot_angle_bothhand;  ///< ˫�ּн�
    // �ֲ��ؼ���, ���û�м�⵽������Ϊ0
    struct bef_ai_tt_key_point key_points[BEF_HAND_KEY_POINT_NUM];
    // �ֲ���չ�㣬���û�м�⵽������Ϊ0
    struct bef_ai_tt_key_point key_points_extension[BEF_HAND_KEY_POINT_NUM_EXTENSION];
    unsigned int seq_action;   // 0 ���û�����ж�������Ϊ0�� ����Ϊ��Чֵ
    unsigned char *segment;         ///< ���Ʒָ�mask ȡֵ��Χ 0��255 Ĭ��: nullptr
    int segment_width;              ///< ���Ʒָ�� Ĭ��: 0
    int segment_height;             ///< ���Ʒָ�� Ĭ��: 0
} bef_ai_hand, *ptr_bef_ai_hand;

/// @brief �����
typedef struct bef_ai_hand_info_st {
    bef_ai_hand p_hands[BEF_MAX_HAND_NUM];    ///< ��⵽���ֲ���Ϣ
    int hand_count;                       ///< ��⵽���ֲ���Ŀ��p_hands �����У�ֻ��hand_count���������Ч�ģ�
} bef_ai_hand_info, *ptr_bef_ai_hand_info;

typedef void *bef_ai_hand_sdk_handle;

typedef enum {
    BEF_HAND_REFRESH_FRAME_INTERVAL = 1,      // ���ü��ˢ��֡��, �ݲ�֧��
    BEF_HAND_MAX_HAND_NUM = 2,                // ���������ֵĸ�����Ĭ��Ϊ1��Ŀǰ�������Ϊ2��
    BEF_HAND_DETECT_MIN_SIDE = 3,             // ���ü�����̱߳���, Ĭ��192
    BEF_HAND_CLS_SMOOTH_FACTOR = 4,           // ���÷���ƽ��������Ĭ��0.7�� ��ֵԽ�����Խ�ȶ�
    BEF_HAND_USE_ACTION_SMOOTH = 5,           // �����Ƿ�ʹ�����ƽ����Ĭ��1��ʹ�����ƽ������ʹ��ƽ��������Ϊ0
    BEF_HAND_ALGO_LOW_POWER_MODE = 6,         // ����ģʽ��Ĭ���߸߼��İ汾�����
    BEF_HAND_ALGO_AUTO_MODE = 7,              // ����ģʽ��Ĭ���߸߼��İ汾�����
    // �������Ϊ HAND_ALGO_AUTO_MODE ģʽ����������²����������㷨��������ֵ
    BEF_HAND_ALGO_TIME_ELAPSED_THRESHOLD = 8, // �㷨��ʱ��ֵ��Ĭ��Ϊ 20ms
    BEF_HAND_ALGO_MAX_TEST_FRAME = 9,         // ��������ʱ�����㷨��ִ�еĴ���, Ĭ���� 150 ��
    BEF_HAND_IS_USE_DOUBLE_GESTURE = 10,      // �����Ƿ�ʹ��˫�����ƣ� Ĭ��Ϊtrue
    BEF_HNAD_ENLARGE_FACTOR_REG = 11,         // ���ûع�ģ�͵������ʼ��ķŴ����
    BEF_HAND_NARUTO_GESTURE = 12,             // ����֧�ֻ�Ӱ�������ƣ�Ĭ��Ϊfalse�������������֧�ְ�����Ӱ���ڵ�45������ʶ��
} bef_ai_hand_param_type;

typedef enum {
    BEF_AI_HAND_MODEL_DETECT = 0x0001,       // ����֣��������
    BEF_AI_HAND_MODEL_BOX_REG = 0x0002,      // ����ֿ򣬱������
    BEF_AI_HAND_MODEL_GESTURE_CLS = 0x0004,  // ���Ʒ��࣬��ѡ
    BEF_AI_HAND_MODEL_KEY_POINT = 0x0008,    // �ֹؼ��㣬��ѡ
    BEF_AI_HAND_MODEL_SEGMENT = 0x0010,    // �ɼ��Ե㣬��ѡ
} bef_ai_hand_model_type;


/**
 * @brief ����������������
 * @param [out] handle Created hand detect handle
 *                     ���������ּ����
 * @param [unsigned int] Ŀǰ��Ч
 * @return If succeed return BEF_RESULT_SUC, other value please see bef_effect_ai_public_define.h
 *         �ɹ����� BEF_RESULT_SUC, ʧ�ܷ�����Ӧ������, ������ο� bef_effect_ai_public_public_define.h
 */
BEF_SDK_API bef_effect_result_t
bef_effect_ai_hand_detect_create(
                              bef_ai_hand_sdk_handle *handle,
                              unsigned int config
                              );

/**
 * @brief ������ͼƬ
 * @param [in] handle Created hand detect handle
 *                    �Ѵ��������ּ����
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
 *                         ����ͼ���ת�򣬾�����ο� bef_effect_ai_public_public_define.h �е� bef_ai_rotate_type
 * @param [in] detection_config �������ģ�飬Ϊ hand_model_type �İ�λ�������Ŀǰֻ��HAND_MODEL_GESTURE_CLS �� HAND_MODEL_KEY_POINT �ǿ�ѡ��
 * @return If succeed return BEF_RESULT_SUC, other value please see bef_effect_ai_public_public_define.h
 *         �ɹ����� BEF_RESULT_SUC, ʧ�ܷ�����Ӧ������, ������ο� bef_effect_ai_public_public_define.h
 */
BEF_SDK_API bef_effect_result_t
bef_effect_ai_hand_detect(
                       bef_ai_hand_sdk_handle handle,
                       const unsigned char *image,
                       bef_ai_pixel_format pixel_format,
                       int image_width,
                       int image_height,
                       int image_stride,
                       bef_ai_rotate_type orientation,
                       unsigned long long detection_config,
                       bef_ai_hand_info *p_hand_info,
                       int delayframecount
                       );

BEF_SDK_API bef_effect_result_t
bef_effect_ai_hand_detect_setmodel(bef_effect_handle_t handle,
                                   bef_ai_hand_model_type type,
                                   const char * strModelPath);


BEF_SDK_API bef_effect_result_t
bef_effect_ai_hand_detect_setparam(bef_effect_handle_t handle,
                                   bef_ai_hand_param_type type,
                                   float value);


/**
 * @param [in] handle Destroy the created hand detect handle
 *                    ���ٴ��������ּ����
 */
BEF_SDK_API void
bef_effect_ai_hand_detect_destroy(
                            bef_ai_hand_sdk_handle handle
                               );


/**
 * @brief ���ּ����Ȩ
 * @param [in] handle Created hand detect handle
 *                    �Ѵ��������ּ����
 * @param [in] license ��Ȩ�ļ��ַ���
 * @param [in] length  ��Ȩ�ļ��ַ�������
 * @return If succeed return BEF_RESULT_SUC, other value please refer bef_effect_ai_public_public_define.h
 *         �ɹ����� BEF_RESULT_SUC, ��Ȩ��Ƿ����� BEF_RESULT_INVALID_LICENSE ������ʧ�ܷ�����Ӧ������, ������ο� bef_effect_ai_public_public_define.h
 */

#if defined(__ANDROID__) || defined(TARGET_OS_ANDROID)
BEF_SDK_API bef_effect_result_t bef_effect_ai_hand_check_license(JNIEnv* env, jobject context,
        bef_effect_handle_t handle, const char *licensePath);
#else
#ifdef __APPLE__
BEF_SDK_API bef_effect_result_t bef_effect_ai_hand_check_license(bef_effect_handle_t handle,
        const char *licensePath);
#else
BEF_SDK_API bef_effect_result_t bef_effect_ai_hand_check_license(bef_effect_handle_t handle,
const char *licensePath);
#endif
#endif


#endif // _BEF_EFFECT_FACE_DETECT_H_
