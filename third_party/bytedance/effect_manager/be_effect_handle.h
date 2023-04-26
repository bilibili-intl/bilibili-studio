#ifndef BE_EFFECT_HANDLE_H
#define BE_EFFECT_HANDLE_H



#include <windows.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <mutex>
#include <map>

#include "bef_effect_ai_api.h"

#include "be_composer_node.h"


const int OFFSET = 16;
const int SUB_OFFSET = 8;

// 一级菜单
//The second menu

const int TYPE_CLOSE = -1;
// Beautify face 美颜
const int TYPE_BEAUTY_FACE        = 1 << OFFSET;
// Beautify face 美颜四件套
const int TYPE_BEAUTY_FACE_4ITEMS = (1 << OFFSET) +  (1<< SUB_OFFSET);


// Beautify reshape 美型
const int TYPE_BEAUTY_RESHAPE     = 2 << OFFSET;
// Beautify body 美体
const int TYPE_BEAUTY_BODY        = 3 << OFFSET;
// Makeup 美妆
const int TYPE_MAKEUP             = 4 << OFFSET;
// Filiter 滤镜
const int TYPE_FILTER             = 5 << OFFSET;
// Makeup option 美妆类型
const int TYPE_MAKEUP_OPTION      = 6 << OFFSET;

// 二级菜单
//The secondary menu

// Beautify face 美颜
const int TYPE_BEAUTY_FACE_SMOOTH                = TYPE_BEAUTY_FACE + 1;    // 磨皮
const int TYPE_BEAUTY_FACE_WHITEN                = TYPE_BEAUTY_FACE + 2;    // 美白
const int TYPE_BEAUTY_FACE_SHARPEN               = TYPE_BEAUTY_FACE + 3;    // 锐化
const int TYPE_BEAUTY_FACE_4ITEMS_BRIGHTEN_EYE   = TYPE_BEAUTY_FACE_4ITEMS + 1;    // 亮眼
const int TYPE_BEAUTY_FACE_4ITEMS_REMOVE_POUCH   = TYPE_BEAUTY_FACE_4ITEMS + 2;    // 黑眼圈
const int TYPE_BEAUTY_FACE_4ITEMS_SMILE_FOLDS    = TYPE_BEAUTY_FACE_4ITEMS + 3;    // 法令纹
const int TYPE_BEAUTY_FACE_4ITEMS_WHITEN_TEETH   = TYPE_BEAUTY_FACE_4ITEMS + 4;    // 白牙

// Beautify reshape 美形
const int TYPE_BEAUTY_RESHAPE_FACE_OVERALL = TYPE_BEAUTY_RESHAPE + 1;     // 瘦脸
const int TYPE_BEAUTY_RESHAPE_FACE_CUT     = TYPE_BEAUTY_RESHAPE + 2;     // 窄脸
const int TYPE_BEAUTY_RESHAPE_FACE_SMALL   = TYPE_BEAUTY_RESHAPE + 3;     // 小脸
const int TYPE_BEAUTY_RESHAPE_EYE          = TYPE_BEAUTY_RESHAPE + 4;     // 大眼
const int TYPE_BEAUTY_RESHAPE_EYE_ROTATE   = TYPE_BEAUTY_RESHAPE + 5;     // 眼角度
const int TYPE_BEAUTY_RESHAPE_CHEEK        = TYPE_BEAUTY_RESHAPE + 6;     // 瘦颧骨
const int TYPE_BEAUTY_RESHAPE_JAW          = TYPE_BEAUTY_RESHAPE + 7;     // 下颌骨 
const int TYPE_BEAUTY_RESHAPE_NOSE_LEAN    = TYPE_BEAUTY_RESHAPE + 8;     // 瘦鼻
const int TYPE_BEAUTY_RESHAPE_NOSE_LONG    = TYPE_BEAUTY_RESHAPE + 9;     // 长鼻
const int TYPE_BEAUTY_RESHAPE_CHIN         = TYPE_BEAUTY_RESHAPE + 10;    // 下巴
const int TYPE_BEAUTY_RESHAPE_FOREHEAD     = TYPE_BEAUTY_RESHAPE + 11;    // 额头
    
