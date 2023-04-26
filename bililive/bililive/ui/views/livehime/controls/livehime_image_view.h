#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_CONTROLS_LIVEHIME_IMAGE_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_CONTROLS_LIVEHIME_IMAGE_VIEW_H_

#include "ui/gfx/image/image_skia.h"
#include "ui/views/controls/button/custom_button.h"

#include "bililive/bililive/livehime/event_report/event_report_center.h"
#include "bililive/secret/public/danmaku_hime_service.h"


// ������ʾ��/����ͼƬ�Ŀؼ���֧�ִ� url ��ȡͼƬ��ͬʱ��ָ��ռλͼ��
// ͼƬ���� id �����ص���ʾ��id Խ����ʾԽ��ǰ��
// ����ͼƬ�����ŷ�ʽ����ʾ��ʽ���μ� SetPreferredSize() ������ SetScaleType() ������
class LivehimeImageView :
    public views::CustomButton,
    public livehime::SupportsEventReportV2
{
public:
    enum Alignment {
        LEADING = 0,
        CENTER,
        TRAILING
    };

    enum ScaleType {
        ST_NONE,
        ST_FILL,
        ST_SHOW_ALL
    };

    explicit LivehimeImageView(views::ButtonListener* listener = nullptr);

    // ȡ��һ�Ų�Ϊ�յ�ͼƬ��ͼƬΪ�վ�ȡռλͼ������ͼƬ��Ϊ�յĻ��ͷ��ؿա�
    // �������ȡ����ͼ��ʵ�ʻ��Ƶ�ͼ���Ѿ���ѹ������
    gfx::ImageSkia GetImage();

    // ȡ��Ӧ id ��ԭͼ
    gfx::ImageSkia GetOrgImage(bool is_placeholder = false, int id = -1) const;

    bool IsURLImage(int id = -1);

    bool SetRound(bool isRound) { return  is_round_ = isRound; }

    int  SetRadius(int radius) { return radius_ = radius; }

    // ����ͼƬ�Ķ��������ݡ�
    // ��ͼƬ��������ʾ�����Ǵ����ͼƬΪ�գ���ʱ����ʾ��Ӧ id ��ռλͼ��
    // ��ͬ�� id ���ж�����ã����� SetImageUrl()���Ļ������һ��������Ч��
    void SetImageBinary(const unsigned char* data, unsigned int len, int id = -1);

    // ����ͼƬ��
    // ��ͼƬ��������ʾ�����Ǵ����ͼƬΪ�գ���ʱ����ʾ��Ӧ id ��ռλͼ��
    // ��ͬ�� id ���ж�����ã����� SetImageUrl()���Ļ������һ��������Ч��
    void SetImage(const gfx::ImageSkia& img, int id = -1);

    // ����ͼƬ URL��
    // ��������ʾռλͼ���ȴ�ͼƬ������ɺ���ʾͼƬ��
    // �� URL ���գ���һֱ��ʾ��Ӧ id ��ռλͼ��
    // ��ͬ�� id ���ж�����ã����� SetImage()���Ļ������һ��������Ч��
    void SetImageUrl(const std::string& url, int id = -1);

    // ����ռλͼ��
    void SetImagePlaceholder(const gfx::ImageSkia& img, int id = -1);

    // ����ͼƬ�������ҵĿ�϶��
    // ���ݲ��ֲ�����ͬ���ÿ�϶����������չ��Ҳ���ܼ�ѹͼƬ
    void SetImagePadding(const gfx::Insets& padding, int id = -1);

    // ����ָ��ͼƬ����ʾ��С��
    // ����ʵ����ʾ���򲻹��󣬷���ͼƬ��С��һֱ�� {size}��
    void SetImageBoundSize(bool enabled, const gfx::Size& size, int id = -1);

    void SetImageHoriAlign(Alignment align, int id = -1);

    void SetImageVertAlign(Alignment align, int id = -1);

    // ����ͼƬ��䷽ʽ�������ö�����ͼƬ��Ч��
    // ST_NONE     ����������
    // ST_FILL     ָ��ͼƬ���ؼ������пռ䣬ͼƬ���ܻᱻ�ü���
    // ST_SHOW_ALL ָ��ͼƬ����ʾ��ȫ������¾����ܴ����ʾ��Ĭ��Ϊ�˷�ʽ
    void SetScaleType(ScaleType type);

    // �Ƴ�ͼƬ��
    // �����ռλͼ��ռλͼҲ��һ�����Ƴ���
    // �� id �����ڣ���ʲôҲ������
    void RemoveImage(int id = -1);

    // �Ƴ�����ͼƬ��
    // �����ռλͼ��ռλͼҲ��һ�����Ƴ���
    // �����ǰδ�����κ�ͼƬ����ʲôҲ������
    void RemoveAllImages();

    // ���ָ��ͼƬ�Ļ��棬���������ɻ��棨����ռλͼ����
    // �õ��ûᴥ��ָ��ͼƬ�� OnImageChanging() �ص���
    // �� id �����ڣ���ʲôҲ������
    void RefreshCache(int id = -1);

    // �������ͼƬ�Ļ��棬���������ɻ��档
    // �õ��ûᴥ������ͼƬ�� OnImageChanging() �ص���
    void RefreshAllCaches();

    // ���� View �Ľ����С��
    // δ���ø÷���ʱ��View �Ĵ�С������ͼƬ������ռλͼ����С������
    // �Ƽ����ø÷�������Ϊ����ȥͼƬ�ı�ʱ���ز��֡�
    void SetPreferredSize(const gfx::Size& size);
    gfx::Size preferred_size() const { return preferred_size_; }

    void SetCursor(gfx::NativeCursor cursor) { cursor_ = cursor; }

    // views::View
    gfx::Size GetPreferredSize() override;

protected:
    // views::View
    void OnPaint(gfx::Canvas* canvas) override;
    gfx::NativeCursor GetCursor(const ui::MouseEvent& event) override;
    bool HitTestRect(const gfx::Rect& rect) const override;
    void OnBoundsChanged(const gfx::Rect& previous_bounds) override;

    // Button
    void NotifyClick(const ui::Event& event) override;

    /**
     * ��ͼƬ������֮ǰ���á�
     * ��ʱ������Ҫ�����õ�ͼƬ�������� URL ��ȡ��ͼƬ������һЩ��������Բ����
     * @param img ��ͼƬ��
     * @param id ָ���������ĵ�ͼƬ id��
     * @param is_placeholder ָ�������ĵ�ͼƬ�Ƿ�Ϊռλͼ��
     * @return ���ش����Ľ�����ɡ�Ĭ��ֱ�ӷ��� img��
     */
    virtual gfx::ImageSkia OnImageChanging(
        const gfx::ImageSkia& img, int id, bool is_placeholder);

    /**
     * ��ͼƬ URL ���سɹ�ʱ�����á�
     * @param id ָ���������ͼƬ�� id��
     */
    virtual void OnImageDownloaded(int id) {}

    struct ImageCache {
        gfx::ImageSkia scaled_img; // ���ź��ͼƬ

        ImageCache() {}

        void Set(const gfx::ImageSkia& img) {
            scaled_img = img;
        }

        bool Hit(const gfx::Size& s) const {
            return !scaled_img.isNull() && scaled_img.size() == s;
        }

        void Reset() {
            scaled_img = gfx::ImageSkia();
        }
    };

    struct ImageLayerEntity {
        // id ������ָ�룬����������ָ�룬
        // ����ص�����ʱ���������
        std::shared_ptr<int> id;

        bool using_url = false;
        std::string url;
        gfx::ImageSkia org_ph_img;
        ImageCache ph_img_cache;

        gfx::ImageSkia org_img;
        ImageCache img_cache;
        gfx::Insets padding;

        bool using_size = false;
        gfx::Size size;
        Alignment hori_align = Alignment::CENTER;
        Alignment vert_align = Alignment::CENTER;
    };

    struct DrawImageDetails
    {
        gfx::ImageSkia image;
        gfx::Point point;
    };

    DrawImageDetails GetImageToPaint(ImageLayerEntity& entity);

    std::map<int, ImageLayerEntity> img_layer_ents_;

private:
    ImageLayerEntity* MakeImageLayerEntity(int id);
    gfx::Point GetImagePoint(const gfx::Size& img_size, const gfx::Rect& bounds, Alignment ha, Alignment va) const;
    void ResizeImages();

    // ͼƬ������ɻص���
    // weak_id ָ���Ӧ�� ImageLayerEntity �е� id ����ָ�롣
    void OnGetImageByUrl(
        bool valid_response, const std::string& data,
        const std::weak_ptr<int>& weak_id);

    ScaleType s_type_;
    gfx::NativeCursor cursor_;
    bool is_spec_preferred_size_;
    gfx::Size preferred_size_;
    base::WeakPtrFactory<LivehimeImageView> weakptr_factory_;
    bool  is_round_ = false;
    int    radius_ = 4;

    DISALLOW_COPY_AND_ASSIGN(LivehimeImageView);
};

// ������ʾԲͼƬ�İ汾
class LivehimeCircleImageView : public LivehimeImageView
{
public:
    explicit LivehimeCircleImageView(views::ButtonListener* listener = nullptr);

protected:
    gfx::ImageSkia OnImageChanging(
        const gfx::ImageSkia& img, int id, bool is_placeholder) override;
};

// ������ʾ���󺽺�ͷ����Բͷ��İ汾
class LivehimeGuardPhotoView : public LivehimeImageView {
public:
    explicit LivehimeGuardPhotoView(views::ButtonListener* listener = nullptr);

    void SetGuardFrame(int guard, int size);

protected:
    gfx::ImageSkia OnImageChanging(
        const gfx::ImageSkia& img, int id, bool is_placeholder) override;
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_CONTROLS_LIVEHIME_IMAGE_VIEW_H_