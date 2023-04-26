// Copyright (C) 2019 Beijing Bytedance Network Technology Co., Ltd.
#ifndef EFFECT_SDK_BEF_EFFECT_AI_PUBLIC_DEFINE_H
#define EFFECT_SDK_BEF_EFFECT_AI_PUBLIC_DEFINE_H


#define BEF_INTENSITY_TYPE_NONE                 0
#define BEF_INTENSITY_TYPE_BEAUTY_WHITEN        1   // ����
#define BEF_INTENSITY_TYPE_BEAUTY_SMOOTH        2   // ĥƽ
#define BEF_INTENSITY_TYPE_FACE_SHAPE           3   // ����/����
#define BEF_INTENSITY_TYPE_BEAUTY_SHARP         9   // ��
#define BEF_INTENSITY_TYPE_GLOBAL_FILTER_V2     12  // �˾�
#define BEF_INTENSITY_TYPE_BUILDIN_LIP          17  // ��ɫ
#define BEF_INTENSITY_TYPE_BUILDIN_BLUSHER      18  // ����

//bef_framework_public_base_define
#ifdef WIN32
#	ifdef __cplusplus
#		ifdef _EFFECT_SDK_EXPORTS_
#			define BEF_SDK_API extern "C" __declspec(dllexport)
#		else
#			define BEF_SDK_API extern "C"
#		endif
#	else
#		ifdef _EFFECT_SDK_EXPORTS_
#			define BEF_SDK_API __attribute__((visibility ("default")))
#		else
#			define BEF_SDK_API
#		endif
#	endif
#else
#	ifdef __cplusplus
#		ifdef _EFFECT_SDK_EXPORTS_
#			define BEF_SDK_API extern "C" __attribute__((visibility ("default")))
#		else
#			define BEF_SDK_API extern "C"
#		endif
#	else
#		ifdef _EFFECT_SDK_EXPORTS_
#			define BEF_SDK_API __attribute__((visibility ("default")))
#		else
#			define BEF_SDK_API
#		endif
#	endif
#endif


typedef    short           int16_t;
typedef    int             int32_t;

typedef unsigned long long UINT64;
// def byted effect handle
typedef void *bef_effect_handle_t;

// def byted effect result
typedef int bef_effect_result_t;

typedef int effect_result;

// define bef_intensity_type
typedef int bef_intensity_type;


//bef_framework_public_constant_define
#define BEF_RESULT_SUC                       0  // �ɹ�����
#define BEF_RESULT_FAIL                     -1  // �ڲ�����
#define BEF_RESULT_FILE_NOT_FIND            -2  // �ļ�û�ҵ�

#define BEF_RESULT_INVALID_INTERFACE        -3  // �ӿ�δʵ��
#define BEF_RESULT_FILE_OPEN_FAILED         -4  // �ļ���ʧ��

#define BEF_RESULT_INVALID_EFFECT_HANDLE    -5  // ��Ч��Effect���
#define BEF_RESULT_INVALID_EFFECT_MANAGER   -6  // ��Ч��EffectManager
#define BEF_RESULT_INVALID_FEATURE_HANDLE   -7  // ��Ч��Feature���
#define BEF_RESULT_INVALID_FEATURE          -8  // ��Ч��Feature
#define BEF_RESULT_INVALID_RENDER_MANAGER   -9  // ��Ч��RenderManager

#define BEF_RESULT_ALG_FACE_106_CREATE_FAIL -22 // �������106�㷨����ʧ��
#define BEF_RESULT_ALG_FACE_280_CREATE_FAIL -23 // �������280�㷨����ʧ��
#define BEF_RESULT_ALG_FACE_PREDICT_FAIL    -24 // �������Ԥ��ʧ��

#define BEF_RESULT_ALG_HAND_CREATE_FAIL     -26 // ���������㷨ʧ��
#define BEF_RESULT_ALG_HAND_PREDICT_FAIL    -27 // �����㷨Ԥ��ʧ��

#define BEF_RESULT_ALG_LIGHT_CLASSIFY_CREATE_FAIL    -30 //���߷���������ʧ��
#define BEF_RESULT_ALG_LIGHT_CLASSIFY_FAIL    -31 //���߷���ʧ��

