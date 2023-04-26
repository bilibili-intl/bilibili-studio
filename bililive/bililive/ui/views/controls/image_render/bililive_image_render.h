#ifndef BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_IMAGE_RENDER_BILILIVE_IMAGE_RENDER_H
#define BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_IMAGE_RENDER_BILILIVE_IMAGE_RENDER_H

#include "base/timer/timer.h"
#include "base/threading/thread.h"
#include "base/files/file_path.h"

#include "third_party/libnsgif/libnsgif.h"

#include "bililive/bililive/utils/image_util.h"

#include "ui/gfx/canvas.h"
#include "ui/gfx/image/image.h"

namespace bililive
{
    // 根据容器的尺寸以及源尺寸得出放到容器中的等比例伸缩尺寸
    gfx::Size CalculateEqualProportionSize(const gfx::Size &container, const gfx::Size &source);
}

namespace
{
    class ImageRenderTimerThread;
}

class BililiveImageRenderDelegate
{
public:
    virtual void OnImageFrameChanged(const gfx::ImageSkia* image) = 0;
};

class BililiveImageRender
    : public base::RefCountedThreadSafe<BililiveImageRender>
{
public:
    explicit BililiveImageRender(BililiveImageRenderDelegate* delegate);

    bililive::ImageType InitFromBinary(const unsigned char * const data, unsigned int len, bool attach_gif_data = false);
    bililive::ImageType InitFromFile(const base::FilePath &file_path);
    void SetEnableGif(bool enable);
    // 上层必须显式调用CleanUp()以便让TimerThread释放引用
    void CleanUp();

    //gfx::Image image(){ return image_; }
    gfx::Size image_size() const { return image_size_; }
    bililive::ImageType image_type() const { return image_type_; }
    const unsigned char * data() const { return data_; }
    unsigned int data_len() const { return data_len_; }

private:
    virtual ~BililiveImageRender();

    // work thread
    void ChangeGifFrame();
    gfx::ImageSkia GetCurrentFrame(void *data, size_t count);
    void SetImage(void *data, size_t count);

    void Start();
    void Stop();
    void CleanResource();

private:
    friend class base::RefCountedThreadSafe<BililiveImageRender>;
    friend class ImageRenderTimerThread;

    BililiveImageRenderDelegate* delegate_;
    bililive::ImageType image_type_;
    gfx::Size image_size_;
    gfx::ImageSkia image_;

    bool enable_gif_;
    unsigned char *data_;
    unsigned int data_len_;
    gif_animation gif_;
    int64 gif_frame_delta_;

    DISALLOW_COPY_AND_ASSIGN(BililiveImageRender);
};

typedef scoped_refptr<BililiveImageRender> RefImageRender;

#endif