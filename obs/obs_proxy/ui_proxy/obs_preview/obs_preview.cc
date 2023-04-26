#include "obs/obs_proxy/ui_proxy/obs_preview/obs_preview.h"

#include "base/strings/utf_string_conversions.h"
#include "base/win/win_util.h"

#include "obs/obs_proxy/core_proxy/core_proxy_impl.h"
#include "obs/obs_proxy/ui_proxy/obs_preview/obs_preview_controller.h"
#include "obs/obs_proxy/ui_proxy/ui_proxy_impl.h"
#include "obs/obs_proxy/public/proxy/obs_sources_wrapper.h"
#include "obs/obs_proxy/public/common/pref_constants.h"

#include "ui/views/controls/native_control.h"

#include <Windowsx.h>

namespace
{
    const int kDefaultRenderWidth = 1280;
    const int kDefaultRenderHeight = 720;
    static float kDefaultRenderScale = 16 * 1.0f / 9;

    void GetScaleAndCenterPos(
        int base_cx, int base_cy, int window_cx, int window_cy,
        int &x, int &y, int& cx, int& cy, float &scale)
    {

        double windowAspect, baseAspect;
        int new_cx, new_cy;

        windowAspect = double(window_cx) / double(window_cy);
        baseAspect = double(base_cx) / double(base_cy);

        if (windowAspect > baseAspect)
        {
            scale = float(window_cy) / float(base_cy);
            new_cx = int(double(window_cy) * baseAspect);
            new_cy = window_cy;
        }
        else
        {
            scale = float(window_cx) / float(base_cx);
            new_cx = window_cx;
            new_cy = int(float(window_cx) / baseAspect);
        }

        x = window_cx / 2 - new_cx / 2;
        y = window_cy / 2 - new_cy / 2;
        cx = new_cx;
        cy = new_cy;
    }

    int TranslateQtKeyboardEventModifiers(ui::Event *event, bool mouseEvent)
    {
        int obsModifiers = INTERACT_NONE;

        if (event->IsControlDown())
            obsModifiers |= INTERACT_CONTROL_KEY;
        if (event->IsShiftDown())
            obsModifiers |= INTERACT_SHIFT_KEY;
        if (event->IsAltDown())
            obsModifiers |= INTERACT_ALT_KEY;

        if (!mouseEvent)
        {
            /*if (event->modifiers().testFlag(Qt::KeypadModifier))
                obsModifiers |= INTERACT_IS_KEY_PAD;*/
        }

        return obsModifiers;
    }

    int TranslateQtMouseEventModifiers(ui::MouseEvent *event)
    {
        int modifiers = TranslateQtKeyboardEventModifiers(event, true);

        if (event->IsLeftMouseButton())
            modifiers |= INTERACT_MOUSE_LEFT;
        if (event->IsMiddleMouseButton())
            modifiers |= INTERACT_MOUSE_MIDDLE;
        if (event->IsRightMouseButton())
            modifiers |= INTERACT_MOUSE_RIGHT;

        return modifiers;
    }

}