#define BEF_RESULT_INVALID_TEXTURE          -36 // ��Ч��texture
#define BEF_RESULT_INVALID_IMAGE_DATA       -37 // ��Ч��ͼ������
#define BEF_RESULT_INVALID_IMAGE_FORMAT     -38 // ��Ч��ͼƬ��ʽ
#define BEF_RESULT_INVALID_PARAM_TYPE       -39 // ��Ч�Ĳ�������
#define BEF_RESULT_INVALID_RESOURCE_VERSION -40 // ��Դ�ļ�ָ��sdk�汾����
#define BEF_RESULT_INVALID_PARAM_VALUE      -47 // ��Ч�Ĳ���ֵ

#define BEF_RESULT_SMASH_E_INTERNAL         -101 // δ�����ڲ�����
#define BEF_RESULT_SMASH_E_NOT_INITED       -102 // δ��ʼ�������Դ
#define BEF_RESULT_SMASH_E_MALLOC           -103 // �����ڴ�ʧ��
#define BEF_RESULT_SMASH_E_INVALID_PARAM    -104 // ��Ч�Ĳ���
#define BEF_RESULT_SMASH_E_ESPRESSO         -105 // ESPRESSO����
#define BEF_RESULT_SMASH_E_MOBILECV         -106 // MOBILECV����
#define BEF_RESULT_SMASH_E_INVALID_CONFIG   -107 // ��Ч������
#define BEF_RESULT_SMASH_E_INVALID_HANDLE   -108 // ��Ч�ľ��
#define BEF_RESULT_SMASH_E_INVALID_MODEL    -109 // ��Ч��ģ��
#define BEF_RESULT_SMASH_E_INVALID_PIXEL_FORMAT        -110 // ��Ч��ͼ���ʽ
#define BEF_RESULT_SMASH_E_INVALID_POINT               -111 // ��Ч�ĵ�
#define BEF_RESULT_SMASH_E_REQUIRE_FEATURE_NOT_INIT    -112 // ����ģ��û�г�ʼ��
#define BEF_RESULT_SMASH_E_NOT_IMPL                    -113 // δʵ�ֵĽӿ�

#define BEF_RESULT_INVALID_LICENSE                     -114 // ��Ч��license
#define BEF_RESULT_NULL_BUNDLEID                       -115 // Application/Bundle ID Ϊ��
#define BEF_RESULT_LICENSE_STATUS_INVALID              -116 // �Ƿ���Ȩ�ļ�
#define BEF_RESULT_LICENSE_STATUS_EXPIRED              -117 // ��Ȩ�ļ�����
#define BEF_RESULT_LICENSE_STATUS_NO_FUNC              -118 // �����ܲ�ƥ��
#define BEF_RESULT_LICENSE_STATUS_ID_NOT_MATCH         -119 // Application/Bundle ID ��ƥ��

#define BEF_RESULT_LICENSE_BAG_NULL_PATH               -120 // ��Ȩ��·��Ϊ��
#define BEF_RESULT_LICENSE_BAG_INVALID_PATH            -121 // �������Ȩ��·��
#define BEF_RESULT_LICENSE_BAG_TYPE_NOT_MATCH          -122 // ��Ȩ�����Ͳ�ƥ��
#define BEF_RESULT_LICENSE_BAG_INVALID_VERSION         -123 // ��Ч�İ汾
#define BEF_RESULT_LICENSE_BAG_INVALID_BLOCK_COUNT     -124 // ��Ч�����ݿ�
#define BEF_RESULT_LICENSE_BAG_INVALID_BLOCK_LEN       -125 // ��Ч�����ݿ鳤��
#define BEF_RESULT_LICENSE_BAG_INCOMPLETE_BLOCK        -126 // ���ݿ鲻����
#define BEF_RESULT_LICENSE_BAG_UNAUTHORIZED_FUNC       -127 // licenseδ��Ȩ�Ĺ���

#define BEF_RESULT_SDK_FUNC_NOT_INCLUDE                -128 // SDK δ��������
#define BEF_RESULT_GL_ERROR_OCCUR                      -150 // opengl��������
// ע�� EffectSDK �ڲ�ʹ����-114 -115�� ����ʹ��TOB�ĺ���и��� ��ת���� -151 -152��������������Ҫ�ܿ�
//#define BEF_RESULT_GL_CONTECT               -151 //��Ч��glcontext
//#define BEF_RESULT_GL_TEXTURE               -152 //��Ч��gltexture


