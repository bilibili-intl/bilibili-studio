#include "be_render_helper.h"



const char* LINE_VERTEX =
"attribute vec4 a_Position;\n"
"void main() {\n"
"    gl_Position = a_Position;\n"
"}\0";

const char* LINE_FRAGMENT =
"precision mediump float;\n"
"uniform vec4 u_Color;\n"
"void main() {\n"
"    gl_FragColor = u_Color;\n"
"}\0";

const char* POINT_VERTEX =
"attribute vec4 a_Position;\n"
"uniform float uPointSize;\n"
"void main() {\n"
"    gl_Position = a_Position;\n"
"    gl_PointSize = uPointSize;\n"
"}\0";

const char* POINT_FRAGMENT = 
"precision mediump float;\n"
"uniform vec4 u_Color;\n"
"void main() {\n"
"    gl_FragColor = u_Color;\n"
"}\0";

const char* CAMREA_RESIZE_VERTEX =
"attribute vec4 position;\n"
"attribute vec4 inputTextureCoordinate;\n"
"varying vec2 textureCoordinate;\n"
"void main() {\n"
"    textureCoordinate = inputTextureCoordinate.xy;\n"
"    gl_Position = position;\n"
"}\0";

const char* CAMREA_RESIZE_FRAGMENT =
"precision mediump float;\n"
"varying highp vec2 textureCoordinate;\n"
"uniform sampler2D inputImageTexture;\n"
"void main()\n"
"{\n"
"    gl_FragColor = texture2D(inputImageTexture, textureCoordinate);\n"
"}\0";

const char* MASK_VERTEX =
"attribute vec4 position;\n"
"attribute vec4 inputTextureCoordinate;\n"
"varying vec2 textureCoordinate;\n"
"void main() {\n"
"    textureCoordinate = vec2(inputTextureCoordinate.x, 1.0 - inputTextureCoordinate.y);\n"
"    gl_Position = position;\n"
"}\0";

const char* MASK_FRAGMENT =
"precision mediump float;\n"
"varying highp vec2 textureCoordinate;\n"
"uniform sampler2D inputMaskTexture;\n"
"uniform vec4 maskColor;\n"
"void main() {\n"
"    float maska = texture2D(inputMaskTexture, textureCoordinate).a;\n"
"    gl_FragColor = vec4(maskColor.rgb, maska * maskColor.a);\n"
"}\0";

const char* MASK_PORTRAIT_FRAGMENT =
"precision mediump float;\n"
"varying highp vec2 textureCoordinate;\n"
"uniform sampler2D inputMaskTexture;\n"
"uniform vec4 maskColor;\n"
"void main() {\n"
"    float maska = texture2D(inputMaskTexture, textureCoordinate).a;\n"
"    gl_FragColor = vec4(maskColor.rgb, 1.0 - maska);\n"
"}\0";


static float TEXTURE_FLIPPED[] = { 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, };
static float TEXTURE_RORATION_0[] = { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, };
static float TEXTURE_ROTATED_90[] = { 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, };
static float TEXTURE_ROTATED_180[] = { 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, };
static float TEXTURE_ROTATED_270[] = { 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, };
static float CUBE[] = { -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, };


BERenderHelper::BERenderHelper()
{
    initializeOpenGLFunctions();
}


BERenderHelper::~BERenderHelper()
{
}

void BERenderHelper::init() {
    loadPointShader();
    loadLineShader();
    loadResizeShader();
    loadMaskShader();
    loadMaskPortraitShader();

    m_viewWidth = 1280;
    m_viewHeight = 720;
    m_cachedTexture = -1;
    m_ratio = 1.0;
    glGenFramebuffers(1, &m_frameBuffer);
    glGenTextures(1, &m_resizeTexture);
}

void BERenderHelper::destroy() {
    glDeleteFramebuffers(1, &m_frameBuffer);
    if (m_cachedTexture != -1)
    {
        glDeleteTextures(1, &m_cachedTexture);
    }
    glDeleteTextures(1, &m_resizeTexture);
}

void BERenderHelper::setViewWidthAndHeight(int width, int height){
    m_viewWidth = width;
    m_viewHeight = height;
}

void BERenderHelper::setResizeRatio(float ratio){
    m_ratio = ratio;
}