const int TYPE_BEAUTY_RESHAPE_MOUTH_ZOOM   = TYPE_BEAUTY_RESHAPE + 12;    // 嘴型
const int TYPE_BEAUTY_RESHAPE_MOUTH_SMILE  = TYPE_BEAUTY_RESHAPE + 13;    // 微笑
//const int TYPE_BEAUTY_RESHAPE_DECREE       = TYPE_BEAUTY_RESHAPE + 14;    // 
//const int TYPE_BEAUTY_RESHAPE_DARK         = TYPE_BEAUTY_RESHAPE + 15;    // 
const int TYPE_BEAUTY_RESHAPE_EYE_SPACING  = TYPE_BEAUTY_RESHAPE + 14;    // 眼距离
const int TYPE_BEAUTY_RESHAPE_EYE_MOVE     = TYPE_BEAUTY_RESHAPE + 15;    // 眼移动
const int TYPE_BEAUTY_RESHAPE_MOUTH_MOVE   = TYPE_BEAUTY_RESHAPE + 16;    // 缩人中

// Beautify body 美体
const int TYPE_BEAUTY_BODY_THIN     = TYPE_BEAUTY_BODY + 1;     // 瘦身
const int TYPE_BEAUTY_BODY_LONG_LEG = TYPE_BEAUTY_BODY + 2;     // 长腿


// Makeup 美妆
const int TYPE_MAKEUP_LIP        = TYPE_MAKEUP_OPTION + (1 << SUB_OFFSET);    // 腮红
const int TYPE_MAKEUP_BLUSH      = TYPE_MAKEUP_OPTION + (2 << SUB_OFFSET);    // 口红
const int TYPE_MAKEUP_PUPIL      = TYPE_MAKEUP_OPTION + (3 << SUB_OFFSET);    // 美瞳
const int TYPE_MAKEUP_HAIRDYE    = TYPE_MAKEUP_OPTION + (4 << SUB_OFFSET);    // 染发
const int TYPE_MAKEUP_EYESHADOW  = TYPE_MAKEUP_OPTION + (5 << SUB_OFFSET);    // 眼影
const int TYPE_MAKEUP_BROW       = TYPE_MAKEUP_OPTION + (6 << SUB_OFFSET);    // 眉毛
const int TYPE_MAKEUP_TRIM       = TYPE_MAKEUP_OPTION + (7 << SUB_OFFSET);    // 修容

//MakeUp-Lip
const int TYPE_MAKEUP_LIP_FUGUHONG      = TYPE_MAKEUP_LIP + 1;    // 复古红
const int TYPE_MAKEUP_LIP_SHAONVFEN     = TYPE_MAKEUP_LIP + 2;    // 少女粉
const int TYPE_MAKEUP_LIP_YUANQIJU      = TYPE_MAKEUP_LIP + 3;    // 元气橘
const int TYPE_MAKEUP_LIP_XIYOUSE       = TYPE_MAKEUP_LIP + 4;    // 西柚色
const int TYPE_MAKEUP_LIP_XIGUAHONG     = TYPE_MAKEUP_LIP + 5;    // 西瓜红
const int TYPE_MAKEUP_LIP_SIRONGHONG    = TYPE_MAKEUP_LIP + 6;    // 丝绒色
const int TYPE_MAKEUP_LIP_ZANGJUSE      = TYPE_MAKEUP_LIP + 7;    // 脏橘色
const int TYPE_MAKEUP_LIP_MEIZISE       = TYPE_MAKEUP_LIP + 8;    // 梅子红
const int TYPE_MAKEUP_LIP_SHANHUSE      = TYPE_MAKEUP_LIP + 9;    // 珊瑚色
const int TYPE_MAKEUP_LIP_DOUSHAFEN     = TYPE_MAKEUP_LIP + 10;   // 豆沙粉

