#ifndef BILILIVE_HINT_VIEW_H_
#define BILILIVE_HINT_VIEW_H_

#include <string>
#include "ui/views/view.h"
#include "ui/gfx/canvas.h"
#include "third_party/skia/include/core/SkPaint.h"

namespace bililive
{
	class RoundRectView : public views::View
	{
	public:
		RoundRectView();
		~RoundRectView();

		void SetBorderColor(SkColor borderColor);
		void SetBackgroundColor(SkColor fillColor);
		void SetCornerRadius(int radius);

		SkColor GetBorderColor() const;
		SkColor GetBackgroundColor() const;
		int GetCornerRadius() const;

		gfx::Rect GetMaxRect() const;

		void OnPaint(gfx::Canvas* canvas) override;

	private:
		SkColor borderColor_;
		SkColor fillColor_;
		int radius_;
	};
};

#endif
