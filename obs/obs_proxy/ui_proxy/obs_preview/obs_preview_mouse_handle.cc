#include "obs/obs_proxy/ui_proxy/obs_preview/obs_preview_mouse_handle.h"

#include "ui/base/events/event.h"

#include "base/prefs/pref_service.h"
#include "base/strings/utf_string_conversions.h"

#include "obs/obs_proxy/core_proxy/core_proxy_impl.h"
#include "obs/obs_proxy/core_proxy/scene_collection/obs_sources_wrapper_impl.h"
#include "obs/obs_proxy/ui_proxy/ui_proxy_impl.h"
#include "obs/obs_proxy/public/common/pref_constants.h"
#include "obs/obs_proxy/public/common/pref_names.h"
#include "obs/obs_proxy/app/obs_proxy_access_stub.h"


namespace obs_proxy_ui{

struct HandleFindData {
	const vec2   &pos;
	const float  scale;

	OBSSceneItem item;
	ItemHandle   handle = ItemHandle::None;

	HandleFindData(const HandleFindData &) = delete;
	HandleFindData(HandleFindData &&) = delete;
	HandleFindData& operator=(const HandleFindData &) = delete;
	HandleFindData& operator=(HandleFindData &&) = delete;

	inline HandleFindData(const vec2 &pos_, float scale_)
		: pos   (pos_),
		  scale (scale_)
	{}
};

struct SceneFindData {
	const vec2   &pos;
	OBSSceneItem item;
	bool         selectBelow;

	SceneFindData(const SceneFindData &) = delete;
	SceneFindData(SceneFindData &&) = delete;
	SceneFindData& operator=(const SceneFindData &) = delete;
	SceneFindData& operator=(SceneFindData &&) = delete;