void BERenderHelper::drawPoint(int x, int y, const be_rgba_color &color, float pointSize) {
    float transX = transformX(x);
    float transY = transformY(y);
    GLfloat positions[] = { transX, transY };
    glViewport(0, 0, m_viewWidth, m_viewHeight);
    glUseProgram(m_pointProgram);
    glVertexAttribPointer(m_pointLocation, 2, GL_FLOAT, false, 0, positions);
    glEnableVertexAttribArray(m_pointLocation);

    glUniform4f(m_pointColor, color.red, color.green, color.blue, color.alpha);
    glUniform1f(m_pointSize, pointSize);
    glDrawArrays(GL_POINTS, 0, 1);
    glDisableVertexAttribArray(m_pointLocation);
    checkGLError();

    
}

void BERenderHelper::drawPoints(bef_ai_fpoint *points, int count, const be_rgba_color &color, float pointSize) {
    GLfloat* positions = (GLfloat*)malloc(count * 2 * sizeof(GLfloat));
    if (positions == 0) {
        printf("drawPoints malloc error\n");
        return;
    }
    for (int i = 0; i < count; i++) {
        positions[i * 2] = transformX(points[i].x);
        positions[i * 2 + 1] = transformY(points[i].y);
    }
    glUseProgram(m_pointProgram);

    glViewport(0, 0, m_viewWidth, m_viewHeight);
    glVertexAttribPointer(m_pointLocation, 2, GL_FLOAT, false, 2 * sizeof(GLfloat), positions);
    glEnableVertexAttribArray(m_pointLocation);

    glUniform4f(m_pointColor, color.red, color.green, color.blue, color.alpha);
    glUniform1f(m_pointSize, pointSize);
    glDrawArrays(GL_POINTS, 0, count);
    glDisableVertexAttribArray(m_pointLocation);

    checkGLError();
    free(positions);
}

void BERenderHelper::drawLine(be_render_helper_line *line, const be_rgba_color &color, float lineWidth) {
    float x1 = transformX(line->x1);
    float y1 = transformY(line->y1);
    float x2 = transformX(line->x2);
    float y2 = transformY(line->y2);

    GLfloat positions[] = {
        x1, y1,
        x2, y2
    };

    glViewport(0, 0, m_viewWidth, m_viewHeight);
    glUseProgram(m_lineProgram);
    glVertexAttribPointer(m_lineLocation, 2, GL_FLOAT, false, 2 * sizeof(GLfloat), positions);
    glEnableVertexAttribArray(m_lineLocation);
    glUniform4f(m_lineColor, color.red, color.green, color.blue, color.alpha);

    glLineWidth(lineWidth);
    glDrawArrays(GL_LINES, 0, 2);
    glDisableVertexAttribArray(m_lineLocation);
    checkGLError();
}

void BERenderHelper::drawLines(bef_ai_fpoint *lines, int count, const be_rgba_color &color, float lineWidth) {
    if (count <= 0) return;

    //here we suppose GFfloat == float, not a good way
    GLfloat* positions = (GLfloat*)calloc(count, sizeof(GLfloat) * 2);

    for (int i = 0; i < count; i++) {
        positions[2 * i] =transformX(lines[i].x);
        positions[2 * i + 1] = transformY(lines[i].y);
    }
    glViewport(0, 0, m_viewWidth, m_viewHeight);
    glUseProgram(m_lineProgram);
    glVertexAttribPointer(m_lineLocation, 2, GL_FLOAT, false, 2 * sizeof(GLfloat), positions);
    glEnableVertexAttribArray(m_lineLocation);
    glUniform4f(m_lineColor, color.red, color.green, color.blue, color.alpha);

    glLineWidth(lineWidth);
    glDrawArrays(GL_LINES, 0, count);
    glDisableVertexAttribArray(m_lineLocation);
    free(positions);
}

void BERenderHelper::drawLinesStrip(bef_ai_fpoint *lines, int count, const be_rgba_color &color, float lineWidth) {
    if (count <= 0) return;

    //here we suppose GFfloat == float, not a good way
    GLfloat* positions = (GLfloat*)calloc(count, sizeof(GLfloat) * 2);

    for (int i = 0; i < count; i++) {
        positions[2 * i] = transformX(lines[i].x);
        positions[2 * i + 1] = transformY(lines[i].y);
    }

    glViewport(0, 0, m_viewWidth, m_viewHeight);
    glUseProgram(m_lineProgram);
    glVertexAttribPointer(m_lineLocation, 2, GL_FLOAT, false, 2 * sizeof(GLfloat), positions);
    glEnableVertexAttribArray(m_lineLocation);
    glUniform4f(m_lineColor, color.red, color.green, color.blue, color.alpha);

    glLineWidth(lineWidth);
    glDrawArrays(GL_LINE_STRIP, 0, count);
    glDisableVertexAttribArray(m_lineLocation);
    free(positions);
}


