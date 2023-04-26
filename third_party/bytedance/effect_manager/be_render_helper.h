#ifndef BE_RENDER_HELPER_H
#define BE_RENDER_HELPER_H


#include "bef_effect_ai_public_define.h"
#include <QOpenGLFunctions>

typedef struct be_rgba_color 
{
    float red;
    float green;
    float blue;
    float alpha;
}be_rgba_color;

typedef struct be_render_helper_line
{
    float x1;
    float y1;
    float x2;
    float y2;
}be_render_helper_line;


class BERenderHelper :public QOpenGLFunctions
{
public: 
    BERenderHelper();
    ~BERenderHelper();

    void init();
    void destroy();

    void setViewWidthAndHeight(int width, int height);
    void setResizeRatio(float ratio);

    void drawPoint(int x, int y, const be_rgba_color &color, float pointSize);
    void drawPoints(bef_ai_fpoint *points,int count, const be_rgba_color &color, float pointSize);

    void drawLine(be_render_helper_line *line, const be_rgba_color &color, float lineWidth);
    void drawLines(bef_ai_fpoint *lines, int count, const be_rgba_color &color, float lineWidth);
    void drawLinesStrip(bef_ai_fpoint *lines, int count, const be_rgba_color &color, float lineWidth);
    void drawRect(bef_ai_rect *rect, const be_rgba_color &color, float lineWidth);

    void drawTexture(GLuint texture);
    void drawMask(unsigned char *mask, const be_rgba_color &color, GLuint currentTexture, GLuint frameBuffer, int *size);
    void drawPortraitMask(unsigned char *mask, const be_rgba_color &color, GLuint currentTexture, GLuint frameBuffer, int *size);

    void textureToImage(GLuint texture, unsigned char * buffer, int width, int height);
    int compileShader(const char* shader, GLenum shaderType);

private:
    void loadPointShader();
    void loadLineShader();
    void loadResizeShader();
    void loadMaskShader();
    void loadMaskPortraitShader();

    void checkGLError();

    float transformX(int x);
    float transformY(int y);

private:
    GLuint m_lineProgram;
    GLuint m_lineLocation;
    GLuint m_lineColor;
           
    GLuint m_pointProgram;
    GLuint m_pointLocation;
    GLuint m_pointColor;
    GLuint m_pointSize;
           
    GLuint m_resizeProgram;
    GLuint m_resizeLocation;
    GLuint m_resizeInputImageTexture;
    GLuint m_resizeTextureCoordinate;
           
    GLuint m_maskProgram;
    GLuint m_maskColor;
    GLuint m_maskInputMaskTexture;
    GLuint m_maskPosition;
    GLuint m_maskCoordinatLocation;

    //Portrait
    GLuint m_maskPortraitProgram;
    GLuint m_maskPortraitColor;
    GLuint m_maskPortraitInputMaskTexture;
    GLuint m_maskPortraitPosition;
    GLuint m_maskPortraitCoordinatLocation;

    GLuint m_cachedTexture;
    GLuint m_resizeTexture;

    //为了resize buffer
    GLuint m_frameBuffer;

    int m_width;
    int m_height;
    int m_viewWidth;
    int m_viewHeight;

    float m_ratio;
};

#endif // !BE_RENDER_HELPER_H
