#include "be_render.h"
#include "bef_effect_ai_skeleton.h"

const be_rgba_color  BE_COLOR_RED = { 1.0f, 0.0f, 0.0f, 1.0f };
const be_rgba_color  BE_COLOR_GREEN = { 0.0f, 1.0f, 0.0f, 1.0f };
const be_rgba_color  BE_COLOR_BLUE = { 0.0f, 0.0f, 1.0f, 1.0f };
const be_rgba_color  BE_COLOR_HAIR = { 0.5f, 0.08f, 1.0f, 0.3f };
const be_rgba_color  BE_COLOR_PRORTRAIT = { 1.0f, 0.0f, 0.0f, 0.3f };

const float  BE_HAND_BOX_LINE_WIDTH = 2.0f;
const float  BE_HAND_KEYPOINT_LINE_WIDTH = 3.0f;
const float  BE_HAND_KEYPOINT_POINT_SIZE = 8.0f;
const float  BE_SKELETON_LINE_WIDTH = 4.0f;
const float  BE_SKELETION_LINE_POINT_SIZE = 8.0f;
const float  BE_FACE_KEYPOINT_SIZE = 3.0f;
const float  BE_FACE_KEYPOINT_EXTRA_SIZE = 3.0f;
const float  BE_FACE_BOX_WIDTH = 2.0f;


BERender::BERender()
{
    initializeOpenGLFunctions();
}


BERender::~BERender()
{
    
}

void BERender::initRender() {
    m_renderHelper = new BERenderHelper();
    m_renderHelper->init();
    glGenFramebuffers(1, &m_frameBuffer);
} 
void BERender::releaseRender() {
    glDeleteFramebuffers(1, &m_frameBuffer);
}

GLuint BERender::image2Texture(unsigned char *buffer, int width, int height) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

void BERender::texture2Image(GLuint texture, unsigned char* buffer, int width, int height) {
    m_renderHelper->textureToImage(texture, buffer, width, height);
}

void BERender::setRenderHelperWidthHeight(int width, int height) {
    m_renderHelper->setViewWidthAndHeight(width, height);
}

void BERender::setRenderHelperResizeRatio(float ratio) {
    m_renderHelper->setResizeRatio(ratio);
}

void BERender::setRenderTargetTexture(GLuint texture) {
    m_currentTexture = texture;
}