void BERenderHelper::drawRect(bef_ai_rect *rect, const be_rgba_color &color, float lineWidth) {
    float x1 = transformX(rect->left);
    float y1 = transformY(rect->top);
    float x2 = transformX(rect->right);
    float y2 = transformY(rect->bottom);

    GLfloat positions[] = {
        x1, y1,
        x1, y2,
        x2, y2,
        x2, y1,
        x1, y1
    };

    glViewport(0, 0, m_viewWidth, m_viewHeight);

    glUseProgram(m_lineProgram);
    glVertexAttribPointer(m_lineLocation, 2, GL_FLOAT, 0, 2 * sizeof(GLfloat), positions);
    glEnableVertexAttribArray(m_lineLocation);

    glUniform4f(m_lineColor, color.red, color.green, color.blue, color.alpha);

    glLineWidth(lineWidth);
    glDrawArrays(GL_LINE_STRIP, 0, 5);
    glDisableVertexAttribArray(m_lineLocation);
    checkGLError();
}


void BERenderHelper::drawTexture(GLuint texture) {
    glUseProgram(m_resizeProgram);
    glVertexAttribPointer(m_resizeLocation, 2, GL_FLOAT, false, 0, CUBE);
    glEnableVertexAttribArray(m_resizeLocation);
    glVertexAttribPointer(m_resizeTextureCoordinate, 2, GL_FLOAT, false, 0, TEXTURE_RORATION_0);
    glEnableVertexAttribArray(m_resizeTextureCoordinate);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(m_resizeInputImageTexture, 0);
    glViewport(0, 0, m_viewHeight, m_viewHeight);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableVertexAttribArray(m_resizeLocation);
    glDisableVertexAttribArray(m_resizeTextureCoordinate);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

void BERenderHelper::drawMask(unsigned char *mask, const be_rgba_color &color, GLuint currentTexture, GLuint frameBuffer, int *size) {
    glViewport(0, 0, m_viewWidth, m_viewHeight);
    glUseProgram(m_maskProgram);
    glVertexAttribPointer(m_maskPosition, 2, GL_FLOAT, false, 0, CUBE);
    glEnableVertexAttribArray(m_maskPosition);
    glVertexAttribPointer(m_maskCoordinatLocation, 2, GL_FLOAT, false, 0, TEXTURE_FLIPPED);
    glEnableVertexAttribArray(m_maskCoordinatLocation);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if (m_cachedTexture == -1) {
        glGenTextures(1, &m_cachedTexture);
        glBindTexture(GL_TEXTURE_2D, m_cachedTexture);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, size[0], size[1], 0, GL_ALPHA, GL_UNSIGNED_BYTE, mask);
    }
    else {
        glBindTexture(GL_TEXTURE_2D, m_cachedTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, size[0], size[1], 0, GL_ALPHA, GL_UNSIGNED_BYTE, mask);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_cachedTexture);
    glUniform1i(m_maskInputMaskTexture, 0);
    glUniform4f(m_maskColor, color.red, color.green, color.blue, color.alpha);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisable(GL_BLEND);
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    glDisableVertexAttribArray(m_maskCoordinatLocation);
    glDisableVertexAttribArray(m_maskPosition);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
    checkGLError();
}