	inline SceneFindData(const vec2 &pos_, bool selectBelow_)
		: pos         (pos_),
		  selectBelow (selectBelow_)
	{}
};

struct SelectedItemBounds {
	bool first = true;
	vec3 tl, br;
};

struct OffsetData {
	float clampDist;
	vec3 tl, br, offset;
};



static float maxfunc(float x, float y) {

	return x > y ? x : y;
}

static float minfunc(float x, float y) {

	return x < y ? x : y;
}

static inline vec2 GetOBSScreenSize() {

	obs_video_info ovi;
	vec2 size;
	vec2_zero(&size);

	if (obs_get_video_info(&ovi)) {
		size.x = float(ovi.base_width);
		size.y = float(ovi.base_height);
	}

	return size;
}

static vec3 GetTransformedPos(float x, float y, const matrix4 &mat) {

	vec3 result;
	vec3_set(&result, x, y, 0.0f);
	vec3_transform(&result, &result, &mat);
	return result;
}

static bool AddItemBounds(obs_scene_t *scene, obs_sceneitem_t *item,
		void *param) {

	SelectedItemBounds *data = reinterpret_cast<SelectedItemBounds*>(param);

	if (!obs_sceneitem_selected(item))
		return true;

	matrix4 boxTransform;
	obs_sceneitem_get_box_transform(item, &boxTransform);

	vec3 t[4] = {
		GetTransformedPos(0.0f, 0.0f, boxTransform),
		GetTransformedPos(1.0f, 0.0f, boxTransform),
		GetTransformedPos(0.0f, 1.0f, boxTransform),
		GetTransformedPos(1.0f, 1.0f, boxTransform)
	};

	for (const vec3 &v : t) {
		if (data->first) {
			vec3_copy(&data->tl, &v);
			vec3_copy(&data->br, &v);
			data->first = false;
		} else {
			vec3_min(&data->tl, &data->tl, &v);
			vec3_max(&data->br, &data->br, &v);
		}
	}

	UNUSED_PARAMETER(scene);
	return true;
}

static bool GetSourceSnapOffset(obs_scene_t *scene, obs_sceneitem_t *item,
		void *param) {

	OffsetData *data = reinterpret_cast<OffsetData*>(param);

	if (obs_sceneitem_selected(item))
		return true;

	matrix4 boxTransform;
	obs_sceneitem_get_box_transform(item, &boxTransform);

	vec3 t[4] = {
		GetTransformedPos(0.0f, 0.0f, boxTransform),
		GetTransformedPos(1.0f, 0.0f, boxTransform),
		GetTransformedPos(0.0f, 1.0f, boxTransform),
		GetTransformedPos(1.0f, 1.0f, boxTransform)
	};

	bool first = true;
	vec3 tl, br;
	vec3_zero(&tl);
	vec3_zero(&br);
	for (const vec3 &v : t) {
		if (first) {
			vec3_copy(&tl, &v);
			vec3_copy(&br, &v);
			first = false;
		} else {
			vec3_min(&tl, &tl, &v);
			vec3_max(&br, &br, &v);
		}
	}

	// Snap to other source edges
#define EDGE_SNAP(l, r, x, y) \
	do { \
		double dist = fabsf(l.x - data->r.x); \
		if (dist < data->clampDist && \
		    fabsf(data->offset.x) < EPSILON && \
		    data->tl.y < br.y && \
		    data->br.y > tl.y && \
		    (fabsf(data->offset.x) > dist || data->offset.x < EPSILON)) \
			data->offset.x = l.x - data->r.x; \
	} while (false)

	EDGE_SNAP(tl, br, x, y);
	EDGE_SNAP(tl, br, y, x);
	EDGE_SNAP(br, tl, x, y);
	EDGE_SNAP(br, tl, y, x);
#undef EDGE_SNAP

	UNUSED_PARAMETER(scene);
	return true;
}

static bool move_items(obs_scene_t *scene, obs_sceneitem_t *item, void *param) {

	vec2 *offset = reinterpret_cast<vec2*>(param);

	if (obs_sceneitem_selected(item) && !obs_sceneitem_locked(item))
    {
		vec2 pos;
		obs_sceneitem_get_pos(item, &pos);
		vec2_add(&pos, &pos, offset);
		obs_sceneitem_set_pos(item, &pos);
	}

	UNUSED_PARAMETER(scene);
	return true;
}

static bool select_one(obs_scene_t *scene, obs_sceneitem_t *item, void *param) {

	obs_sceneitem_t *selectedItem =
		reinterpret_cast<obs_sceneitem_t*>(param);
	obs_sceneitem_select(item, (selectedItem == item));

	UNUSED_PARAMETER(scene);
	return true;
}

static bool SceneItemHasVideo(obs_sceneitem_t *item) {

	obs_source_t *source = obs_sceneitem_get_source(item);
	uint32_t flags = obs_source_get_output_flags(source);
	return (flags & OBS_SOURCE_VIDEO) != 0;
}

static bool FindItemAtPos(obs_scene_t *scene, obs_sceneitem_t *item,
		void *param) {

	SceneFindData *data = reinterpret_cast<SceneFindData*>(param);
	matrix4       transform;
	matrix4       invTransform;
	vec3          transformedPos;
	vec3          pos3;
	vec3          pos3_;

	if (!SceneItemHasVideo(item))
    {
        return true;
    }

	vec3_set(&pos3, data->pos.x, data->pos.y, 0.0f);

	obs_sceneitem_get_box_transform(item, &transform);

	matrix4_inv(&invTransform, &transform);
	vec3_transform(&transformedPos, &pos3, &invTransform);
	vec3_transform(&pos3_, &transformedPos, &transform);

	if (CloseFloat(pos3.x, pos3_.x) && CloseFloat(pos3.y, pos3_.y) &&
	    transformedPos.x >= 0.0f && transformedPos.x <= 1.0f &&
	    transformedPos.y >= 0.0f && transformedPos.y <= 1.0f)
    {
        // 由底层往顶层枚举，枚举到当前选中项时若在先前就已经找到了层级低于它的项则直接返回，
        // 若先前仍未找到层级低于它的项则寻找层级高于它的项，中途不中止，直至最顶层
		if (data->selectBelow && obs_sceneitem_selected(item))
        {
			if (data->item)
            {
                // 找到了紧挨着当前选中项的下一层级的项了
                return false;
            }
			else
            {
                data->selectBelow = false;
            }
		}

        //// 被锁定的项直接穿透过去
        //if (!obs_sceneitem_locked(item))
        //{
        //    data->item = item;
        //}
        data->item = item;
	}

	UNUSED_PARAMETER(scene);
	return true;
}

static bool CheckItemSelectedAtPos(obs_scene_t *scene, obs_sceneitem_t *item, void *param)
{
	SceneFindData *data = reinterpret_cast<SceneFindData*>(param);
	matrix4       transform;
	vec3          transformedPos;
	vec3          pos3;

	if (!SceneItemHasVideo(item))
    {
        return true;
    }

	vec3_set(&pos3, data->pos.x, data->pos.y, 0.0f);

	obs_sceneitem_get_box_transform(item, &transform);

	matrix4_inv(&transform, &transform);
	vec3_transform(&transformedPos, &pos3, &transform);

    if (transformedPos.x >= 0.0f && transformedPos.x <= 1.0f &&
        transformedPos.y >= 0.0f && transformedPos.y <= 1.0f)
    {
        if (obs_sceneitem_selected(item))
        {
            data->item = item;
            return false;
        }
    }

	UNUSED_PARAMETER(scene);
	return true;
}

static vec2 GetItemSize(obs_sceneitem_t *item) {

	obs_bounds_type boundsType = obs_sceneitem_get_bounds_type(item);
	vec2 size;

	if (boundsType != OBS_BOUNDS_NONE) {
		obs_sceneitem_get_bounds(item, &size);
	} else {
		obs_source_t *source = obs_sceneitem_get_source(item);
		obs_sceneitem_crop crop;
		vec2 scale;

		obs_sceneitem_get_scale(item, &scale);
		obs_sceneitem_get_crop(item, &crop);
		size.x = float(obs_source_get_width(source) -
				crop.left - crop.right) * scale.x;
		size.y = float(obs_source_get_height(source) -
				crop.top - crop.bottom) * scale.y;
	}

	return size;
}

static vec3 GetTransformedPosScaled(float x, float y, const matrix4 &mat,
		float scale) {

	vec3 result;
	vec3_set(&result, x, y, 0.0f);
	vec3_transform(&result, &result, &mat);
	vec3_mulf(&result, &result, scale);
	return result;
}

static bool ItemHitTestAtPos(obs_scene_t *scene, obs_sceneitem_t *item, void *param)
{
	if (!obs_sceneitem_selected(item))
    {
        return true;
    }

    HandleFindData *data = reinterpret_cast<HandleFindData*>(param);
	matrix4        transform;
	vec3           pos3;
	float          closestHandle = HANDLE_SEL_RADIUS;

	vec3_set(&pos3, data->pos.x, data->pos.y, 0.0f);

	obs_sceneitem_get_box_transform(item, &transform);

	auto TestHandle = [&] (float x, float y, ItemHandle handle) {

		vec3 handlePos = GetTransformedPosScaled(x, y, transform, data->scale);

		float dist = vec3_dist(&handlePos, &pos3);
		if (dist < HANDLE_SEL_RADIUS) {
			if (dist < closestHandle) {
                closestHandle = dist;
				data->handle  = handle;
                data->item    = item;
			}
		}
	};

	TestHandle(0.0f, 0.0f, ItemHandle::TopLeft);
	TestHandle(0.5f, 0.0f, ItemHandle::TopCenter);
	TestHandle(1.0f, 0.0f, ItemHandle::TopRight);
	TestHandle(0.0f, 0.5f, ItemHandle::CenterLeft);
	TestHandle(1.0f, 0.5f, ItemHandle::CenterRight);
	TestHandle(0.0f, 1.0f, ItemHandle::BottomLeft);
	TestHandle(0.5f, 1.0f, ItemHandle::BottomCenter);
	TestHandle(1.0f, 1.0f, ItemHandle::BottomRight);

	UNUSED_PARAMETER(scene);
	return true;
}


// -*- OBSPreviewMouseHandle -*-

OBSPreviewMouseHandle::OBSPreviewMouseHandle(PreviewType type)
    : preview_type_(type)
{
    screen_to_item_.reset((matrix4 *)_aligned_malloc(sizeof(matrix4), 16));
    item_to_screen_.reset((matrix4 *)_aligned_malloc(sizeof(matrix4), 16));
    ResetDetails();
}

OBSPreviewMouseHandle::~OBSPreviewMouseHandle() {
}

void OBSPreviewMouseHandle::ResetDetails()
{
    hover_item_ = nullptr;
    hittest_item_ = nullptr;
    mouse_down_ = false;
    mouse_moved_ = false;
    is_cropping_ = false;
    hittest_handle_ = ItemHandle::None;
}

void OBSPreviewMouseHandle::SetDragMode()
{
    std::string drag_mode = obs_proxy::GetPrefs()->GetString(prefs::kSourceDragMode);
    int cmd = 0;
    if (drag_mode == prefs::kSourceDragModeStretch)
        cmd = 1;
    else if (drag_mode == prefs::kSourceDragModeCut)
        cmd = 2;
    SetAltShift(cmd);
}

void OBSPreviewMouseHandle::SetAltShift(int cmd)
{
    always_shift_ = cmd & 0x01;
    always_alt_ = cmd >> 1;
}

void OBSPreviewMouseHandle::MousePressed(ui::MouseEvent *event)
{
    // 只接受左键和右键
	if (!(event->IsLeftMouseButton()) && !(event->IsRightMouseButton()))
    {
        return;
    }

    // 交互参考obs的做法，点击的时候不选中具体的item，仅仅只是判断
    // 1、判断鼠标点击位置是否在当前选中项上，在的话则计算该项将处于何种拖拽模式，
    //    同时记录下其当前位置和尺寸参数，以便在MouseMove进行拖拽操作；
    // 2、位置所指的项是否已经被选中了，没被选中的话将在MouseMove和MouseRelease时被选中；

    gfx::Point screen_point = gfx::Point(event->x(), event->y());
	float x = float(screen_point.x()) - preview_x_;
	float y = float(screen_point.y()) - preview_y_;

    if (event->IsLeftMouseButton())
    {
		mouse_down_ = true;
        SetDragMode();
    }

    is_cropping_ = always_alt_ ? true : event->IsAltDown();

	vec2_set(&start_pos_, x, y);
    // 1
    InitSelectedItemDragInfo(start_pos_);

	vec2_divf(&start_pos_, &start_pos_, preview_scale_);
	start_pos_.x = std::round(start_pos_.x);
	start_pos_.y = std::round(start_pos_.y);

    // 2
    hover_item_ = IsItemSelectedAtPos(start_pos_);
	vec2_zero(&last_move_offset_);
}

void OBSPreviewMouseHandle::MouseMoved(ui::MouseEvent *event)
{
	if (mouse_down_)
    {
        vec2 pos = GetMouseEventPos(event);
        pos.x = std::round(pos.x);
        pos.y = std::round(pos.y);
        OBSSceneItem operate_item;

        // 对于MousePressed有三种情况，在MouseMove分别处理：
        // 1、presssed的时候点在了选中项上，但不在边缘拖拽圆圈区域，则移动项；
        // 2、presssed的时候点在了选中项的边缘拖拽圆圈区域内，则裁减拉伸项；
        //    （若是点在外半圈则其实已经不在item区域内了，只是我们允许以这种坐标偏差继续对它操作而已）
        // 3、presssed的时候既没有点在选中项上也没在选中项的拖拽圆圈内，则尝试选中presssed时鼠标所在的项；

        // “hittest_handle_ == ItemHandle::None”条件是防止鼠标超过item区域但是仍在边缘红色的拖拽边框圆圈中
		if (!mouse_moved_ &&
            !hover_item_ &&
            (hittest_handle_ == ItemHandle::None))
        {
            // 3
			TryToSelectItemAtPos(event, start_pos_);
            hover_item_ = IsItemSelectedAtPos(start_pos_);
		}
		mouse_moved_ = true;

        // 2
        if (hittest_handle_ != ItemHandle::None)
        {
            if (!obs_sceneitem_locked(hittest_item_))
            {
                operate_item = hittest_item_;

                if (is_cropping_)
                {
                    CropItem(pos);
                }
                else
                {
                    StretchItem(event, pos);
                }
            }
        }
        //else if (is_mouse_hover_on_selected_items_)
        else if (hover_item_)
        {
            if (!obs_sceneitem_locked(hover_item_))
            {
                operate_item = hover_item_;

                // 1
                MoveItems(event, pos);
            }
        }
        // 移动、缩放、拉伸、裁减有一项被执行了就立刻取消全屏适配标识
        if (operate_item)
        {
            obs_sceneitem_set_fit_to_screen(operate_item, false);
        }
    }
}

void OBSPreviewMouseHandle::MouseReleased(ui::MouseEvent *event)
{
	if (mouse_down_)
    {
		if (!mouse_moved_)
        {
            vec2 pos = GetMouseEventPos(event);
            TryToSelectItemAtPos(event, pos);
        }

        hover_item_ = nullptr;
		hittest_item_ = nullptr;
		mouse_down_   = false;
		mouse_moved_  = false;
		is_cropping_  = false;
	}
}

void OBSPreviewMouseHandle::InitSelectedItemDragInfo(const vec2 &pos)
{
	auto scene = GetScene();
	if (!scene)
    {
        return;
    }

	//todo: sure: device pixel ratio
	HandleFindData data(pos, preview_scale_);
	obs_scene_enum_items(scene, ItemHitTestAtPos, &data);

	hittest_item_     = std::move(data.item);
	hittest_handle_   = data.handle;

	if (hittest_handle_ != ItemHandle::None)
    {
		matrix4 boxTransform;
		vec3    itemUL;
		float   itemRot;

		stretch_item_size_ = GetItemSize(hittest_item_);

		obs_sceneitem_get_box_transform(hittest_item_, &boxTransform);
		itemRot = obs_sceneitem_get_rot(hittest_item_);
		vec3_from_vec4(&itemUL, &boxTransform.t);

		/* build the item space conversion matrices */
		matrix4_identity(item_to_screen_.get());
		matrix4_rotate_aa4f(item_to_screen_.get(), item_to_screen_.get(),
				0.0f, 0.0f, 1.0f, RAD(itemRot));
		matrix4_translate3f(item_to_screen_.get(), item_to_screen_.get(),
				itemUL.x, itemUL.y, 0.0f);

		matrix4_identity(screen_to_item_.get());
		matrix4_translate3f(screen_to_item_.get(), screen_to_item_.get(),
				-itemUL.x, -itemUL.y, 0.0f);
		matrix4_rotate_aa4f(screen_to_item_.get(), screen_to_item_.get(),
				0.0f, 0.0f, 1.0f, RAD(-itemRot));

		obs_sceneitem_get_crop(hittest_item_, &start_crop_);
		obs_sceneitem_get_pos(hittest_item_, &start_item_pos_);

		obs_source_t *source = obs_sceneitem_get_source(hittest_item_);
		crop_size_.x = float(obs_source_get_width(source) -
				start_crop_.left - start_crop_.right);
		crop_size_.y = float(obs_source_get_height(source) -
				start_crop_.top - start_crop_.bottom);
	}
}

int OBSPreviewMouseHandle::GetHTComponent(const gfx::Point &location)
{
    int arrow = ARROW;
	auto scene = GetScene();
    if (!scene)
    {
        return arrow;
    }

    gfx::Point screen_point = gfx::Point(location.x(), location.y());
    float x = float(screen_point.x()) - preview_x_;
    float y = float(screen_point.y()) - preview_y_;

    vec2 pos;
    vec2_set(&pos, x, y);
    vec2_divf(&pos, &pos, preview_scale_);
    pos.x = std::round(pos.x);
    pos.y = std::round(pos.y);
    OBSSceneItem selected_item = IsItemSelectedAtPos(pos);
    if (nullptr == selected_item || obs_sceneitem_locked(selected_item))
    {
        return arrow;
    }

    // IsItemSelectedAtPos.CheckItemSelectedAtPos与ItemHitTestAtPos对于鼠标hover项的检测算法不一致，
    // 导致IsItemSelectedAtPos判断通过但是ItemHitTestAtPos却得出item为空handle为NONE的结果，
    // 所以此两种检测算法似乎可以优化统一，TODO by any one who can do

    vec2_set(&pos, x, y);
    HandleFindData data(pos, preview_scale_);
    obs_scene_enum_items(scene, ItemHitTestAtPos, &data);

    if (data.item)
    {
        // 顺时针顺序排列下，控制点的顺序
        ItemHandle center_orders[] = { ItemHandle::TopCenter, ItemHandle::CenterRight, ItemHandle::BottomCenter, ItemHandle::CenterLeft };
        ItemHandle corner_orders[] = { ItemHandle::TopLeft, ItemHandle::TopRight, ItemHandle::BottomRight, ItemHandle::BottomLeft };

        // 当前鼠标所在位置对应的“原始”控制点是什么
        auto center_handle_index = std::find(center_orders, center_orders + arraysize(center_orders), data.handle) - center_orders;
        auto corner_handle_index = std::find(corner_orders, corner_orders + arraysize(corner_orders), data.handle) - corner_orders;

        // 如果场景元素经过翻转，那么调整顺时针排列下控制点的顺序
        vec2 scale_vec;
        obs_sceneitem_get_scale(data.item, &scale_vec);
        if (scale_vec.x < 0)
        {
            std::swap(center_orders[1], center_orders[3]);
            std::swap(corner_orders[0], corner_orders[1]);
            std::swap(corner_orders[2], corner_orders[3]);
        }
        if (scale_vec.y < 0)
        {
            std::swap(center_orders[0], center_orders[2]);
            std::swap(corner_orders[0], corner_orders[3]);
            std::swap(corner_orders[1], corner_orders[2]);
        }

        // 这个“原始”的控制点被右转（顺时针转）了几次
        int turn_right_count = static_cast<int>(std::round(obs_sceneitem_get_rot(data.item) / 90.0f));

        // 取控制点类型的时候，将顺时针转的次数通过逆时针转给转回来
        if (center_handle_index >= 0 && center_handle_index < arraysize(center_orders))
        {
            center_handle_index = ((center_handle_index - turn_right_count) + arraysize(center_orders) * 2) % arraysize(center_orders);
            bool hasbug = !(center_handle_index >= 0 && center_handle_index < arraysize(center_orders));
            DCHECK(!hasbug);
            if (!hasbug)
                data.handle = center_orders[center_handle_index];
        }
        else if (corner_handle_index >= 0 && corner_handle_index < arraysize(corner_orders))
        {
            corner_handle_index = ((corner_handle_index - turn_right_count) + arraysize(corner_orders) * 2) % arraysize(corner_orders);
            bool hasbug = !(corner_handle_index >= 0 && corner_handle_index < arraysize(corner_orders));
            DCHECK(!hasbug);
            if (!hasbug)
                data.handle = corner_orders[corner_handle_index];
        }
    }

    switch (data.handle)
    {
    case ItemHandle::TopCenter:
    case ItemHandle::BottomCenter:
        arrow = SIZENS;
        break;
    case ItemHandle::CenterLeft:
    case ItemHandle::CenterRight:
        arrow = SIZEWE;
        break;
    case ItemHandle::TopRight:
    case ItemHandle::BottomLeft:
        arrow = SIZENESW;
        break;
    case ItemHandle::TopLeft:
    case ItemHandle::BottomRight:
        arrow = SIZENWSE;
        break;
    default:
        arrow = SIZEALL;
        break;
    }

    return arrow;
}

OBSSceneItem OBSPreviewMouseHandle::IsItemSelectedAtPos(const vec2 &pos)
{
	auto scene = GetScene();
	if (!scene)
    {
        return {};
    }

	SceneFindData data(pos, false);
	obs_scene_enum_items(scene, CheckItemSelectedAtPos, &data);
	return data.item;
}

vec2 OBSPreviewMouseHandle::GetMouseEventPos(ui::MouseEvent *event) {

    gfx::Point screen_point = gfx::Point(event->x(), event->y());

	//todo: sure: device pixel ratio
	float pixel_ratio = 1;
	float scale = pixel_ratio  / preview_scale_;

	vec2 pos;
	vec2_set(&pos,
		(float(screen_point.x()) - preview_x_ / pixel_ratio) * scale,
		(float(screen_point.y()) - preview_y_ / pixel_ratio) * scale);

	return pos;
}

void OBSPreviewMouseHandle::TryToSelectItemAtPos(ui::MouseEvent *event, const vec2 &pos)
{
	/*if (event->IsControlDown())
    {
        DoCtrlSelect(pos);
    }
	else*/
    {
        DoSelect(pos);
    }
}

//void OBSPreviewMouseHandle::DoCtrlSelect(const vec2 &pos)
//{
//	OBSSceneItem item = GetItemAtPos(pos, false);
//	if (!item)
//    {
//        return;
//    }
//
//	bool selected = obs_sceneitem_selected(item);
//	obs_sceneitem_select(item, !selected);
//}

void OBSPreviewMouseHandle::DoSelect(const vec2 &pos)
{
	auto scene = GetScene();
	if (!scene)
    {
        return;
    }

    OBSSceneItem scene_item = GetItemAtPos(pos, false);
    obs_scene_enum_items(scene, select_one, (obs_sceneitem_t *)scene_item);
}

OBSSceneItem OBSPreviewMouseHandle::GetItemAtPos(const vec2 &pos, bool selectBelow) {

	auto scene = GetScene();
	if (!scene)
    {
        return OBSSceneItem();
    }

	SceneFindData data(pos, selectBelow);
	obs_scene_enum_items(scene, FindItemAtPos, &data);
	return data.item;
}

void OBSPreviewMouseHandle::CropItem(const vec2 &pos)
{
	obs_bounds_type boundsType = obs_sceneitem_get_bounds_type(hittest_item_);
	uint32_t stretchFlags = (uint32_t)hittest_handle_;
	uint32_t align = obs_sceneitem_get_alignment(hittest_item_);
	vec3 tl, br, pos3;

	if (boundsType != OBS_BOUNDS_NONE) /* TODO */
		return;

	vec3_zero(&tl);
	vec3_set(&br, stretch_item_size_.x, stretch_item_size_.y, 0.0f);

	vec3_set(&pos3, pos.x, pos.y, 0.0f);
	vec3_transform(&pos3, &pos3, screen_to_item_.get());

	obs_sceneitem_crop crop = start_crop_;
	vec2 scale;

	obs_sceneitem_get_scale(hittest_item_, &scale);

	vec2 max_tl;
	vec2 max_br;

	vec2_set(&max_tl,
		float(-crop.left) * scale.x,
		float(-crop.top) * scale.y);
	vec2_set(&max_br,
		stretch_item_size_.x + crop.right * scale.x,
		stretch_item_size_.y + crop.bottom * scale.y);

	typedef std::function<float (float, float)> minmax_func_t;

	minmax_func_t min_x = scale.x < 0.0f ? maxfunc : minfunc;
	minmax_func_t min_y = scale.y < 0.0f ? maxfunc : minfunc;
	minmax_func_t max_x = scale.x < 0.0f ? minfunc : maxfunc;
	minmax_func_t max_y = scale.y < 0.0f ? minfunc : maxfunc;

	pos3.x = min_x(pos3.x, max_br.x);
	pos3.x = max_x(pos3.x, max_tl.x);
	pos3.y = min_y(pos3.y, max_br.y);
	pos3.y = max_y(pos3.y, max_tl.y);

	if (stretchFlags & ITEM_LEFT) {
		float maxX = stretch_item_size_.x - (2.0 * scale.x);
		pos3.x = tl.x = min_x(pos3.x, maxX);

	} else if (stretchFlags & ITEM_RIGHT) {
		float minX = (2.0 * scale.x);
		pos3.x = br.x = max_x(pos3.x, minX);
	}

	if (stretchFlags & ITEM_TOP) {
		float maxY = stretch_item_size_.y - (2.0 * scale.y);
		pos3.y = tl.y = min_y(pos3.y, maxY);

	} else if (stretchFlags & ITEM_BOTTOM) {
		float minY = (2.0 * scale.y);
		pos3.y = br.y = max_y(pos3.y, minY);
	}

#define ALIGN_X (ITEM_LEFT|ITEM_RIGHT)
#define ALIGN_Y (ITEM_TOP|ITEM_BOTTOM)
	vec3 newPos;
	vec3_zero(&newPos);

	uint32_t align_x = (align & ALIGN_X);
	uint32_t align_y = (align & ALIGN_Y);
	if (align_x == (stretchFlags & ALIGN_X) && align_x != 0)
		newPos.x = pos3.x;
	else if (align & ITEM_RIGHT)
		newPos.x = stretch_item_size_.x;
	else if (!(align & ITEM_LEFT))
		newPos.x = stretch_item_size_.x * 0.5f;

	if (align_y == (stretchFlags & ALIGN_Y) && align_y != 0)
		newPos.y = pos3.y;
	else if (align & ITEM_BOTTOM)
		newPos.y = stretch_item_size_.y;
	else if (!(align & ITEM_TOP))
		newPos.y = stretch_item_size_.y * 0.5f;
#undef ALIGN_X
#undef ALIGN_Y

	crop = start_crop_;

	if (stretchFlags & ITEM_LEFT)
		crop.left += int(std::round(tl.x / scale.x));
	else if (stretchFlags & ITEM_RIGHT)
		crop.right += int(std::round((stretch_item_size_.x - br.x) / scale.x));

	if (stretchFlags & ITEM_TOP)
		crop.top += int(std::round(tl.y / scale.y));
	else if (stretchFlags & ITEM_BOTTOM)
		crop.bottom += int(std::round((stretch_item_size_.y - br.y) / scale.y));

	vec3_transform(&newPos, &newPos, item_to_screen_.get());
	newPos.x = std::round(newPos.x);
	newPos.y = std::round(newPos.y);

#if 0
	vec3 curPos;
	vec3_zero(&curPos);
	obs_sceneitem_get_pos(stretch_item_, (vec2*)&curPos);
	blog(LOG_DEBUG, "curPos {%d, %d} - newPos {%d, %d}",
			int(curPos.x), int(curPos.y),
			int(newPos.x), int(newPos.y));
	blog(LOG_DEBUG, "crop {%d, %d, %d, %d}",
			crop.left, crop.top,
			crop.right, crop.bottom);
#endif

	obs_sceneitem_defer_update_begin(hittest_item_);
	obs_sceneitem_set_crop(hittest_item_, &crop);
	obs_sceneitem_set_pos(hittest_item_, (vec2*)&newPos);
	obs_sceneitem_defer_update_end(hittest_item_);
}

void OBSPreviewMouseHandle::StretchItem(ui::MouseEvent *event, const vec2 &pos)
{
	obs_bounds_type boundsType = obs_sceneitem_get_bounds_type(hittest_item_);
	uint32_t stretchFlags = (uint32_t)hittest_handle_;
	bool shiftDown = always_shift_? true : event->IsShiftDown();
	vec3 tl, br, pos3;

	vec3_zero(&tl);
	vec3_set(&br, stretch_item_size_.x, stretch_item_size_.y, 0.0f);

	vec3_set(&pos3, pos.x, pos.y, 0.0f);
	vec3_transform(&pos3, &pos3, screen_to_item_.get());

	if (stretchFlags & ITEM_LEFT)
		tl.x = pos3.x;
	else if (stretchFlags & ITEM_RIGHT)
		br.x = pos3.x;

	if (stretchFlags & ITEM_TOP)
		tl.y = pos3.y;
	else if (stretchFlags & ITEM_BOTTOM)
		br.y = pos3.y;

	if (!event->IsControlDown())
    {
        SnapItemStretch(tl, br);
    }

	obs_source_t *source = obs_sceneitem_get_source(hittest_item_);

	vec2 baseSize;
	vec2_set(&baseSize,
		float(obs_source_get_width(source)),
		float(obs_source_get_height(source)));

	vec2 size;
	vec2_set(&size,br. x - tl.x, br.y - tl.y);

	if (boundsType != OBS_BOUNDS_NONE) {
		if (shiftDown)
			ClampAspect(tl, br, size, baseSize);

		if (tl.x > br.x) std::swap(tl.x, br.x);
		if (tl.y > br.y) std::swap(tl.y, br.y);

		vec2_abs(&size, &size);

		obs_sceneitem_set_bounds(hittest_item_, &size);
	} else {
		obs_sceneitem_crop crop;
		obs_sceneitem_get_crop(hittest_item_, &crop);

		baseSize.x -= float(crop.left + crop.right);
		baseSize.y -= float(crop.top + crop.bottom);

        if (!shiftDown)
			ClampAspect(tl, br, size, baseSize);

		vec2_div(&size, &size, &baseSize);
		obs_sceneitem_set_scale(hittest_item_, &size);
	}

	pos3 = CalculateStretchPos(tl, br);
	vec3_transform(&pos3, &pos3, item_to_screen_.get());

	vec2 newPos;
	vec2_set(&newPos, std::round(pos3.x), std::round(pos3.y));
	obs_sceneitem_set_pos(hittest_item_, &newPos);
}

vec3 OBSPreviewMouseHandle::CalculateStretchPos(const vec3 &tl, const vec3 &br)
{
    uint32_t alignment = obs_sceneitem_get_alignment(hittest_item_);
    vec3 pos;

    vec3_zero(&pos);

    if (alignment & OBS_ALIGN_LEFT)
        pos.x = tl.x;
    else if (alignment & OBS_ALIGN_RIGHT)
        pos.x = br.x;
    else
        pos.x = (br.x - tl.x) * 0.5f + tl.x;

    if (alignment & OBS_ALIGN_TOP)
        pos.y = tl.y;
    else if (alignment & OBS_ALIGN_BOTTOM)
        pos.y = br.y;
    else
        pos.y = (br.y - tl.y) * 0.5f + tl.y;

    return pos;
}

void OBSPreviewMouseHandle::ClampAspect(vec3 &tl, vec3 &br,
	vec2 &size, const vec2 &baseSize) {

	float    baseAspect = baseSize.x / baseSize.y;
	float    aspect = size.x / size.y;
	uint32_t stretchFlags = (uint32_t)hittest_handle_;

	if (hittest_handle_ == ItemHandle::TopLeft ||
		hittest_handle_ == ItemHandle::TopRight ||
		hittest_handle_ == ItemHandle::BottomLeft ||
		hittest_handle_ == ItemHandle::BottomRight) {
		if (aspect < baseAspect) {
			if ((size.y >= 0.0f && size.x >= 0.0f) ||
				(size.y <= 0.0f && size.x <= 0.0f))
				size.x = size.y * baseAspect;
			else
				size.x = size.y * baseAspect * -1.0f;
		} else {
			if ((size.y >= 0.0f && size.x >= 0.0f) ||
				(size.y <= 0.0f && size.x <= 0.0f))
				size.y = size.x / baseAspect;
			else
				size.y = size.x / baseAspect * -1.0f;
		}

	} else if (hittest_handle_ == ItemHandle::TopCenter ||
		hittest_handle_ == ItemHandle::BottomCenter) {
		if ((size.y >= 0.0f && size.x >= 0.0f) ||
			(size.y <= 0.0f && size.x <= 0.0f))
			size.x = size.y * baseAspect;
		else
			size.x = size.y * baseAspect * -1.0f;

	} else if (hittest_handle_ == ItemHandle::CenterLeft ||
		hittest_handle_ == ItemHandle::CenterRight) {
		if ((size.y >= 0.0f && size.x >= 0.0f) ||
			(size.y <= 0.0f && size.x <= 0.0f))
			size.y = size.x / baseAspect;
		else
			size.y = size.x / baseAspect * -1.0f;
	}

	size.x = std::round(size.x);
	size.y = std::round(size.y);

	if (stretchFlags & ITEM_LEFT)
		tl.x = br.x - size.x;
	else if (stretchFlags & ITEM_RIGHT)
		br.x = tl.x + size.x;

	if (stretchFlags & ITEM_TOP)
		tl.y = br.y - size.y;
	else if (stretchFlags & ITEM_BOTTOM)
		br.y = tl.y + size.y;
}

void OBSPreviewMouseHandle::SnapItemStretch(vec3 &tl, vec3 &br)
{
    uint32_t stretchFlags = (uint32_t)hittest_handle_;
    vec3     newTL = GetTransformedPos(tl.x, tl.y, *(item_to_screen_.get()));
    vec3     newTR = GetTransformedPos(br.x, tl.y, *(item_to_screen_.get()));
    vec3     newBL = GetTransformedPos(tl.x, br.y, *(item_to_screen_.get()));
    vec3     newBR = GetTransformedPos(br.x, br.y, *(item_to_screen_.get()));
    vec3     boundingTL;
    vec3     boundingBR;

    vec3_copy(&boundingTL, &newTL);
    vec3_min(&boundingTL, &boundingTL, &newTR);
    vec3_min(&boundingTL, &boundingTL, &newBL);
    vec3_min(&boundingTL, &boundingTL, &newBR);

    vec3_copy(&boundingBR, &newTL);
    vec3_max(&boundingBR, &boundingBR, &newTR);
    vec3_max(&boundingBR, &boundingBR, &newBL);
    vec3_max(&boundingBR, &boundingBR, &newBR);

    vec3 offset = GetSnapOffset(boundingTL, boundingBR);
    vec3_add(&offset, &offset, &newTL);
    vec3_transform(&offset, &offset, screen_to_item_.get());
    vec3_sub(&offset, &offset, &tl);

    if (stretchFlags & ITEM_LEFT)
        tl.x += offset.x;
    else if (stretchFlags & ITEM_RIGHT)
        br.x += offset.x;

    if (stretchFlags & ITEM_TOP)
        tl.y += offset.y;
    else if (stretchFlags & ITEM_BOTTOM)
        br.y += offset.y;
}

void OBSPreviewMouseHandle::MoveItems(ui::MouseEvent *event, const vec2 &pos)
{
	auto scene = GetScene();
	if (!scene)
    {
        return;
    }

	vec2 offset, move_off_set;
	vec2_sub(&offset, &pos, &start_pos_);
	vec2_sub(&move_off_set, &offset, &last_move_offset_);

	if (!event->IsControlDown())
    {
        SnapItemMovement(move_off_set);
    }

	vec2_add(&last_move_offset_, &last_move_offset_, &move_off_set);

    // 这个是挪全部选中的（虽然当前我们是只允许单选）
	obs_scene_enum_items(scene, move_items, &move_off_set);
}

void OBSPreviewMouseHandle::SnapItemMovement(vec2 &offset)
{
	auto scene = GetScene();
	if (!scene)
    {
        return;
    }

    SelectedItemBounds data;
    obs_scene_enum_items(scene, AddItemBounds, &data);

    data.tl.x += offset.x;
    data.tl.y += offset.y;
    data.br.x += offset.x;
    data.br.y += offset.y;

    vec3 snapOffset = GetSnapOffset(data.tl, data.br);

    const bool snap = obs_proxy::GetPrefs()->GetBoolean(prefs::kSnappingEnabled);
    const bool sourcesSnap = obs_proxy::GetPrefs()->GetBoolean(prefs::kSourceSnapping);
    if (snap == false)
        return;
    if (sourcesSnap == false)
    {
        offset.x += snapOffset.x;
        offset.y += snapOffset.y;
        return;
    }

    const float clampDist = obs_proxy::GetPrefs()->GetDouble(prefs::kSnapDistance) / preview_scale_;

    OffsetData offsetData;
    offsetData.clampDist = clampDist;
    offsetData.tl = data.tl;
    offsetData.br = data.br;
    vec3_copy(&offsetData.offset, &snapOffset);

    obs_scene_enum_items(scene, GetSourceSnapOffset, &offsetData);

    if (fabsf(offsetData.offset.x) > EPSILON ||
        fabsf(offsetData.offset.y) > EPSILON)
    {
        offset.x += offsetData.offset.x;
        offset.y += offsetData.offset.y;
    }
    else
    {
        offset.x += snapOffset.x;
        offset.y += snapOffset.y;
    }
}

vec3 OBSPreviewMouseHandle::GetSnapOffset(const vec3 &tl, const vec3 &br) {

	vec2 screenSize = GetOBSScreenSize();
	vec3 clampOffset;

	vec3_zero(&clampOffset);

	const bool snap = obs_proxy::GetPrefs()->GetBoolean(prefs::kSnappingEnabled);
	if (snap == false)
		return clampOffset;

	const bool screenSnap = obs_proxy::GetPrefs()->GetBoolean(prefs::kScreenSnapping);
	const bool centerSnap = obs_proxy::GetPrefs()->GetBoolean(prefs::kCenterSnapping);
	const float clampDist = obs_proxy::GetPrefs()->GetDouble(prefs::kSnapDistance) / preview_scale_;

	const float centerX = br.x - (br.x - tl.x) / 2.0f;
	const float centerY = br.y - (br.y - tl.y) / 2.0f;

	// Left screen edge.
	if (screenSnap &&
	    fabsf(tl.x) < clampDist)
		clampOffset.x = -tl.x;
	// Right screen edge.
	if (screenSnap &&
	    fabsf(clampOffset.x) < EPSILON &&
	    fabsf(screenSize.x - br.x) < clampDist)
		clampOffset.x = screenSize.x - br.x;
	// Horizontal center.
	if (centerSnap &&
	    fabsf(screenSize.x - (br.x - tl.x)) > clampDist &&
	    fabsf(screenSize.x / 2.0f - centerX) < clampDist)
		clampOffset.x = screenSize.x / 2.0f - centerX;

	// Top screen edge.
	if (screenSnap &&
	    fabsf(tl.y) < clampDist)
		clampOffset.y = -tl.y;
	// Bottom screen edge.
	if (screenSnap &&
	    fabsf(clampOffset.y) < EPSILON &&
	    fabsf(screenSize.y - br.y) < clampDist)
		clampOffset.y = screenSize.y - br.y;
	// Vertical center.
	if (centerSnap &&
	    fabsf(screenSize.y - (br.y - tl.y)) > clampDist &&
	    fabsf(screenSize.y / 2.0f - centerY) < clampDist)
		clampOffset.y = screenSize.y / 2.0f - centerY;

	return clampOffset;
}

obs_scene_t* OBSPreviewMouseHandle::GetScene() const {
	switch (preview_type_) {
	case PreviewType::PREVIEW_MAIN:
		return UIProxyImpl::GetRawCurrentScene();
	default:
		return nullptr;
	}
}

}// obs_proxy_ui