void BERender::drawFace(bef_ai_face_info *faceInfo, bool withExtra) {
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_currentTexture, 0);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("GLError BERender::drawFace \n");
    }

    for (int i = 0; i < faceInfo->face_count; i++) {
        //draw basic 106 rect
        bef_ai_rect* rect = &faceInfo->base_infos[i].rect;
        m_renderHelper->drawRect(rect, BE_COLOR_RED, BE_FACE_BOX_WIDTH);

        //draw basic 106 points
        bef_ai_fpoint* points = faceInfo->base_infos[i].points_array;
        m_renderHelper->drawPoints(points, 106, BE_COLOR_RED, BE_FACE_KEYPOINT_SIZE);

        if (withExtra) { //draw th extra face info
            bef_ai_face_ext_info* extendInfo = faceInfo->extra_infos + i;
            if (extendInfo->eye_count > 0) {
                //left eye
                be_rgba_color color = { 200.0f / 255.0f, 0.0f, 0.0f, 0.0f };
                bef_ai_fpoint* left_eye = extendInfo->eye_left;

                m_renderHelper->drawPoints(left_eye, 22, color, BE_FACE_KEYPOINT_EXTRA_SIZE);

                //right eye
                bef_ai_fpoint* right_eye = extendInfo->eye_right;
                m_renderHelper->drawPoints(right_eye, 22, color, BE_FACE_KEYPOINT_EXTRA_SIZE);
            }
            if (extendInfo->eyebrow_count > 0) {
                //left eyebrow
                bef_ai_fpoint* left_eyebrow = extendInfo->eyebrow_left;
                be_rgba_color color = { 220.0f / 255.0f, 0.0f, 0.0f, 0.0f };

                m_renderHelper->drawPoints(left_eyebrow, 13, color, BE_FACE_KEYPOINT_EXTRA_SIZE);

                //right eyebrow
                bef_ai_fpoint* right_eyebrow = extendInfo->eyebrow_right;
                m_renderHelper->drawPoints(right_eyebrow, 13, color, BE_FACE_KEYPOINT_EXTRA_SIZE);
            }
            if (extendInfo->iris_count > 0) {
                //left iris
                bef_ai_fpoint* left_iris = extendInfo->left_iris;
                be_rgba_color color = { 1.0f, 180.0f / 255.0f, 0.0f, 0.0f };

                m_renderHelper->drawPoints(left_iris, 20, color, BE_FACE_KEYPOINT_EXTRA_SIZE);
                m_renderHelper->drawPoint(left_iris->x, left_iris->y,BE_COLOR_GREEN,BE_FACE_KEYPOINT_SIZE);

                //right iris
                bef_ai_fpoint* right_iris = extendInfo->right_iris;

                m_renderHelper->drawPoints(right_iris, 20, color,  BE_FACE_KEYPOINT_EXTRA_SIZE);
                m_renderHelper->drawPoint(right_iris->x, right_iris->y, BE_COLOR_GREEN, BE_FACE_KEYPOINT_SIZE);
            }
            if (extendInfo->lips_count > 0) {
                //lips
                bef_ai_fpoint* lips = extendInfo->lips;
                be_rgba_color color = { 200.0f / 255.0f, 40.0f / 255.0f, 40.0f / 255.0f, 0.0f };

                m_renderHelper->drawPoints(lips, 60, color, BE_FACE_KEYPOINT_EXTRA_SIZE);
            }
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    checkGLError();
}

void BERender::drawFaceRect(bef_ai_face_info * faceInfo) {

    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_currentTexture, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        printf("error BERender::drawFaceRect glCheckFramebufferStatus when draw face\n");
    }

    for (int i = 0; i < faceInfo->face_count; i++) {
        //draw basic 106 rect
        bef_ai_rect* rect = &faceInfo->base_infos[i].rect;
        m_renderHelper->drawRect(rect, BE_COLOR_RED ,BE_FACE_BOX_WIDTH);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    checkGLError();
}

void BERender::drawSkeleton(bef_skeleton_info *skeletonInfo, int validCount) {
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_currentTexture, 0);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        printf("BERender::drawSkeleton glCheckFramebufferStatus failed!\n");
    }
    for (int countIndex = 0; countIndex < validCount; countIndex++)
    {
        bef_skeleton_info *info = skeletonInfo + countIndex;
        bef_ai_rect rect= info->skeletonRect;
        m_renderHelper->drawRect(&rect, BE_COLOR_BLUE, BE_HAND_KEYPOINT_LINE_WIDTH);
        for (int pointIndex =0 ;pointIndex<BEF_MAX_SKELETON_POINT_NUM;pointIndex++)
        {
             bef_skeleton_point_info *point = info->keyPointInfos + pointIndex;
             if (point->is_detect)
             {
                 m_renderHelper->drawPoint(point->x, point->y, BE_COLOR_BLUE, BE_SKELETION_LINE_POINT_SIZE);
             }
        }
        int pairs[36] = { 4, 3, 3, 2, 2, 1, 1, 5, 5, 6, 6, 7,
            16, 14, 14, 0, 17, 15, 15, 0,
            1, 8, 8, 11, 11, 1, 1, 0,
            8, 9, 9, 10, 11, 12, 12, 13 };
        for (int lineIndex = 0; lineIndex <= 34; lineIndex += 2)
        {
            bef_skeleton_point_info *left = info->keyPointInfos + pairs[lineIndex];
            bef_skeleton_point_info *right = info->keyPointInfos + pairs[lineIndex + 1];
            be_render_helper_line line = { 0.0f, 0.0f, 0.0f, 0.0f };
            if (left->is_detect && right->is_detect)
            {
                line.x1 = left->x;
                line.y1 = left->y;
                line.x2 = right->x;
                line.y2 = right->y;
                m_renderHelper->drawLine(&line, BE_COLOR_BLUE, BE_SKELETON_LINE_WIDTH);
            }
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    checkGLError();
}

void BERender::drawHands(bef_ai_hand_info *handsInfo) {
    int handsCount = handsInfo->hand_count;
    if (handsCount <= 0)
    {
        return;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_currentTexture, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        printf("glCheckFramebufferStatus when draw hands\n");
    }

    for (int i = 0; i < handsCount; i++) {
        bef_ai_hand* hand = &handsInfo->p_hands[i];

        //draw hand rect
        bef_ai_rect* handRect = &hand->rect;
        m_renderHelper->drawRect(handRect, BE_COLOR_RED, BE_HAND_BOX_LINE_WIDTH);

        //draw hand basic key points
        struct bef_ai_tt_key_point *baiscHandKeyPoints = hand->key_points;
        for (int j = 0; j < BEF_HAND_KEY_POINT_NUM; j++) {
            struct bef_ai_tt_key_point *point = baiscHandKeyPoints + j;

            if (point->is_detect) {
                m_renderHelper->drawPoint(point->x, point->y, BE_COLOR_RED, BE_HAND_KEYPOINT_POINT_SIZE);
            }
        }

        //draw hand extend key points
        struct bef_ai_tt_key_point *extendHandKeyPoins = hand->key_points_extension;
        for (int j = 0; j < BEF_HAND_KEY_POINT_NUM_EXTENSION; j++) {
            struct bef_ai_tt_key_point *point = extendHandKeyPoins + j;

            if (point->is_detect) {
                m_renderHelper->drawPoint(point->x, point->y, BE_COLOR_RED, BE_HAND_KEYPOINT_POINT_SIZE);
            }
        }

        bef_ai_fpoint points[5];
        points[0].x = hand->key_points[0].x;
        points[0].y = hand->key_points[0].y;

        //draw hand line
        for (int n = 0; n < 5; n++) {
            int index = 4 * n + 1;
            for (int k = 1; k < 5; k++) {
                points[k].x = hand->key_points[index].x;
                points[k].y = hand->key_points[index++].y;
            }
            m_renderHelper->drawLinesStrip(points, 5, BE_COLOR_RED, BE_HAND_KEYPOINT_LINE_WIDTH);
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    checkGLError();
    
}

void BERender::drawHairParse(unsigned char *mask, int *maskSize) {
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_currentTexture, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        printf("glCheckFramebufferStatus when draw hair\n");
    }
    m_renderHelper->drawMask(mask, BE_COLOR_HAIR, m_currentTexture, m_frameBuffer, maskSize);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    checkGLError();
}

void BERender::drawPortrait(unsigned char *mask, int *maskSize) {
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_currentTexture, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        printf("glCheckFramebufferStatus when draw portrait\n");
    }
    m_renderHelper->drawPortraitMask(mask, BE_COLOR_PRORTRAIT, m_currentTexture, m_frameBuffer, maskSize);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    checkGLError();
}



void BERender::checkGLError(){
    int error = glGetError();
    if (error != GL_NO_ERROR)
    {
        printf("GlError! %d\n", error);
    }
    assert(error == GL_NO_ERROR); 
}