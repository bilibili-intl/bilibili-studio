#include "bililive/bililive/livehime/toolbar/toolbar_presenter_impl.h"

#include "base/ext/callable_callback.h"

#include "ui/gfx/image/image_skia.h"

#include "bililive/bililive/utils/bililive_image_util.h"
#include "bililive/public/bililive/bililive_process.h"


ToolbarPresenterImpl::ToolbarPresenterImpl(contracts::ToolbarView* view)
    : view_(view),
      weak_ptr_factory_(this){
}

ToolbarPresenterImpl::~ToolbarPresenterImpl()
{
}
