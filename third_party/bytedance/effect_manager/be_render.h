#ifndef BE_RENDER_H
#define BE_RENDER_H


#include "be_render_helper.h"
#include "bef_effect_ai_face_detect.h"
#include "bef_effect_ai_face_attribute.h"
#include "bef_effect_ai_skeleton.h"
#include "bef_effect_ai_hand.h"

#include <QOpenGLFunctions>

class BERender:public QOpenGLFunctions
{
public:
    BERender();
    ~BERender();
    void initRender();
    void releaseRender();

    GLuint image2Texture(unsigned char *buffer, int width, int height);
    void texture2Image(GLuint texture, unsigned char* buffer, int width, int height);

    void setRenderHelperWidthHeight(int width, int height);
    void setRenderHelperResizeRatio(float ratio);
    void setRenderTargetTexture(GLuint texture);

    void drawFace(bef_ai_face_info *faceInfo, bool withExtra);
    void drawFaceRect(bef_ai_face_info * faceInfo);
    void drawSkeleton(bef_skeleton_info *skeletonInfo, int validCount);
    void drawHands(bef_ai_hand_info *handInfo);
    void drawHairParse(unsigned char *mask, int *maskSize);
    void drawPortrait(unsigned char *mask, int *maskSize);
private:
    void checkGLError();
private:
    BERenderHelper *m_renderHelper;
    GLuint m_frameBuffer;
    GLuint m_textureInput;
    GLuint m_textureOutput;
    GLuint m_currentTexture;
    unsigned char *m_pixelBuffer = nullptr;
};

#endif // !BE_RENDER_H
