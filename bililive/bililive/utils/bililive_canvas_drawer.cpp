#include "bililive_canvas_drawer.h"

#include <atomic>

#include "ui/gfx/canvas.h"
#include "ui/gfx/skia_util.h"

#include "SkBitmapScaler.h"

namespace bililive
{
    typedef std::vector<std::vector<double> > RoundCornerPointsCache;
    const RoundCornerPointsCache* GetOrCreateRoundCornerCache(int radius);
    double QueryCornerValueAt(const RoundCornerPointsCache *arc, int x, int y, bool left, bool top);

    static const double PI = 3.1415926;

    static double GetAlpha(double distanceX, double distanceY)
    {
        DCHECK(distanceX >= 0);
        DCHECK(distanceY >= 0);
        double r = 1 - sqrt(distanceX * distanceX + distanceY * distanceY);
        if (r > 0)
            return r;
        else
            return 0;
    }

//����֮ǰ�ļ����������ٲ�ѯ��
//���ܻ��в���Ҫ�Ķ����ڴ�ռ��
//TODO: �Ƿ������ڴ�й©Ҫȷ��

    struct RoundCornerPointsCacheNode
    {
        int radius;
        RoundCornerPointsCache val;
        RoundCornerPointsCacheNode* next;
    };

    static std::atomic<RoundCornerPointsCacheNode*> roundCornerCacheHead = nullptr;


    static void DrawRoundRectCorner(const RoundCornerPointsCache* cache, gfx::Canvas* canvas,
        SkPaint& paint, int centerX, int centerY, int radius, bool isXPositive, bool isYPositive)
    {
        // +1 ����Ϊ�ܹ�ȡ��radius�Ǹ��±�
        // +2 ��Ϊ��ͼ���㡭������ +1 ��ʱ�����ж�
        DCHECK((*cache).size() == size_t(radius + 2));
        for (auto& x : *cache)
            DCHECK(x.size() == size_t(radius + 2));

        SkColor color = paint.getColor();

        int sx = isXPositive ? 1 : -1;
        int sy = isYPositive ? 1 : -1;

        for (int i = 0; i <= radius; ++i)
            for (int j = 0; j <= radius; ++j)
            {
                if ((*cache)[i][j] > 0)
                {
                    paint.setColor(SkColorSetARGB((*cache)[i][j] * SkColorGetA(color), SkColorGetR(color), SkColorGetG(color), SkColorGetB(color)));
                    canvas->DrawPoint(gfx::Point(centerX + i * sx, centerY + j * sy), paint);
                }
            }

        paint.setColor(color);
    }


    //��Ȼ����֧�ֶ��̵߳�д����
    //����ʵ���ϲ�û�в��Թ��ǲ�����Ŀ�������
    //�����Ҫ����߳�ͬʱ���ʣ�����ȼ��һ��
    const RoundCornerPointsCache* GetOrCreateRoundCornerCache(int radius)
    {
        RoundCornerPointsCacheNode* cur = roundCornerCacheHead;
        while (cur)
        {
            if (cur->radius == radius)
                return &cur->val;
            cur = cur->next;
        }

        //�����ڣ�����
        RoundCornerPointsCache tmp(radius + 2);

        for (auto& x : tmp)
            x.resize(radius + 2);

        //����Բ�ǵĻ���
        double fullLength = 2 * PI * radius / 4;

        //ÿ1���ػ�����Ӧ��Բ�Ľ�
        double cornerStep = PI / 2 / fullLength;

        for (double i = 0; i <= PI / 2; i += cornerStep)
        {
            double x = std::sin(i) * radius;
            double y = std::cos(i) * radius;
            int ix = (int)x;
            int iy = (int)y;
            tmp[ix][iy] += GetAlpha(x - ix, y - iy);
            tmp[ix + 1][iy] += GetAlpha(ix + 1 - x, y - iy);
            tmp[ix + 1][iy + 1] += GetAlpha(ix + 1 - x, iy + 1 - y);
            tmp[ix][iy + 1] += GetAlpha(x - ix, iy + 1 - y);
        }

        for (auto& x : tmp)
            for (auto& y : x)
                if (y > 1)
                    y = 1;

        RoundCornerPointsCacheNode* newNode = new RoundCornerPointsCacheNode();
        newNode->radius = radius;
        newNode->val = std::move(tmp);

        RoundCornerPointsCacheNode* headNode = roundCornerCacheHead;
        newNode->next = headNode;
        while (std::atomic_compare_exchange_strong(&roundCornerCacheHead, &headNode, newNode) == false)
        {
            headNode = roundCornerCacheHead;
            newNode->next = headNode;
        }

        return &newNode->val;
    }