void BERenderHelper::drawPortraitMask(unsigned char *mask, const be_rgba_color &color, GLuint currentTexture, GLuint frameBuffer, int *size) {
    glViewport(0, 0, m_viewWidth, m_viewHeight);
    glUseProgram(m_maskPortraitProgram);
    glVertexAttribPointer(m_maskPortraitPosition, 2, GL_FLOAT, false, 0, CUBE);
    glEnableVertexAttribArray(m_maskPortraitPosition);
    glVertexAttribPointer(m_maskPortraitCoordinatLocation, 2, GL_FLOAT, false, 0, TEXTURE_FLIPPED);
    glEnableVertexAttribArray(m_maskPortraitCoordinatLocation);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if (m_cachedTexture == -1) {
        glGenTextures(1, &m_cachedTexture);
        glBindTexture(GL_TEXTURE_2D, m_cachedTexture);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, size[0], size[1], 0, GL_ALPHA, GL_UNSIGNED_BYTE, mask);
    }
    else {
        glBindTexture(GL_TEXTURE_2D, m_cachedTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, size[0], size[1], 0, GL_ALPHA, GL_UNSIGNED_BYTE, mask);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_cachedTexture);
    glUniform1i(m_maskPortraitInputMaskTexture, 0);
    glUniform4f(m_maskPortraitColor, color.red, color.green, color.blue, color.alpha);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisable(GL_BLEND);

    glDisableVertexAttribArray(m_maskPortraitCoordinatLocation);
    glDisableVertexAttribArray(m_maskPortraitPosition);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
    checkGLError();
}

void BERenderHelper::textureToImage(GLuint texture, unsigned char * buffer, int width, int height) {
    glBindTexture(GL_TEXTURE_2D, m_resizeTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_resizeTexture, 0);

    glUseProgram(m_resizeProgram);
    glVertexAttribPointer(m_resizeLocation, 2, GL_FLOAT, false, 0, CUBE);
    glEnableVertexAttribArray(m_resizeLocation);
    glVertexAttribPointer(m_resizeTextureCoordinate, 2, GL_FLOAT, false, 0, TEXTURE_RORATION_0);
    glEnableVertexAttribArray(m_resizeTextureCoordinate);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(m_resizeInputImageTexture, 0);
    glViewport(0, 0, width, height);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableVertexAttribArray(m_resizeLocation);
    glDisableVertexAttribArray(m_resizeTextureCoordinate);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    checkGLError();
}

int BERenderHelper::compileShader(const char* shader, GLenum shaderType) {
    GLuint shaderHandle = glCreateShader(shaderType);
    glShaderSource(shaderHandle, 1, &shader, NULL);
    glCompileShader(shaderHandle);
    GLint success;
    glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE)
    {
        printf("BErenderHelper compiler shader error: %s\n", shader);
        return 0;
    }
    return shaderHandle;
}