//MakeUp-Blush 腮红
const int TYPE_MAKEUP_BLUSH_WEIXUN       = TYPE_MAKEUP_BLUSH + 1;    // 微醺
const int TYPE_MAKEUP_BLUSH_RICHANG      = TYPE_MAKEUP_BLUSH + 2;    // 日常
const int TYPE_MAKEUP_BLUSH_MITAO        = TYPE_MAKEUP_BLUSH + 3;    // 蜜桃
const int TYPE_MAKEUP_BLUSH_TIANCHENG    = TYPE_MAKEUP_BLUSH + 4;    // 甜橙
const int TYPE_MAKEUP_BLUSH_QIAOPI       = TYPE_MAKEUP_BLUSH + 5;    // 俏皮
const int TYPE_MAKEUP_BLUSH_XINJI        = TYPE_MAKEUP_BLUSH + 6;    // 心机
const int TYPE_MAKEUP_BLUSH_SHAISHANG    = TYPE_MAKEUP_BLUSH + 7;    // 晒伤
          
//MakeUp-PUPIL 美瞳
const int TYPE_MAKEUP_PUPIL_HUNXUEZONG     = TYPE_MAKEUP_PUPIL + 1;    // 混血棕
const int TYPE_MAKEUP_PUPIL_KEKEZONG       = TYPE_MAKEUP_PUPIL + 2;    // 可可棕
const int TYPE_MAKEUP_PUPIL_MITAOFEN       = TYPE_MAKEUP_PUPIL + 3;    // 蜜桃粉
const int TYPE_MAKEUP_PUPIL_SHUIGUANGHEI   = TYPE_MAKEUP_PUPIL + 4;    // 水光黑
const int TYPE_MAKEUP_PUPIL_XINGKONGLAN    = TYPE_MAKEUP_PUPIL + 5;    // 星空蓝
const int TYPE_MAKEUP_PUPIL_CHUJIANHUI     = TYPE_MAKEUP_PUPIL + 6;    // 初见灰


//MakeUp-HAIRDYE 染发
const int TYPE_MAKEUP_HAIRDYE_ANLAN        = TYPE_MAKEUP_HAIRDYE + 1;    // 暗蓝
const int TYPE_MAKEUP_HAIRDYE_MOLV         = TYPE_MAKEUP_HAIRDYE + 2;    // 墨绿
const int TYPE_MAKEUP_HAIRDYE_SHENZONG     = TYPE_MAKEUP_HAIRDYE + 3;    // 深棕

//MakeUp-EYESHADOW 眼影
const int TYPE_MAKEUP_EYESHADOW_DADIZONG       = TYPE_MAKEUP_EYESHADOW + 1;    // 大地棕
const int TYPE_MAKEUP_EYESHADOW_WANXIAHONG     = TYPE_MAKEUP_EYESHADOW + 2;    // 晚霞红
const int TYPE_MAKEUP_EYESHADOW_SHAONVFEN      = TYPE_MAKEUP_EYESHADOW + 3;    // 少女粉
const int TYPE_MAKEUP_EYESHADOW_QIZHIFEN       = TYPE_MAKEUP_EYESHADOW + 4;    // 气质粉
const int TYPE_MAKEUP_EYESHADOW_MEIZIHONG      = TYPE_MAKEUP_EYESHADOW + 5;    // 梅子红
const int TYPE_MAKEUP_EYESHADOW_JIAOTANGZONG   = TYPE_MAKEUP_EYESHADOW + 6;    // 焦糖棕
const int TYPE_MAKEUP_EYESHADOW_YUANQIJU       = TYPE_MAKEUP_EYESHADOW + 7;    // 元气橘
const int TYPE_MAKEUP_EYESHADOW_NAICHASE       = TYPE_MAKEUP_EYESHADOW + 8;    // 奶茶色