    double QueryCornerValueAt(const RoundCornerPointsCache *arc, int x, int y, bool left, bool top)
    {
        assert(x < (int)arc->size());
        assert(y < (int)(*arc)[0].size());

        int scale = arc->size() - 1;
        int dst_x;
        int dst_y;

        if (left)
            dst_x = scale - 1 - x;
        else
            dst_x = x;

        if (top)
            dst_y = scale - 1 - y;
        else
            dst_y = y;


        ////if (left)
        ////    dst_x = scale - 1 - x;
        ////else
        //    dst_x = x;

        ////if (top)
        ////    dst_y = scale - 1 - y;
        ////else
        //    dst_y = y;

        return (arc->operator[](dst_x))[dst_y];
    }

    gfx::Rect GetRoundRectInnerRect(int w, int h, int radius)
    {
        const double sin45 = 0.70710678118654752440084436210485;

        int diff = int(radius - radius * sin45 + 0.5);
        int availWidth = w - diff * 2;
        int availHeight = h - diff * 2;

        //����һ�����أ�������ݻ��Ƶ�ʱ����
        availWidth = std::max(availWidth, 2);
        availHeight = std::max(availHeight, 2);
        return gfx::Rect(diff + 1, diff + 1, availWidth - 2, availHeight - 2);
    }

