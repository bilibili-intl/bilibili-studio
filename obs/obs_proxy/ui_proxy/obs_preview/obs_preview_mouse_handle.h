#ifndef BILILIVE_OBS_OBS_PROXY_UI_PROXY_OBS_PREVIEW_OBS_PREVIEW_MOUSE_HANDLE_H_
#define BILILIVE_OBS_OBS_PROXY_UI_PROXY_OBS_PREVIEW_OBS_PREVIEW_MOUSE_HANDLE_H_

#include "obs/obs-studio/libobs/graphics/matrix4.h"
#include "obs/obs-studio/libobs/obs.hpp"

#include "obs/obs_proxy/public/proxy/obs_proxy.h"

#include "base/memory/scoped_ptr.h"
#include "base/strings/string16.h"

#include "ui/gfx/point.h"

namespace ui{
class MouseEvent;
}

namespace obs_proxy_ui{

    class UIProxyImpl;

#define HANDLE_RADIUS     4.0f
#define HANDLE_SEL_RADIUS (HANDLE_RADIUS * 1.5f)

#define ITEM_LEFT   (1<<0)
#define ITEM_RIGHT  (1<<1)
#define ITEM_TOP    (1<<2)
#define ITEM_BOTTOM (1<<3)

enum MOUSEPOS
{
    ARROW = 32512,
    SIZENWSE = 32642,
    SIZENESW = 32643,
    SIZEWE = 32644,
    SIZENS = 32645,
    SIZEALL = 32646
};

enum class ItemHandle : uint32_t {
	None         = 0,
	TopLeft      = ITEM_TOP | ITEM_LEFT,
	TopCenter    = ITEM_TOP,
	TopRight     = ITEM_TOP | ITEM_RIGHT,
	CenterLeft   = ITEM_LEFT,
	CenterRight  = ITEM_RIGHT,
	BottomLeft   = ITEM_BOTTOM | ITEM_LEFT,
	BottomCenter = ITEM_BOTTOM,
	BottomRight  = ITEM_BOTTOM | ITEM_RIGHT
};

static bool CloseFloat(float a, float b, float epsilon=0.01) {

	using std::abs;
	return abs(a-b) <= epsilon;
}

static inline bool crop_enabled(const obs_sceneitem_crop *crop) {

	return crop->left > 0  ||
	       crop->top > 0   ||
	       crop->right > 0 ||
	       crop->bottom > 0;
}

class OBSPreviewMouseHandle
{
	struct AlignedDeleter {
		void operator()(void* p)const {
			_aligned_free(p);
		}
	};

public:
	explicit OBSPreviewMouseHandle(PreviewType type);
	virtual ~OBSPreviewMouseHandle();

    float preview_scale() const { return preview_scale_; }

protected:
    virtual int GetHTComponent(const gfx::Point &location);

    // mouse event
    void MousePressed(ui::MouseEvent *event);
    void MouseMoved(ui::MouseEvent *event);
    void MouseReleased(ui::MouseEvent *event);

    PreviewType preview_type_;

private:
    void ResetDetails();

    // item operate
    void DoSelect(const vec2 &pos);
    //void DoCtrlSelect(const vec2 &pos);
    void CropItem(const vec2 &pos);
    void StretchItem(ui::MouseEvent *event, const vec2 &pos);
    void SnapItemStretch(vec3 &tl, vec3 &br);
    void MoveItems(ui::MouseEvent *event, const vec2 &pos);
    void SnapItemMovement(vec2 &offset);

    void InitSelectedItemDragInfo(const vec2 &pos);
    void SetDragMode();
    void SetAltShift(int cmd);
    vec2 GetMouseEventPos(ui::MouseEvent *event);
    void TryToSelectItemAtPos(ui::MouseEvent *event, const vec2 &pos);
    vec3 GetSnapOffset(const vec3 &tl, const vec3 &br);
    OBSSceneItem GetItemAtPos(const vec2 &pos, bool selectBelow);
    void ClampAspect(vec3 &tl, vec3 &br, vec2 &size, const vec2 &baseSize);
    vec3 CalculateStretchPos(const vec3 &tl, const vec3 &br);
    OBSSceneItem IsItemSelectedAtPos(const vec2 &pos);

protected:
    int preview_x_ = 0;
    int preview_y_ = 0;
    int preview_cx_ = 0;
    int preview_cy_ = 0;
    float preview_scale_ = 0.0f;

    obs_scene_t* GetScene() const;

private:
    friend class UIProxyImpl;

    OBSSceneItem hover_item_;// 鼠标落在item矩形范围点的项
    OBSSceneItem hittest_item_;// 鼠标落在周围八个拖拽点的项（落点不一定在item矩形范围内）
    obs_sceneitem_crop start_crop_;
    vec2         start_item_pos_;
    vec2         crop_size_;
    vec2         stretch_item_size_;
    ItemHandle   hittest_handle_ = ItemHandle::None;

    vec2 start_pos_;
    vec2 last_move_offset_;
    bool mouse_down_ = false;
    bool mouse_moved_ = false;
    bool is_cropping_ = false;

    int always_shift_ = 0;
    int always_alt_ = 0;

    scoped_ptr<matrix4, AlignedDeleter> screen_to_item_;
    scoped_ptr<matrix4, AlignedDeleter> item_to_screen_;

    DISALLOW_COPY_AND_ASSIGN(OBSPreviewMouseHandle);
};

}// obs_proxy_ui

#endif // BILILIVE_OBS_OBS_PROXY_UI_PROXY_OBS_PREVIEW_OBS_PREVIEW_MOUSE_HANDLE_H_