namespace obs_proxy_ui {

#define PREVIEW_EDGE_SIZE 0

static void DrawCircleAtPos(float x, float y, matrix4 &matrix,
        float previewScale) {

    struct vec3 pos;
    vec3_set(&pos, x, y, 0.0f);
    vec3_transform(&pos, &pos, &matrix);
    vec3_mulf(&pos, &pos, previewScale);

    gs_matrix_push();
    gs_matrix_translate(&pos);
    gs_matrix_scale3f(HANDLE_RADIUS, HANDLE_RADIUS, 1.0f);
    gs_draw(GS_LINESTRIP, 0, 0);
    gs_matrix_pop();
}

// -*- OBSPreview -*-

OBSPreview::OBSPreview(OBSPreviewController *controller, PreviewType type)
    : OBSPreviewMouseHandle(type),
      controller_(controller)
    , should_render_(false)
    , should_clear_only_(false)
{
}

OBSPreview::~OBSPreview() {

    //todo: sure: free ...

    obs_enter_graphics();
    gs_vertexbuffer_destroy(box_);
    gs_vertexbuffer_destroy(box_left_);
    gs_vertexbuffer_destroy(box_top_);
    gs_vertexbuffer_destroy(box_right_);
    gs_vertexbuffer_destroy(box_bottom_);
    gs_vertexbuffer_destroy(circle_);
    obs_leave_graphics();

    if (display_) {
        obs_display_remove_draw_callback(
            display_->obj, OBSPreview::Render, this);
    }
}

//static
void OBSPreview::Render(void *data, uint32_t cx, uint32_t cy) {

    OBSPreview *obs_preview = reinterpret_cast<OBSPreview *>(data);

    if (!obs_preview->should_render_.load(std::memory_order_relaxed)) {
        return;
    }

    gfx::Size render_size = obs_preview->RecalculateDisplayViewport();

    if (obs_preview->should_clear_only_.load(std::memory_order_relaxed)) {
        vec4 background_color;
        vec4_set(&background_color, 0.f, 0.f, 0.f, 1.0f);
        gs_clear(GS_CLEAR_COLOR | GS_CLEAR_DEPTH, &background_color, 0.0f, 0.0f);
        return;
    }

    float blackground_color_arr[3]{};
    obs_preview->GetBackgroundColor(&blackground_color_arr[0], &blackground_color_arr[1], &blackground_color_arr[2]);

    vec4 background_color;
    vec4_set(&background_color, blackground_color_arr[0], blackground_color_arr[1], blackground_color_arr[2], 1.0f);
    gs_clear(GS_CLEAR_COLOR | GS_CLEAR_DEPTH, &background_color, 0.0f, 0.0f);

    gs_viewport_push();
    gs_projection_push();

    /* --------------------------------------- */

    gs_ortho(0.0f, float(render_size.width()), 0.0f, float(render_size.height()),
        -100.0f, 100.0f);
    gs_set_viewport(obs_preview->preview_x_, obs_preview->preview_y_,
            obs_preview->preview_cx_, obs_preview->preview_cy_);

    obs_preview->DrawBackdrop(float(render_size.width()), float(render_size.height()));

    //ignore PreviewProgramMode
    switch (obs_preview->preview_type_)
    {
    case PreviewType::PREVIEW_MAIN:
    case PreviewType::PREVIEW_PROJECTOR:
    case PreviewType::PREVIEW_MAIN_PREVIEW:
        obs_render_main_texture();
        break;
    case PreviewType::PREVIEW_SOURCE:
    {
        obs_source_t *source = obs_preview->controller_->GetRenderSource();
        if (source)
        {
            auto scene_item = obs_preview->controller_->GetRenderScenceItem();
            if (scene_item)
            {
                gs_matrix_push();
                matrix4 m{};
                obs_sceneitem_get_draw_transform(scene_item, &m);
                obs_transform_info info{};
                obs_sceneitem_get_info(scene_item, &info);

                float width = obs_source_get_width(source);
                float height = obs_source_get_height(source);

                float scale_x = info.scale.x;
                float scale_y = info.scale.y;

                float x{}, y{};
                bool reverse{};
                bool reverse_x{};
                bool reverse_y{};
                if (info.rot < 0.0f && info.rot >= -90.f)
                {
                    y += width;
                    std::swap(scale_x, scale_y);
                    reverse_x = true;
                }
                else if (info.rot < -90.f && info.rot >= -180.f)
                {
                    x += width;
                    y += height;
                    reverse = true;
                }
                else if (info.rot < -180.f && info.rot >= -270.f)
                {
                    x += height;
                    std::swap(scale_x, scale_y);
                    reverse_y = true;
                }

                if (info.scale.x <= -0.0f)
                {
                    if (reverse_x)
                    {
                        y = y ? 0 : width;
                    }
                    else if(reverse_y)
                    {
                        y = y ? 0 : width;
                    }
                    else if (reverse)
                    {
                        x = x ? 0 : width;
                    }
                    else
                    {
                        x = x ? 0 : width;
                    }
                }

                if (info.scale.y <= -0.0f)
                {
                    if (reverse_x)
                    {
                        x = x ? 0 : height;
                    }
                    else if (reverse_y)
                    {
                        x = x ? 0 : height;
                    }
                    else if (reverse)
                    {
                        y = y ? 0 : height;
                    }
                    else
                    {
                        y = y ? 0 : height;
                    }
                }

                matrix4 sm{};
                vec3 rs = { std::abs(1.0f / scale_x), std::abs(1.0f / scale_y), 1.0f };
                matrix4_scale(&sm, &m, &rs);

                matrix4 nm{};
                vec3 pos = { -(info.pos.x / std::abs(scale_x) - x), -(info.pos.y / std::abs(scale_y) - y), 0.0f };
                matrix4_translate3v(&nm, &sm, &pos);

                gs_matrix_set(&nm);
            }

            obs_source_video_render(source);

            if (scene_item)
            {
                gs_matrix_pop();
            }
        }
    }
        break;
    default:
        NOTREACHED() << "Unknown preview type";
        obs_render_main_texture();
        break;
    }

    gs_load_vertexbuffer(nullptr);

    /* --------------------------------------- */

    gfx::Rect rect = obs_preview->obs_preview_controller()->GetViewRect();
    float right  = float(rect.width())  - obs_preview->preview_x_;
    float bottom = float(rect.height()) - obs_preview->preview_y_;

    gs_ortho(-obs_preview->preview_x_, right,
             -obs_preview->preview_y_, bottom,
             -100.0f, 100.0f);
    gs_reset_viewport();

    switch (obs_preview->preview_type_) {
    case PreviewType::PREVIEW_MAIN:
        obs_preview->DrawSceneEditing();
        break;

    default:
        break;
    }

    /* --------------------------------------- */

    gs_projection_pop();
    gs_viewport_pop();

    UNUSED_PARAMETER(cx);
    UNUSED_PARAMETER(cy);
}

//static
bool OBSPreview::DrawSelectedItem(obs_scene_t *scene,
        obs_sceneitem_t *item, void *param)
{
    // 没选中就不画操作边缘了
    if (!obs_sceneitem_selected(item))
    {
        return true;
    }

    OBSPreview *obs_preview = static_cast<OBSPreview *>(param);

    matrix4 boxTransform;
    matrix4 invBoxTransform;
    obs_sceneitem_get_box_transform(item, &boxTransform);
    matrix4_inv(&invBoxTransform, &boxTransform);

    vec3 bounds[] = {
        {{{0.f, 0.f, 0.f}}},
        {{{1.f, 0.f, 0.f}}},
        {{{0.f, 1.f, 0.f}}},
        {{{1.f, 1.f, 0.f}}},
    };

    bool visible = std::all_of(std::begin(bounds), std::end(bounds),
            [&](const vec3 &b) {
        vec3 pos;
        vec3_transform(&pos, &b, &boxTransform);
        vec3_transform(&pos, &pos, &invBoxTransform);
        return CloseFloat(pos.x, b.x) && CloseFloat(pos.y, b.y);
    });

    if (!visible)
        return true;

    obs_transform_info info;
    obs_sceneitem_get_info(item, &info);

    gs_load_vertexbuffer(obs_preview->circle_);

    DrawCircleAtPos(0.0f, 0.0f, boxTransform, obs_preview->preview_scale_);
    DrawCircleAtPos(0.0f, 1.0f, boxTransform, obs_preview->preview_scale_);
    DrawCircleAtPos(1.0f, 0.0f, boxTransform, obs_preview->preview_scale_);
    DrawCircleAtPos(1.0f, 1.0f, boxTransform, obs_preview->preview_scale_);
    DrawCircleAtPos(0.5f, 0.0f, boxTransform, obs_preview->preview_scale_);
    DrawCircleAtPos(0.0f, 0.5f, boxTransform, obs_preview->preview_scale_);
    DrawCircleAtPos(0.5f, 1.0f, boxTransform, obs_preview->preview_scale_);
    DrawCircleAtPos(1.0f, 0.5f, boxTransform, obs_preview->preview_scale_);

    gs_matrix_push();
    gs_matrix_scale3f(obs_preview->preview_scale_, obs_preview->preview_scale_, 1.0f);
    gs_matrix_mul(&boxTransform);

    obs_sceneitem_crop crop;
    obs_sceneitem_get_crop(item, &crop);

    if (info.bounds_type == OBS_BOUNDS_NONE && crop_enabled(&crop)) {
        vec4 color;
        gs_effect_t *eff = gs_get_effect();
        gs_eparam_t *param = gs_effect_get_param_by_name(eff, "color");

#define DRAW_SIDE(side, vb) \
        if (crop.side > 0) \
            vec4_set(&color, 0.0f, 1.0f, 0.0f, 1.0f); \
        else \
            vec4_set(&color, 1.0f, 0.0f, 0.0f, 1.0f); \
        gs_effect_set_vec4(param, &color); \
        gs_load_vertexbuffer(obs_preview->vb); \
        gs_draw(GS_LINESTRIP, 0, 0);

        DRAW_SIDE(left,   box_left_);
        DRAW_SIDE(top,    box_top_);
        DRAW_SIDE(right,  box_right_);
        DRAW_SIDE(bottom, box_bottom_);
#undef DRAW_SIDE
    } else {
        gs_load_vertexbuffer(obs_preview->box_);
        gs_draw(GS_LINESTRIP, 0, 0);
    }

    gs_matrix_pop();

    UNUSED_PARAMETER(scene);
    UNUSED_PARAMETER(param);
    return true;
}

void OBSPreview::InitGraphicPrimitives() {

    obs_enter_graphics();

    gs_render_start(true);
    gs_vertex2f(0.0f, 0.0f);
    gs_vertex2f(0.0f, 1.0f);
    gs_vertex2f(1.0f, 1.0f);
    gs_vertex2f(1.0f, 0.0f);
    gs_vertex2f(0.0f, 0.0f);
    box_ = gs_render_save();

    gs_render_start(true);
    gs_vertex2f(0.0f, 0.0f);
    gs_vertex2f(0.0f, 1.0f);
    box_left_ = gs_render_save();

    gs_render_start(true);
    gs_vertex2f(0.0f, 0.0f);
    gs_vertex2f(1.0f, 0.0f);
    box_top_ = gs_render_save();

    gs_render_start(true);
    gs_vertex2f(1.0f, 0.0f);
    gs_vertex2f(1.0f, 1.0f);
    box_right_ = gs_render_save();

    gs_render_start(true);
    gs_vertex2f(0.0f, 1.0f);
    gs_vertex2f(1.0f, 1.0f);
    box_bottom_ = gs_render_save();

    gs_render_start(true);
    for (int i = 0; i <= 360; i += (360/20)) {
        float pos = RAD(float(i));
        gs_vertex2f(cosf(pos), sinf(pos));
    }
    circle_ = gs_render_save();

    obs_leave_graphics();
}

void OBSPreview::CreateDisplay() {

    if (display_)
        return;

    InitGraphicPrimitives();

    gfx::Rect rect = controller_->GetViewRect();
    gs_init_data info = {};
    info.cx = rect.width();
    info.cy = rect.height();
    info.format = GS_RGBA;
    info.zsformat = GS_ZS_NONE;
    info.window.hwnd = controller_->GetNativeHWND();

    display_.reset(new OBSObjWrapper<OBSDisplay>());
    display_->obj = obs_display_create(&info, 0);

    AddDisplayRender();
    is_bind_render_ = true;
}

void OBSPreview::AddDisplayRender()
{
    RecalculateDisplayViewport();

    if (display_) {
        obs_display_add_draw_callback(display_->obj, OBSPreview::Render, this);
    }
}

void OBSPreview::ResetSourcesPos(obs_sceneitem_t* si, obs_video_info& old_ovi) {

    obs_video_info new_ovi;
    if (obs_get_video_info(&new_ovi)) {

        struct vec2 old_pos;
        obs_sceneitem_get_pos(si, &old_pos);

        if (((uint32_t)old_pos.x > new_ovi.output_width) ||
            ((uint32_t)old_pos.y > new_ovi.output_height)) {

            struct vec2 new_pos;
            new_pos.x = old_pos.x / (float)old_ovi.output_width * new_ovi.output_width;
            new_pos.y = old_pos.y / (float)old_ovi.output_height * new_ovi.output_height;
            obs_sceneitem_set_pos(si, &new_pos);
        }
    }
}

void OBSPreview::SetClearOnly(bool clear_only) {
    should_clear_only_.store(clear_only, std::memory_order_relaxed);
}

void OBSPreview::GetBackgroundColor(float* r, float* g, float* b)
{
    *r = background_color_[0];
    *g = background_color_[1];
    *b = background_color_[2];
}

void OBSPreview::SetBackgroundColor(float r, float g, float b)
{
    background_color_[0] = r;
    background_color_[1] = g;
    background_color_[2] = b;
}

int OBSPreview::GetHTComponent(const gfx::Point &location)
{
    switch (preview_type_) {
    case PreviewType::PREVIEW_MAIN:
        return __super::GetHTComponent(location);

    default:
        break;
    }

    return obs_proxy_ui::ARROW;
}

void OBSPreview::DrawBackdrop(float cx, float cy) {

    if (!box_)
        return;

    gs_effect_t    *solid = obs_get_base_effect(OBS_EFFECT_SOLID);
    gs_eparam_t    *color = gs_effect_get_param_by_name(solid, "color");
    gs_technique_t *tech  = gs_effect_get_technique(solid, "Solid");

    vec4 colorVal;
    vec4_set(&colorVal, 0.0f, 0.0f, 0.0f, 1.0f);
    gs_effect_set_vec4(color, &colorVal);

    gs_technique_begin(tech);
    gs_technique_begin_pass(tech, 0);
    gs_matrix_push();
    gs_matrix_identity();
    gs_matrix_scale3f(float(cx), float(cy), 1.0f);

    gs_load_vertexbuffer(box_);
    gs_draw(GS_TRISTRIP, 0, 0);

    gs_matrix_pop();
    gs_technique_end_pass(tech);
    gs_technique_end(tech);

    gs_load_vertexbuffer(nullptr);
}

void OBSPreview::DrawSceneEditing() {
    auto scene = GetScene();
    if (!scene)
    {
        return;
    }

    gs_effect_t    *solid = obs_get_base_effect(OBS_EFFECT_SOLID);
    gs_technique_t *tech = gs_effect_get_technique(solid, "Solid");

    vec4 color;
    vec4_set(&color, 1.0f, 0.0f, 0.0f, 1.0f);
    gs_effect_set_vec4(gs_effect_get_param_by_name(solid, "color"), &color);

    gs_technique_begin(tech);
    gs_technique_begin_pass(tech, 0);

    obs_scene_enum_items(scene, DrawSelectedItem, this);

    gs_load_vertexbuffer(nullptr);

    gs_technique_end_pass(tech);
    gs_technique_end(tech);
}

gfx::Size OBSPreview::DetermineLogicRenderSize()
{
    // 连麦场景的渲染区横屏时与主场景一致，竖屏时宽为主场景宽度两倍，高度基于此按16:9再算；
    // 单纯的源渲染时预览区的实际渲染尺寸采用源自身的大小尺寸；
    // 其他场景的渲染预览区的实际渲染尺寸统一采用全局的推流分辨率尺寸；
    gfx::Size size(kDefaultRenderWidth, kDefaultRenderHeight);
    switch (preview_type_)
    {
    case PreviewType::PREVIEW_MAIN:
    case PreviewType::PREVIEW_PROJECTOR:
    case PreviewType::PREVIEW_MAIN_PREVIEW:
        {
            struct obs_video_info ovi;
            if (obs_get_video_info(&ovi))
            {
                size.SetSize(ovi.base_width, ovi.base_height);
            }
        }
        break;
    case PreviewType::PREVIEW_SOURCE:
        {
            obs_source_t* source = controller_->GetRenderSource();
            //DCHECK(source);
            if (source)
            {
                size.SetSize(obs_source_get_width(source), obs_source_get_height(source));
            }
        }
        break;
    default:
        NOTREACHED() << "Unknown preview type";
        break;
    }
    return size;
}

gfx::Size OBSPreview::RecalculateDisplayViewport()
{
    gfx::Size render_size = DetermineLogicRenderSize();

    gfx::Rect rect = controller_->GetViewRect();
    GetScaleAndCenterPos(render_size.width(), render_size.height(),
                         rect.width() - PREVIEW_EDGE_SIZE * 2,
                         rect.height() - PREVIEW_EDGE_SIZE * 2,
                         preview_x_, preview_y_, preview_cx_, preview_cy_, preview_scale_);

    preview_x_ += float(PREVIEW_EDGE_SIZE);
    preview_y_ += float(PREVIEW_EDGE_SIZE);
    return render_size;
}

void OBSPreview::OnOBSPreviewControlMouseEvent(ui::MouseEvent *event)
{
    switch (preview_type_)
    {
    case PreviewType::PREVIEW_MAIN:
        OnMainPreviewMouseEvent(event);
        break;

    case PreviewType::PREVIEW_SOURCE:
        OnSourcePreviewMouseEvent(event);
        break;

    default:
        break;
    }
}

void OBSPreview::OnOBSPreviewControlMouseWheelEvent(ui::MouseWheelEvent *event)
{
    switch (preview_type_)
    {
    case PreviewType::PREVIEW_SOURCE:
        OnSourcePreviewMouseWheelEvent(event);
        break;
    default:
        break;
    }
}

void OBSPreview::OnOBSPreviewControlKeyEvent(ui::KeyEvent *event)
{
    switch (preview_type_)
    {
    case PreviewType::PREVIEW_MAIN:
        OnMainPreviewKeyEvent(event);
        break;

    case PreviewType::PREVIEW_SOURCE:
        OnSourcePreviewKeyEvent(event);
        break;

    default:
        break;
    }
}

void OBSPreview::OnOBSPreviewControlFocusEvent(bool focus)
{
    switch (preview_type_)
    {
    case PreviewType::PREVIEW_SOURCE:
        OnSourcePreviewFocusEvent(focus);
        break;
    default:
        break;
    }
}

void OBSPreview::OnOBSPreviewControlVisibility(bool visible) {
    should_render_.store(visible, std::memory_order_relaxed);
}

void OBSPreview::OnOBSPreviewControlSizeChanged(const gfx::Rect& bounds)
{
#if 0
    CreateDisplay();
#else
    if (!display_)
        return;
#endif

    // 上层预览控件尺寸变化时，底层根据需要渲染的东西（场景、单一源）的尺寸计算实际渲染区的缩放率
    RecalculateDisplayViewport();

    // 更新obs画布的大小
     gfx::Rect rect = controller_->GetViewRect();
    obs_display_resize(display_->obj, rect.width(), rect.height());
}

void OBSPreview::OnOBSPreviewControlDestorying()
{
    if (display_) {
        obs_display_remove_draw_callback(display_->obj, OBSPreview::Render, this);
    }
    is_bind_render_ = false;
}

// PREVIEW_MAIN
void OBSPreview::OnMainPreviewMouseEvent(ui::MouseEvent *event)
{
    DCHECK(preview_type_ == PreviewType::PREVIEW_MAIN);

    switch (event->type())
    {
    case ui::ET_MOUSE_PRESSED:
    {
        MousePressed(event);
        break;
    }
    case ui::ET_MOUSE_RELEASED:
    {
        MouseReleased(event);
        break;
    }
    case ui::ET_MOUSE_MOVED:
    case ui::ET_MOUSE_DRAGGED:
    {
        MouseMoved(event);
        break;
    }
    case ui::ET_MOUSE_EXITED:
        break;
    case ui::ET_MOUSEWHEEL:
        break;
    default:
        break;
    }
}

void OBSPreview::OnMainPreviewKeyEvent(ui::KeyEvent *event)
{
    DCHECK(preview_type_ == PreviewType::PREVIEW_MAIN);

    auto scene = GetScene();
    if (!scene)
    {
        return;
    }

    enum class MoveDir
    {
        Up,
        Down,
        Left,
        Right
    };

    auto nude = [scene](int dist, MoveDir dir) {

        struct MoveInfo
        {
            float dist;
            MoveDir dir;
        } info = { (float)dist, dir };

        auto func = [](obs_scene_t*, obs_sceneitem_t *item, void *param) {

            MoveInfo *info = reinterpret_cast<MoveInfo*>(param);
            struct vec2 dir;
            struct vec2 pos;

            vec2_set(&dir, 0.0f, 0.0f);

            if (!obs_sceneitem_selected(item) || obs_sceneitem_locked(item))
            {
                return true;
            }

            switch (info->dir)
            {
            case MoveDir::Up:    dir.y = -info->dist; break;
            case MoveDir::Down:  dir.y = info->dist; break;
            case MoveDir::Left:  dir.x = -info->dist; break;
            case MoveDir::Right: dir.x = info->dist; break;
            }

            obs_sceneitem_get_pos(item, &pos);
            vec2_add(&pos, &pos, &dir);
            obs_sceneitem_set_pos(item, &pos);
            return true;
        };

        obs_scene_enum_items(scene, func, &info);
    };

    int dist = 1;

    switch (event->key_code())
    {
    case ui::VKEY_UP:
    {
        nude(dist, MoveDir::Up);
        break;
    }
    case ui::VKEY_DOWN:
    {
        nude(dist, MoveDir::Down);
        break;
    }
    case ui::VKEY_LEFT:
    {
        nude(dist, MoveDir::Left);
        break;
    }
    case ui::VKEY_RIGHT:
    {
        nude(dist, MoveDir::Right);
        break;
    }
    default:
        break;
    }
}

// PREVIEW_SOURCE
bool OBSPreview::GetSourceRelativeXY(
    int mouseX, int mouseY, int &relX, int &relY)
{
    uint32_t sourceCX = std::max(obs_source_get_width(controller_->GetRenderSource()), 1u);
    uint32_t sourceCY = std::max(obs_source_get_height(controller_->GetRenderSource()), 1u);

    if (preview_x_ > 0)
    {
        relX = int(float(mouseX - preview_x_) / preview_scale_);
        relY = int(float(mouseY / preview_scale_));
    }
    else
    {
        relX = int(float(mouseX / preview_scale_));
        relY = int(float(mouseY - preview_y_) / preview_scale_);
    }

    // Confirm mouse is inside the source
    if (relX < 0 || relX > int(sourceCX))
        return false;
    if (relY < 0 || relY > int(sourceCY))
        return false;

    return true;
}

void OBSPreview::OnSourcePreviewMouseEvent(ui::MouseEvent *event)
{
    DCHECK(preview_type_ == PreviewType::PREVIEW_SOURCE);

    switch (event->type())
    {
    case ui::ET_MOUSE_PRESSED:
    case ui::ET_MOUSE_RELEASED:
    {
        struct obs_mouse_event mouseEvent = {};
        mouseEvent.modifiers = TranslateQtMouseEventModifiers(event);
        bool insideSource = GetSourceRelativeXY(event->x(), event->y(),
                                                mouseEvent.x, mouseEvent.y);

        bool mouseUp = (event->type() == ui::ET_MOUSE_RELEASED);
        int clickCount = event->GetClickCount();
        // bililive[
        if (mouseUp && clickCount == 0) {
            clickCount = 1;
        }
        // ]bililive

        int32_t button = 0;

        if (event->IsLeftMouseButton())
            button = MOUSE_LEFT;
        if (event->IsMiddleMouseButton())
            button = MOUSE_MIDDLE;
        if (event->IsRightMouseButton())
            button = MOUSE_RIGHT;

        if (mouseUp || insideSource)
            obs_source_send_mouse_click(controller_->GetRenderSource(), &mouseEvent, button,
                                        mouseUp, clickCount);
    }
    break;
    case ui::ET_MOUSE_ENTERED:
    case ui::ET_MOUSE_MOVED:
    case ui::ET_MOUSE_DRAGGED:
    case ui::ET_MOUSE_EXITED:
    {
        struct obs_mouse_event mouseEvent = {};

        bool mouseLeave = (event->type() == ui::ET_MOUSE_EXITED);

        if (!mouseLeave)
        {
            mouseEvent.modifiers = TranslateQtMouseEventModifiers(event);
            mouseLeave = !GetSourceRelativeXY(event->x(), event->y(),
                                              mouseEvent.x, mouseEvent.y);
        }

        obs_source_send_mouse_move(controller_->GetRenderSource(), &mouseEvent, mouseLeave);
    }
    break;
    default:
        break;
    }
}

void OBSPreview::OnSourcePreviewMouseWheelEvent(ui::MouseWheelEvent *event)
{
    DCHECK(preview_type_ == PreviewType::PREVIEW_SOURCE);

    switch (event->type())
    {
    case ui::ET_MOUSEWHEEL:
    {
        struct obs_mouse_event mouseEvent = {};
        mouseEvent.modifiers = TranslateQtKeyboardEventModifiers(event, true);

        if (GetSourceRelativeXY(event->x(), event->y(),
                                mouseEvent.x, mouseEvent.y))
        {
            obs_source_send_mouse_wheel(controller_->GetRenderSource(), &mouseEvent,
                                        event->x_offset(), event->y_offset());
        }
    }
    break;
    default:
        break;
    }
}

void OBSPreview::OnSourcePreviewKeyEvent(ui::KeyEvent *event)
{
    DCHECK(preview_type_ == PreviewType::PREVIEW_SOURCE);

    base::NativeEvent msg = event->native_event();

    struct obs_key_event keyEvent = { 0 };

    std::string text;
    if (event->is_char())
    {
        wchar_t ch[2] = { event->GetCharacter(), 0 };
        text = base::WideToUTF8(std::wstring(ch));
    }
    keyEvent.text = const_cast<char*>(text.c_str());
    keyEvent.modifiers = TranslateQtKeyboardEventModifiers(event, false);
    // 这个nativeModifiers不知道到底对应native msg的哪个地方，暂时按照obs里面QKeyEvent的值来搞
    //keyEvent.native_modifiers = event->nativeModifiers();
    uint32_t native_modifiers = (::GetKeyState(VK_CAPITAL) & 1) ? 0x300 : 0x200;
    if (base::win::IsShiftPressed())
        native_modifiers |= 1;
    if (base::win::IsCtrlPressed())
        native_modifiers |= 2;
    if (base::win::IsAltPressed())
        native_modifiers |= 4;
    keyEvent.native_modifiers = native_modifiers;
    keyEvent.native_scancode = (msg.lParam & 0xFF0000) >> 16;
    keyEvent.native_vkey = MapVirtualKey(keyEvent.native_scancode, MAPVK_VSC_TO_VK);

    bool keyUp = (event->type() == ui::ET_KEY_RELEASED);

    // 对于Ctrl+V，OBS只处理WM_CHAR，不处理WM_KEYDOWN，我们也这样处理
    if (!keyUp)
    {
        if ((keyEvent.native_vkey == ui::VKEY_V) && // V
            (native_modifiers & 2) && // Ctrl
            !event->is_char() // WM_KEYDOWN
            )
        {
            return;
        }
    }

    obs_source_send_key_click(controller_->GetRenderSource(), &keyEvent, keyUp);
}

void OBSPreview::OnSourcePreviewFocusEvent(bool focus)
{
    DCHECK(preview_type_ == PreviewType::PREVIEW_SOURCE);

    obs_source_send_focus(controller_->GetRenderSource(), focus);
}

}//namespace obs_proxy_ui