    void DrawRoundRect(gfx::Canvas* canvas, int x, int y, int w, int h, int radius, SkPaint& paint)
    {
        bool useSkiaAlgorithm = true;

        SkMatrix transformMatrix = canvas->sk_canvas()->getTotalMatrix();
        SkScalar strokeWidth = paint.getStrokeWidth();
        SkScalar scaledStrokeWidth = transformMatrix.mapRadius(paint.getStrokeWidth());

        int scaledR, scaledX, scaledY, scaledW, scaledH;

        //�߿����ź�С�ڵ���1����ֻ�а������������ŵ����
        //��֧���б�
        if (transformMatrix.mapRadius(strokeWidth) <= 1.0f)
        {
            if (transformMatrix.getSkewX() == 0.0f && transformMatrix.getSkewY() == 0.0f)
            {
                SkRect rect;
                rect.setLTRB(x, y, x + w, y + h);
                transformMatrix.mapRect(&rect);
                SkScalar fScaledR = transformMatrix.mapRadius(radius);
                SkScalar fScaledX = rect.x();
                SkScalar fScaledY = rect.y();
                SkScalar fScaledW = rect.width();
                SkScalar fScaledH = rect.height();

                auto IsInt = [](SkScalar val)->bool {
                    return std::abs(val - static_cast<int>(val)) < std::numeric_limits<SkScalar>::epsilon();
                };

                if (IsInt(fScaledR) && IsInt(fScaledX) && IsInt(fScaledY) && IsInt(fScaledW) && IsInt(fScaledH))
                {
                    scaledR = static_cast<int>(fScaledR);
                    scaledX = static_cast<int>(fScaledX);
                    scaledY = static_cast<int>(fScaledY);
                    scaledW = static_cast<int>(fScaledW);
                    scaledH = static_cast<int>(fScaledH);
                    useSkiaAlgorithm = false;
                }
            }
        }

        if (useSkiaAlgorithm)
        {
            SkPath path;
            SkRect rect;

            //����������ھ����ڵ�
            rect.setLTRB(x + strokeWidth / 2.0f, y + strokeWidth / 2.0f, x + w - strokeWidth / 2.0f, y + h - strokeWidth / 2.0f);

            //����Skia����㷨
            path.addRoundRect(rect, radius, radius);
            SkPaint localPaint(paint);
            localPaint.setAntiAlias(true);
            localPaint.setStrokeWidth(strokeWidth);
            localPaint.setStyle(SkPaint::kStroke_Style);
            canvas->DrawPath(path, localPaint);
        }
        else
        {
            //ʹ�������㷨��ֹԲ�ǲ��ֹ����黯

            //Ϊ��DPI����ӳ������ֵ
            {
                radius = scaledR;
                x = scaledX;
                y = scaledY;
                w = scaledW;
                h = scaledH;

                //�ͽ����Ĳ���w��h�ǲ�����
                //Ҫת���ɺ��ġ���
                h -= 1;
                w -= 1;
            }

            //ȡ���任����
            canvas->Save();
            canvas->sk_canvas()->setMatrix(SkMatrix::I());

            SkPaint localPaint(paint);

            //ʹ�����ƿ�����㷨
            localPaint.setAntiAlias(false);

            canvas->DrawLine(gfx::Point(x + radius + 1, y), gfx::Point(x + w - radius, y), localPaint); //��
            canvas->DrawLine(gfx::Point(x, y + radius/* + 1*/), gfx::Point(x, y + h - radius + 1), localPaint); //��
            canvas->DrawLine(gfx::Point(x + radius + 1, y + h), gfx::Point(x + w - radius, y + h), localPaint); //��
            canvas->DrawLine(gfx::Point(x + w, y + radius/* + 1*/), gfx::Point(x + w, y + h - radius + 1), localPaint); //��

            const RoundCornerPointsCache* cache = GetOrCreateRoundCornerCache(radius);

            DrawRoundRectCorner(cache, canvas, localPaint, x + radius, y + radius, radius, false, false); //����
            DrawRoundRectCorner(cache, canvas, localPaint, x + w - radius, y + radius, radius, true, false); //����
            DrawRoundRectCorner(cache, canvas, localPaint, x + radius, y + h - radius, radius, false, true); //����
            DrawRoundRectCorner(cache, canvas, localPaint, x + w - radius, y + h - radius, radius, true, true); //����

            canvas->Restore();
        }
    }

    void FillRoundRect(gfx::Canvas* canvas, int x, int y, int w, int h, int radius, SkPaint& paint)
    {
        static const int line_width = 1;
        static const int line_half_width = (line_width + 1) / 2; // ceil(line_width / 2.0)

        gfx::Rect rect(x, y, w, h);
        //���Ƶ�ɫ
        //ע�⣺������ϸ�����1���أ���2��dpi��Ҳ��1���ز���2���ؿ�
        SkRect skRect;
        skRect.fTop = rect.y();
        skRect.fLeft = rect.x();
        skRect.fBottom = rect.bottom();
        skRect.fRight = rect.right();
        const SkMatrix& canvasMatrix = canvas->sk_canvas()->getTotalMatrix();
        skRect.inset(line_half_width / canvasMatrix.getScaleX(), line_half_width / canvasMatrix.getScaleY());
        canvas->sk_canvas()->drawRoundRect(skRect, radius - line_half_width, radius - line_half_width, paint);
    }