#define BEF_EFFECT_FEATURE_LEN                          128 //feature name Ĭ�ϳ���

// bef_framework_public_geometry_define
// @brief image rotate type definition
typedef enum {
    BEF_AI_CLOCKWISE_ROTATE_0 = 0, // ͼ����Ҫ��ת��ͼ���е�����Ϊ����
    BEF_AI_CLOCKWISE_ROTATE_90 = 1, // ͼ����Ҫ˳ʱ����ת90�ȣ�ʹͼ���е�����Ϊ��
    BEF_AI_CLOCKWISE_ROTATE_180 = 2, // ͼ����Ҫ˳ʱ����ת180�ȣ�ʹͼ���е�����Ϊ��
    BEF_AI_CLOCKWISE_ROTATE_270 = 3  // ͼ����Ҫ˳ʱ����ת270�ȣ�ʹͼ���е�����Ϊ��
} bef_ai_rotate_type;

// ORDER!!!
typedef enum {
    BEF_AI_PIX_FMT_RGBA8888, // RGBA 8:8:8:8 32bpp ( 4ͨ��32bit RGBA ���� )
    BEF_AI_PIX_FMT_BGRA8888, // BGRA 8:8:8:8 32bpp ( 4ͨ��32bit RGBA ���� )
    BEF_AI_PIX_FMT_BGR888,   // BGR 8:8:8 24bpp ( 3ͨ��32bit RGB ���� )
    BEF_AI_PIX_FMT_RGB888,   // RGB 8:8:8 24bpp ( 3ͨ��32bit RGB ���� )
    BEF_AI_PIX_FMT_GRAY8,    // GRAY 8bpp ( 1ͨ��8bit �Ҷ����� ). Ŀǰ����֧��
    BEF_AI_PIX_FMT_YUV420P,  // YUV  4:2:0   12bpp ( 3ͨ��, һ������ͨ��, ������ΪU������V����ͨ��, ����ͨ������������ ). Ŀǰ����֧��
    BEF_AI_PIX_FMT_NV12,     // YUV  4:2:0   12bpp ( 3ͨ��, һ������ͨ��, ��һ��ΪUV�������� ). Ŀǰ����֧��
    BEF_AI_PIX_FMT_NV21      // YUV  4:2:0   12bpp ( 3ͨ��, һ������ͨ��, ��һ��ΪVU�������� ). Ŀǰ����֧��
} bef_ai_pixel_format;


typedef struct bef_ai_fpoint_t {
    float x;
    float y;
} bef_ai_fpoint;

typedef struct bef_ai_fpoint3d_t {
    float x;
    float y;
    float z;
} bef_ai_fpoint3d;

typedef struct bef_ai_rect_t {
    int left;   // Left most coordinate in rectangle. ��������ߵ�����
    int top;    // Top coordinate in rectangle.  �������ϱߵ�����
    int right;  // Right most coordinate in rectangle.  �������ұߵ�����
    int bottom; // Bottom coordinate in rectangle. �������±ߵ�����
} bef_ai_rect;

// Same definiation as bef_rect, but in float type
// ��bef_rectһ���Ķ��壬����Ϊ�����ȸ���
typedef struct bef_ai_rectf_t {
    float left;
    float top;
    float right;
    float bottom;
} bef_ai_rectf;


typedef enum bef_ai_camera_position_t {
    bef_ai_camera_position_front,
    bef_ai_camera_position_back
} bef_ai_camera_position;



typedef struct bef_ai_frect_st {
    float left;   ///< ��������ߵ�����
    float top;    ///< �������ϱߵ�����
    float right;  ///< �������ұߵ�����
    float bottom; ///< �������±ߵ�����
} bef_ai_frect;


typedef struct bef_ai_image_t {
    const unsigned char *data;
    int width;
    int height;
    int stride;
    int format;
} bef_ai_image;

struct bef_ai_tt_key_point {
    float x; // ��Ӧ cols, ��Χ�� [0, width] ֮��
    float y; // ��Ӧ rows, ��Χ�� [0, height] ֮��
    bool is_detect; // �����ֵΪ false, �� x,y ������
};


#define EFFECT_HAND_DETECT_DELAY_FRAME_COUNT 4


#endif
