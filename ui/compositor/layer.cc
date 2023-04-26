// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "layer.h"

#include <algorithm>

#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "ui/base/animation/animation.h"
#include "layer_animator.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/interpolated_transform.h"
#include "ui/gfx/point3_f.h"
#include <math.h>

namespace {

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const float EPSILON = 1e-3f;

bool IsApproximateMultilpleOf(float value, float base) {
  float remainder = fmod(fabs(value), base);
  return remainder < EPSILON || base - remainder < EPSILON;
}

bool FactorTRS(const gfx::Transform& transform,
               gfx::Point* translation,
               float* rotation,
               gfx::Point3F* scale) 
{
  const SkMatrix44& m = transform.matrix();
  float m00 = m.get(0, 0);
  float m01 = m.get(0, 1);
  float m10 = m.get(1, 0);
  float m11 = m.get(1, 1);

  // A factorable 2D TRS matrix must be of the form:
  //    [ sx*cos_theta -(sy*sin_theta) 0 tx ]
  //    [ sx*sin_theta   sy*cos_theta  0 ty ]
  //    [ 0              0             1 0  ]
  //    [ 0              0             0 1  ]
  if (m.get(0, 2) != 0 ||
    m.get(1, 2) != 0 ||
    m.get(2, 0) != 0 ||
    m.get(2, 1) != 0 ||
    m.get(2, 2) != 1 ||
    m.get(2, 3) != 0 ||
    m.get(3, 0) != 0 ||
    m.get(3, 1) != 0 ||
    m.get(3, 2) != 0 ||
    m.get(3, 3) != 1) {
      return false;
  }

  float scale_x = sqrt(m00 * m00 + m10 * m10);
  float scale_y = sqrt(m01 * m01 + m11 * m11);

  if (scale_x == 0 || scale_y == 0)
    return false;

  float cos_theta = m00 / scale_x;
  float sin_theta = m10 / scale_x;

  if ((fabs(cos_theta - (m11 / scale_y))) > EPSILON ||
    (fabs(sin_theta + (m01 / scale_y))) > EPSILON ||
    (fabs(cos_theta*cos_theta + sin_theta*sin_theta - 1.0f) > EPSILON)) {
      return false;
  }

  float radians = atan2(sin_theta, cos_theta);

  if (translation)
    *translation = gfx::Point(m.get(0, 3), m.get(1, 3));
  if (rotation)
    *rotation = radians * 180 / M_PI;
  if (scale)
    *scale = gfx::Point3F(scale_x, scale_y, 1.0f);

  return true;
}

}  // namespace