    void DrawCircleRect(gfx::Canvas* canvas, int x, int y, int cx, int cy, SkPaint& paint,
                        bool is_horiz/* = true*/, SkPath* ret_path/* = nullptr*/)
    {
        // Ϊ�õ��Ϻõĳ���Ч������������Ӧ�ñ�֤����Բͷ���εĳ��ȴ��ڵ���Բͷֱ����
        // ������Ƶ��߼�Ϊ��ֻ������ࣨˮƽ��/�ϲ�����ֱ���İ�Բ���ټ���ʣ��ռ�ľ��Σ�
        // ������Ϊ��һ���ƺ����������Ϊ�˻���Բͷ��������д�ģ������ڳ��Ȳ����Ի�������������Բͷʱ
        // ֻ��Ҫ��֤���ȿ�ʼ�����һ����Բͷ���ɣ�

        SkPath path;
        if (is_horiz)
        {
            int radius = std::ceil(cy*1.0f / 2);
            if (radius * 2 > cy)
            {
                radius = std::floor(cy*1.0f / 2);
            }

            if (cx >= radius * 2)
            {
                // ���ȹ�����Բ+һ��
                path.addCircle(x + radius, y + radius, radius);
                path.addRect(SkRect::MakeXYWH(x + radius, y, cx - radius * 2, radius * 2));
                path.addCircle(x + cx - radius, y + radius, radius);
            }
            else if (cx > 0)
            {
                // ���Ȳ�����һԲ+һ��
                if (cx >= radius)
                {
                    path.addCircle(radius, radius, radius);
                    path.addRect(SkRect::MakeXYWH(radius, 0, radius, radius * 2));
                }
                else
                {
                    path.addCircle(radius, radius, radius);
                }
            }
        }
        else
        {
            int radius = std::ceil(cx*1.0f / 2);
            if (radius * 2 > cx)
            {
                radius = std::floor(cx*1.0f / 2);
            }

            if (cy >= radius * 2)
            {
                // ���ȹ�����Բ+һ��
                path.addCircle(x + radius, y + radius, radius);
                path.addRect(SkRect::MakeXYWH(x, y + radius, radius * 2, cy - radius * 2));
                path.addCircle(x + radius, y + cy - radius, radius);
            }
            else if (cy > 0)
            {
                // ���Ȳ�����һԲ+һ��
                if (cy >= radius)
                {
                    path.addCircle(radius, radius, radius);
                    path.addRect(SkRect::MakeXYWH(0, radius, radius * 2, radius));
                }
                else
                {
                    path.addCircle(radius, radius, radius);
                }
            }
        }

        canvas->Save();
        canvas->ClipRect(gfx::Rect(x, y, cx, cy));
        canvas->DrawPath(path, paint);
        canvas->Restore();

        // ������·���ش������ڵ�������·���ϼ�������
        if (ret_path)
        {
            *ret_path = path;
        }
    }

    gfx::ImageSkia CreateScaleBitmap(const gfx::ImageSkia& sourceImage, const gfx::Size& targetSize, bool fill/* = false*/)
    {
        return CreateScaleBitmap(*sourceImage.bitmap(), targetSize, fill);
    }

    gfx::ImageSkia CreateScaleBitmap(const SkBitmap& sourceBitmap, const gfx::Size& targetSize, bool fill/* = false*/)
    {
        float scale = sourceBitmap.width() * 1.0f / sourceBitmap.height();
        int dstCX = std::min(sourceBitmap.width(), targetSize.width());
        int dstCY = dstCX / scale;
        if (!fill)
        {
            // �Գ���Ϊ����
            if (dstCY > targetSize.height())
            {
                dstCY = targetSize.height();
                dstCX = dstCY * scale;
            }
        }
        else
        {
            // �Զ̱�Ϊ����
            if (dstCY < targetSize.height())
            {
                dstCY = targetSize.height();
                dstCX = dstCY * scale;
            }
        }

        SkBitmap result;
        SkConvolutionProcs convolutionProcs{};
        SkBitmapScaler::Resize(&result, sourceBitmap,
            SkBitmapScaler::ResizeMethod::RESIZE_LANCZOS3,
            dstCX, dstCY,
            &convolutionProcs);

        return gfx::ImageSkia::CreateFrom1xBitmap(result);
    }
};