//MakeUp-BROW 眉毛
const int TYPE_MAKEUP_BROW_BR01 = TYPE_MAKEUP_BROW + 1;    // BR01
const int TYPE_MAKEUP_BROW_BK01 = TYPE_MAKEUP_BROW + 2;    // BK01
const int TYPE_MAKEUP_BROW_BK02 = TYPE_MAKEUP_BROW + 3;    // BK02
const int TYPE_MAKEUP_BROW_BK03 = TYPE_MAKEUP_BROW + 4;    // BK03

//MakeUp-TRIM 修容
const int TYPE_MAKEUP_TRIM_TRIM01 = TYPE_MAKEUP_TRIM + 1;    // 修容01
const int TYPE_MAKEUP_TRIM_TRIM02 = TYPE_MAKEUP_TRIM + 2;    // 修容02
const int TYPE_MAKEUP_TRIM_TRIM03 = TYPE_MAKEUP_TRIM + 3;    // 修容03
const int TYPE_MAKEUP_TRIM_TRIM04 = TYPE_MAKEUP_TRIM + 4;    // 修容04


// Node name 结点名称
const std::string NODE_BEAUTY = "beauty_Android";
const std::string NODE_BEAUTY_4ITEMS = "beauty_4Items";
const std::string NODE_RESHAPE = "reshape";
const std::string NODE_LONG_LEG = "body/longleg";
const std::string NODE_THIN = "body/thin";




typedef enum 
{
    EFFECT_TYPE_BEAUTY = 0,
    EFFECT_TYPE_RESHAPE = 1,
    EFFECT_TYPE_MAKEUP = 2,
}BE_EFFECT_TYPE;

class EffectHandle {
public:
    EffectHandle();
    ~EffectHandle();
    bef_effect_result_t initializeHandle() ;
    bef_effect_result_t releaseHandle() ;
    bef_effect_result_t process(GLint texture, GLint textureSticker, int width, int height, int timeStamp);
    void setInitWidthAndHeight(int width, int height);
    bef_effect_result_t setEffectWidthAndHeight(int width, int height);
    
   
public:
    typedef std::map<int, BEComposerNode*> ComposerNodeMap;
    void updateComposerNode(int subId, int updateStatus, float value);
private:
    void setComposerNodes();
    void updateComposerNodeValue(BEComposerNode *node);
    //updateStatus 0: off , 1: on, 2: update value
    bool addComposerPath(std::map<int, std::string> &pathMap, BEComposerNode *node);
    bool removeComposerPath(std::map<int, std::string> &pathMap, BEComposerNode *node);
    
private:
    void registerBeautyComposerNodes();
    void registerBeautyFaceNodes();
    void registerBeautyReshaperNodes();
    void registerBeautyMakeupNodes();
    void registerComposerNode(int majorId,int subId, bool isMajor, std::string NodeName, std::string key);
public:
    void setEffectOn(BE_EFFECT_TYPE effectType, bool checked);

    void setIntensity(int key, float val);
    void setReshapeIntensity(float val1, float val2);
    void setSticker(const std::string &stickerPath);
    void setFilter(const std::string &filterPath);
private:
    void setReshapeOn(bool checked);
    void setBeautyOn(bool checked);
    void setMakeupOn(bool checked);

private:
    bef_effect_handle_t m_renderMangerHandle = 0;
    int m_init_width = 0;
    int m__init_height = 0;
    std::mutex m_effectMutex;
    bool m_initialized = false;

    ComposerNodeMap m_allComposerNodeMap;
    
    std::map<int, std::string> m_currentMajorPaths;
    std::map<int, std::string> m_currentSubPaths;

    ComposerNodeMap m_currentComposerNodes;     
};

#endif // BE_EFFECT_HANDLE_H