namespace ui {

Layer::Layer(Compositor* compositor)
    : type_(LAYER_TEXTURED),
      compositor_(compositor),
      parent_(NULL),
      visible_(true),
      fills_bounds_opaquely_(true),
      layer_updated_externally_(false),
      opacity_(1.0f),
      delegate_(NULL) {
}

Layer::Layer(Compositor* compositor, LayerType type)
    : type_(type),
      compositor_(compositor),
      parent_(NULL),
      visible_(true),
      fills_bounds_opaquely_(true),
      layer_updated_externally_(false),
      opacity_(1.0f),
      delegate_(NULL) {
}

Layer::~Layer() {
  if (parent_)
    parent_->Remove(this);
  for (size_t i = 0; i < children_.size(); ++i)
    children_[i]->parent_ = NULL;
}

Compositor* Layer::GetCompositor() {
  return compositor_ ? compositor_ : parent_ ? parent_->GetCompositor() : NULL;
}

void Layer::SetCompositor(Compositor* compositor) {
  // This function must only be called once, with a valid compositor, and only
  // for the compositor's root layer.
  DCHECK(!compositor_);
  DCHECK(compositor);
  DCHECK_EQ(compositor->root_layer(), this);
  compositor_ = compositor;
}

void Layer::Add(Layer* child) {
  if (child->parent_)
    child->parent_->Remove(child);
  child->parent_ = this;
  children_.push_back(child);

  RecomputeHole();
}

void Layer::Remove(Layer* child) {
  std::vector<Layer*>::iterator i =
      std::find(children_.begin(), children_.end(), child);
  DCHECK(i != children_.end());
  children_.erase(i);
  child->parent_ = NULL;

  RecomputeHole();

  child->DropTextures();
}

void Layer::MoveToFront(Layer* child) {
  std::vector<Layer*>::iterator i =
      std::find(children_.begin(), children_.end(), child);
  DCHECK(i != children_.end());
  children_.erase(i);
  children_.push_back(child);
}

void Layer::StackAtTop(Layer* child) {
  if (children_.size() <= 1 || child == children_.back())
    return;  // Already in front.
  StackAbove(child, children_.back());
}

void Layer::StackAbove(Layer* child, Layer* other) {
  StackRelativeTo(child, other, true);
}

void Layer::StackAtBottom(Layer* child) {
  if (children_.size() <= 1 || child == children_.front())
    return;  // Already on bottom.
  StackBelow(child, children_.front());
}

void Layer::StackBelow(Layer* child, Layer* other) {
  StackRelativeTo(child, other, false);
}

bool Layer::Contains(const Layer* other) const {
  for (const Layer* parent = other; parent; parent = parent->parent()) {
    if (parent == this)
      return true;
  }
  return false;
}

void Layer::SetAnimation(Animation* animation) {
  if (animation) {
    if (!animator_.get())
      animator_.reset(new LayerAnimator(this));
    animation->Start();
    animator_->SetAnimation(animation);
  } else {
    animator_.reset();
  }
}

void Layer::SetTransform(const gfx::Transform& transform) {
  StopAnimatingIfNecessary(LayerAnimator::TRANSFORM);
  if (animator_.get() && animator_->IsRunning()) {
    animator_->AnimateTransform(transform);
    return;
  }
  SetTransformImmediately(transform);
}

void Layer::SetBounds(const gfx::Rect& bounds) {
  StopAnimatingIfNecessary(LayerAnimator::LOCATION);
  if (animator_.get() && animator_->IsRunning() &&
      bounds.size() == bounds_.size()) {
    animator_->AnimateToPoint(bounds.origin());
    return;
  }
  SetBoundsImmediately(bounds);
}

gfx::Rect Layer::GetTargetBounds() const {
  if (animator_.get() && animator_->IsRunning())
    return gfx::Rect(animator_->GetTargetPoint(), bounds_.size());
  return bounds_;
}

void Layer::SetOpacity(float opacity) {
  StopAnimatingIfNecessary(LayerAnimator::OPACITY);
  if (animator_.get() && animator_->IsRunning()) {
    animator_->AnimateOpacity(opacity);
    return;
  }
  SetOpacityImmediately(opacity);
}

void Layer::SetVisible(bool visible) {
  if (visible_ == visible)
    return;

  bool was_drawn = IsDrawn();
  visible_ = visible;
  bool is_drawn = IsDrawn();
  if (was_drawn == is_drawn)
    return;

  if (!is_drawn)
    DropTextures();
  if (parent_)
    parent_->RecomputeHole();
}

bool Layer::IsDrawn() const {
  const Layer* layer = this;
  while (layer && layer->visible_)
    layer = layer->parent_;
  return layer == NULL;
}

bool Layer::ShouldDraw() {
  return type_ == LAYER_TEXTURED && GetCombinedOpacity() > 0.0f &&
      !hole_rect_.Contains(gfx::Rect(gfx::Point(0, 0), bounds_.size()));
}

// static
void Layer::ConvertPointToLayer(const Layer* source,
                                const Layer* target,
                                gfx::Point* point) {
  const Layer* inner = NULL;
  const Layer* outer = NULL;
  if (source->Contains(target)) {
    inner = target;
    outer = source;
    inner->ConvertPointFromAncestor(outer, point);
  } else if (target->Contains(source)) {
    inner = source;
    outer = target;
    inner->ConvertPointForAncestor(outer, point);
  } else {
    NOTREACHED(); // |source| and |target| are in unrelated hierarchies.
  }
}

void Layer::SetFillsBoundsOpaquely(bool fills_bounds_opaquely) {
  if (fills_bounds_opaquely_ == fills_bounds_opaquely)
    return;

  fills_bounds_opaquely_ = fills_bounds_opaquely;

  if (parent())
    parent()->RecomputeHole();
}

void Layer::SetExternalTexture(ui::Texture* texture) {
  DCHECK(texture);
  layer_updated_externally_ = true;
  texture_ = texture;
}

void Layer::SetCanvas(const SkCanvas& canvas, const gfx::Point& origin) {
  DCHECK_EQ(type_, LAYER_TEXTURED);

  if (!texture_.get())
    texture_ = GetCompositor()->CreateTexture();

  texture_->SetCanvas(canvas, origin, bounds_.size());
  invalid_rect_ = gfx::Rect();
}

void Layer::SchedulePaint(const gfx::Rect& invalid_rect) {
  invalid_rect_.Union(invalid_rect);
  ScheduleDraw();
}

void Layer::ScheduleDraw() {
  if (GetCompositor())
    GetCompositor()->ScheduleDraw();
}

void Layer::Draw() {
  DCHECK(GetCompositor());
  if (!ShouldDraw())
    return;

  UpdateLayerCanvas();

  // Layer drew nothing, no texture was created.
  if (!texture_.get())
    return;

  ui::TextureDrawParams texture_draw_params;
  for (Layer* layer = this; layer; layer = layer->parent_) {
    texture_draw_params.transform.ConcatTransform(layer->transform_);
    gfx::Transform translate;
    translate.matrix().setTranslate(
      SkFloatToScalar(layer->bounds_.x()), SkFloatToScalar(layer->bounds_.y()), 0);
    texture_draw_params.transform.ConcatTransform(translate);
  }

  const float combined_opacity = GetCombinedOpacity();

  // Only blend for transparent child layers (and when we're forcing
  // transparency). The root layer will clobber the cleared bg.
  const bool is_root = parent_ == NULL;
  const bool forcing_transparency = combined_opacity < 1.0f;
  const bool is_opaque = fills_bounds_opaquely_ || !has_valid_alpha_channel();
  texture_draw_params.blend = !is_root && (forcing_transparency || !is_opaque);

  texture_draw_params.compositor_size = GetCompositor()->size();
  texture_draw_params.opacity = combined_opacity;
  texture_draw_params.has_valid_alpha_channel = has_valid_alpha_channel();

  std::vector<gfx::Rect> regions_to_draw;
  PunchHole(gfx::Rect(gfx::Point(), bounds().size()), hole_rect_,
            &regions_to_draw);

  for (size_t i = 0; i < regions_to_draw.size(); ++i) {
    if (!regions_to_draw[i].IsEmpty())
      texture_->Draw(texture_draw_params, regions_to_draw[i]);
  }
}

void Layer::DrawTree() {
  if (!visible_)
    return;

  Draw();
  for (size_t i = 0; i < children_.size(); ++i)
    children_.at(i)->DrawTree();
}

void Layer::StackRelativeTo(Layer* child, Layer* other, bool above) {
  DCHECK_NE(child, other);
  DCHECK_EQ(this, child->parent());
  DCHECK_EQ(this, other->parent());

  const size_t child_i =
    std::find(children_.begin(), children_.end(), child) - children_.begin();
  const size_t other_i =
    std::find(children_.begin(), children_.end(), other) - children_.begin();
  if ((above && child_i == other_i + 1) || (!above && child_i + 1 == other_i))
    return;

  const size_t dest_i =
    above ?
    (child_i < other_i ? other_i : other_i + 1) :
    (child_i < other_i ? other_i - 1 : other_i);
  children_.erase(children_.begin() + child_i);
  children_.insert(children_.begin() + dest_i, child);
}

float Layer::GetCombinedOpacity() const {
  float opacity = opacity_;
  Layer* current = this->parent_;
  while (current) {
    opacity *= current->opacity_;
    current = current->parent_;
  }
  return opacity;
}

void Layer::UpdateLayerCanvas() {
  // If we have no delegate, that means that whoever constructed the Layer is
  // setting its canvas directly with SetCanvas().
  if (!delegate_ || layer_updated_externally_)
    return;
  gfx::Rect local_bounds = gfx::Rect(gfx::Point(), bounds_.size());
  gfx::Rect invalid_rect = invalid_rect_;
  invalid_rect.Intersect(local_bounds);
  gfx::Rect draw_rect = texture_.get() ? invalid_rect :
      local_bounds;
  if (draw_rect.IsEmpty()) {
    invalid_rect_ = gfx::Rect();
    return;
  }
  scoped_ptr<gfx::Canvas> canvas(new gfx::Canvas(
    draw_rect.size(), ui::SCALE_FACTOR_100P, false));
  canvas->sk_canvas()->translate(-draw_rect.x(), -draw_rect.y());
  delegate_->OnPaintLayer(canvas.get());
  SetCanvas(*canvas->sk_canvas(), draw_rect.origin());
}

void Layer::RecomputeHole() {
  if (type_ == LAYER_NO_TEXTURE)
    return;

  // Reset to default.
  hole_rect_ = gfx::Rect();

  // Find the largest hole
  for (size_t i = 0; i < children_.size(); ++i) {
    // Ignore non-opaque and hidden children.
    if (!children_[i]->IsCompletelyOpaque() || !children_[i]->visible_)
      continue;

    // Ignore children that aren't rotated by multiples of 90 degrees.
    float degrees;
    if (!FactorTRS(children_[i]->transform(),
                                          NULL, &degrees, NULL) ||
        !IsApproximateMultilpleOf(degrees, 90.0f))
      continue;

    // The reason why we don't just take the bounds and apply the transform is
    // that the bounds encodes a position, too, so the effective transformation
    // matrix is actually different that the one reported. As well, the bounds
    // will not necessarily be at the origin.
    gfx::Rect candidate_hole(children_[i]->bounds_.size());
    gfx::Transform transform = children_[i]->transform();
    gfx::Transform translate;
    translate.matrix().setTranslate(
      SkFloatToScalar(static_cast<float>(children_[i]->bounds_.x())), 
      SkFloatToScalar(static_cast<float>(children_[i]->bounds_.y())), 0);
    transform.ConcatTransform(translate);

    gfx::RectF candidate_hole_f(
      static_cast<float>(candidate_hole.x()),
      static_cast<float>(candidate_hole.y()), 
      static_cast<float>(candidate_hole.width()), 
      static_cast<float>(candidate_hole.height()));
    transform.TransformRect(&candidate_hole_f);
    candidate_hole.SetRect(candidate_hole_f.x(), candidate_hole_f.y(),
      candidate_hole_f.width(), candidate_hole_f.height());

    // This layer might not contain the child (e.g., a portion of the child may
    // be offscreen). Only the portion of the child that overlaps this layer is
    // of any importance, so take the intersection.
    candidate_hole.Intersect(gfx::Rect(bounds().size()));

    // Ensure we have the largest hole.
    if (candidate_hole.size().GetArea() > hole_rect_.size().GetArea())
      hole_rect_ = candidate_hole;
  }

  // Free up texture memory if the hole fills bounds of layer.
  if (!ShouldDraw() && !layer_updated_externally_)
    texture_ = NULL;
}

bool Layer::IsCompletelyOpaque() const {
  return fills_bounds_opaquely() && GetCombinedOpacity() == 1.0f;
}

// static
void Layer::PunchHole(const gfx::Rect& rect,
                      const gfx::Rect& region_to_punch_out,
                      std::vector<gfx::Rect>* sides) {
  gfx::Rect trimmed_rect = rect;
  trimmed_rect.Intersect(region_to_punch_out);

  if (trimmed_rect.IsEmpty()) {
    sides->push_back(rect);
    return;
  }

  // Top (above the hole).
  sides->push_back(gfx::Rect(rect.x(),
                             rect.y(),
                             rect.width(),
                             trimmed_rect.y() - rect.y()));

  // Left (of the hole).
  sides->push_back(gfx::Rect(rect.x(),
                             trimmed_rect.y(),
                             trimmed_rect.x() - rect.x(),
                             trimmed_rect.height()));

  // Right (of the hole).
  sides->push_back(gfx::Rect(trimmed_rect.right(),
                             trimmed_rect.y(),
                             rect.right() - trimmed_rect.right(),
                             trimmed_rect.height()));

  // Bottom (below the hole).
  sides->push_back(gfx::Rect(rect.x(),
                             trimmed_rect.bottom(),
                             rect.width(),
                             rect.bottom() - trimmed_rect.bottom()));
}

void Layer::DropTextures() {
  if (!layer_updated_externally_)
    texture_ = NULL;
  for (size_t i = 0; i < children_.size(); ++i)
    children_[i]->DropTextures();
}

bool Layer::ConvertPointForAncestor(const Layer* ancestor,
                                    gfx::Point* point) const {
  gfx::Transform transform;
  bool result = GetTransformRelativeTo(ancestor, &transform);
  gfx::Point3F p(*point);
  transform.TransformPoint(p);
  point->set_x(p.AsPointF().x());
  point->set_y(p.AsPointF().y());
  return result;
}

bool Layer::ConvertPointFromAncestor(const Layer* ancestor,
                                     gfx::Point* point) const {
  gfx::Transform transform;
  bool result = GetTransformRelativeTo(ancestor, &transform);
  gfx::Point3F p(*point);
  transform.TransformPointReverse(p);
  point->set_x(p.AsPointF().x());
  point->set_y(p.AsPointF().y());
  return result;
}

bool Layer::GetTransformRelativeTo(const Layer* ancestor,
                                   gfx::Transform* transform) const {
  const Layer* p = this;
  for (; p && p != ancestor; p = p->parent()) {
    if (!p->transform().IsIdentity())
      transform->ConcatTransform(p->transform());
    gfx::Transform translate;
    translate.matrix().setTranslate(
      SkFloatToScalar(static_cast<float>(p->bounds().x())), 
      SkFloatToScalar(static_cast<float>(p->bounds().y())), 0);
    transform->ConcatTransform(translate);
  }
  return p == ancestor;
}

void Layer::StopAnimatingIfNecessary(
    LayerAnimator::AnimationProperty property) {
  if (!animator_.get() || !animator_->IsRunning() ||
      !animator_->got_initial_tick()) {
    return;
  }

  if (property != LayerAnimator::LOCATION &&
      animator_->IsAnimating(LayerAnimator::LOCATION)) {
    SetBoundsImmediately(
        gfx::Rect(animator_->GetTargetPoint(), bounds_.size()));
  }
  if (property != LayerAnimator::OPACITY &&
      animator_->IsAnimating(LayerAnimator::OPACITY)) {
    SetOpacityImmediately(animator_->GetTargetOpacity());
  }
  if (property != LayerAnimator::TRANSFORM &&
      animator_->IsAnimating(LayerAnimator::TRANSFORM)) {
    SetTransformImmediately(animator_->GetTargetTransform());
  }
  animator_.reset();
}

void Layer::SetBoundsImmediately(const gfx::Rect& bounds) {
  bounds_ = bounds;

  if (parent())
    parent()->RecomputeHole();
}

void Layer::SetTransformImmediately(const gfx::Transform& transform) {
  transform_ = transform;

  if (parent())
    parent()->RecomputeHole();
}

void Layer::SetOpacityImmediately(float opacity) {
  bool was_opaque = GetCombinedOpacity() == 1.0f;
  opacity_ = opacity;
  bool is_opaque = GetCombinedOpacity() == 1.0f;

  // If our opacity has changed we need to recompute our hole, our parent's hole
  // and the holes of all our descendants.
  if (was_opaque != is_opaque) {
    if (parent_)
      parent_->RecomputeHole();
    std::queue<Layer*> to_process;
    to_process.push(this);
    while (!to_process.empty()) {
      Layer* current = to_process.front();
      to_process.pop();
      current->RecomputeHole();
      for (size_t i = 0; i < current->children_.size(); ++i)
        to_process.push(current->children_.at(i));
    }
  }
}

void Layer::SetBoundsFromAnimator(const gfx::Rect& bounds) {
  SetBoundsImmediately(bounds);
}

void Layer::SetTransformFromAnimator(const gfx::Transform& transform) {
  SetTransformImmediately(transform);
}

void Layer::SetOpacityFromAnimator(float opacity) {
  SetOpacityImmediately(opacity);
}

}  // namespace ui