void BERenderHelper::loadPointShader() {
    GLuint vertexShader = compileShader(POINT_VERTEX, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(POINT_FRAGMENT, GL_FRAGMENT_SHADER);

    m_pointProgram = glCreateProgram();
    glAttachShader(m_pointProgram, vertexShader);
    glAttachShader(m_pointProgram, fragmentShader);
    glLinkProgram(m_pointProgram);
    GLint linkSuccess;
    glGetProgramiv(m_pointProgram, GL_LINK_STATUS, &linkSuccess);
    if (linkSuccess == GL_FALSE) {
        printf("BERenderHelper link shader error\n");
    }

    glUseProgram(m_pointProgram);
    m_pointLocation = glGetAttribLocation(m_pointProgram, "a_Position");
    m_pointColor = glGetUniformLocation(m_pointProgram, "u_Color");
    m_pointSize = glGetUniformLocation(m_pointProgram, "uPointSize");
    if (vertexShader)
    {
        glDeleteShader(vertexShader);
    }
    if (fragmentShader)
    {
        glDeleteShader(fragmentShader);
    }
    checkGLError();
}

void BERenderHelper::loadLineShader() {
    GLuint vertexShader = compileShader(LINE_VERTEX, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(LINE_FRAGMENT,GL_FRAGMENT_SHADER);

    m_lineProgram = glCreateProgram();
    glAttachShader(m_lineProgram, vertexShader);
    glAttachShader(m_lineProgram, fragmentShader);
    glLinkProgram(m_lineProgram);

    GLint linkSuccess;
    glGetProgramiv(m_lineProgram, GL_LINK_STATUS, &linkSuccess);
    if (linkSuccess == GL_FALSE) {
        printf("BERenderHelper link shader error\n");
    }
    glUseProgram(m_lineProgram);
    m_lineLocation = glGetAttribLocation(m_lineProgram, "a_Position");
    m_lineColor = glGetUniformLocation(m_lineProgram, "u_Color");
    if (vertexShader) 
    {
        glDeleteShader(vertexShader);
    }
    if (fragmentShader)
    {
        glDeleteShader(fragmentShader);
    }
    checkGLError();
}


void BERenderHelper::loadResizeShader() {
    GLuint vertexShader = compileShader(CAMREA_RESIZE_VERTEX, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(CAMREA_RESIZE_FRAGMENT, GL_FRAGMENT_SHADER);

    m_resizeProgram = glCreateProgram();
    glAttachShader(m_resizeProgram, vertexShader);
    glAttachShader(m_resizeProgram, fragmentShader);
    glLinkProgram(m_resizeProgram);

    GLint linkSuccess;
    glGetProgramiv(m_resizeProgram, GL_LINK_STATUS, &linkSuccess);
    if (linkSuccess == GL_FALSE) {
        printf("BERenderHelper link shader error\n");
    }

    glUseProgram(m_resizeProgram);
    m_resizeLocation = glGetAttribLocation(m_resizeProgram, "position");
    m_resizeTextureCoordinate = glGetAttribLocation(m_resizeProgram, "inputTextureCoordinate");
    m_resizeInputImageTexture = glGetUniformLocation(m_resizeProgram, "inputImageTexture");

    if (vertexShader) {
        glDeleteShader(vertexShader);
    }
    if (fragmentShader) {
        glDeleteShader(fragmentShader);
    }
    checkGLError();
}

void BERenderHelper::loadMaskShader() {
    GLuint vertexShader = compileShader(MASK_VERTEX , GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader ( MASK_FRAGMENT , GL_FRAGMENT_SHADER);

    m_maskProgram = glCreateProgram();
    glAttachShader(m_maskProgram, vertexShader);
    glAttachShader(m_maskProgram, fragmentShader);
    glLinkProgram(m_maskProgram);

    GLint linkSuccess;
    glGetProgramiv(m_maskProgram, GL_LINK_STATUS, &linkSuccess);
    if (linkSuccess == GL_FALSE) {
       printf("BERenderHelper link shader error\n");
    }

    glUseProgram(m_maskProgram);
    m_maskPosition = glGetAttribLocation(m_maskProgram, "position");
    m_maskCoordinatLocation = glGetAttribLocation(m_maskProgram, "inputTextureCoordinate");

    m_maskInputMaskTexture = glGetUniformLocation(m_maskProgram, "inputMaskTexture");
    m_maskColor = glGetUniformLocation(m_maskProgram, "maskColor");

    if (vertexShader) {
        glDeleteShader(vertexShader);
    }
    if (fragmentShader) {
        glDeleteShader(fragmentShader);
    }
    checkGLError();
}


void BERenderHelper::loadMaskPortraitShader() {
    GLuint vertexShader = compileShader(MASK_VERTEX, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(MASK_PORTRAIT_FRAGMENT, GL_FRAGMENT_SHADER);

    m_maskPortraitProgram = glCreateProgram();
    glAttachShader(m_maskPortraitProgram, vertexShader);
    glAttachShader(m_maskPortraitProgram, fragmentShader);
    glLinkProgram(m_maskPortraitProgram);

    GLint linkSuccess;
    glGetProgramiv(m_maskPortraitProgram, GL_LINK_STATUS, &linkSuccess);
    if (linkSuccess == GL_FALSE) {
        printf("BERenderHelper link shader error\n");
    }

    glUseProgram(m_maskPortraitProgram);
    m_maskPortraitPosition = glGetAttribLocation(m_maskPortraitProgram, "position");
    m_maskPortraitCoordinatLocation = glGetAttribLocation(m_maskPortraitProgram, "inputTextureCoordinate");

    m_maskPortraitInputMaskTexture = glGetUniformLocation(m_maskPortraitProgram, "inputMaskTexture");
    m_maskPortraitColor = glGetUniformLocation(m_maskPortraitProgram, "maskColor");
    
    if (vertexShader) {
        glDeleteShader(vertexShader);
    }

    if (fragmentShader) {
        glDeleteShader(fragmentShader);
    }
    checkGLError();
}

void BERenderHelper::checkGLError() {
    //return;
    int error = glGetError();
    if (error != 0)
    {
        printf("BERenderHelper::checkGLError %d\n", error);
        assert(error == GL_NO_ERROR);
    }
}

float BERenderHelper::transformX(int x) {
    return 2.0f * x / m_ratio / m_viewWidth - 1.0f;
}

float BERenderHelper::transformY(int y) {
    return 2.0f * y / m_ratio / m_viewHeight - 1.0f;
}

