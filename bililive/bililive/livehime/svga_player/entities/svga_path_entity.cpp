#include "bililive/bililive/livehime/svga_player/entities/svga_path_entity.h"

#include <set>

#include "ui/gfx/point.h"

#include "base/strings/string_util.h"
#include "base/strings/string_number_conversions.h"


namespace svga {

    namespace {

        const std::set<std::string> VALID_METHODS {
            "M", "L", "H", "V", "C", "S", "Q", "R", "A", "Z",
            "m", "l", "h", "v", "c", "s", "q", "r", "a", "z"
        };

    }

    SVGAPathEntity::SVGAPathEntity(const std::string& origin_val) {
        replaced_val_ = origin_val;
        ReplaceSubstringsAfterOffset(&replaced_val_, 0, ",", " ");
    }

    void SVGAPathEntity::buildPath(gfx::Path* to_path) {
        if (!cached_path_.isEmpty()) {
            to_path->reset();
            to_path->addPath(cached_path_);
            return;
        }

        gfx::Path cached_path;
        auto segments = base::StringTokenizer(replaced_val_, "MLHVCSQRAZmlhvcsqraz");
        std::string cur_method;

        while (segments.GetNext()) {
            auto segment = segments.token();
            if (segment.empty()) {
                continue;
            }

            if (VALID_METHODS.find(segment) != VALID_METHODS.end()) {
                cur_method = segment;
                if (cur_method == "Z" || cur_method == "z") {
                    auto st = base::StringTokenizer("", "");
                    operate(cached_path, cur_method, st);
                }
            } else {
                auto st = base::StringTokenizer(segment, " ");
                operate(cached_path, cur_method, st);
            }
        }

        cached_path_.addPath(cached_path);
        to_path->reset();
        to_path->addPath(cached_path);
    }

    void SVGAPathEntity::operate(
        gfx::Path& final_path, const std::string& method, base::StringTokenizer& args)
    {
        float x0 = 0;
        float y0 = 0;
        float x1 = 0;
        float y1 = 0;
        float x2 = 0;
        float y2 = 0;

        int index = 0;
        while (args.GetNext()) {
            auto s = args.token();
            if (s.empty()) {
                continue;
            }

            if (index == 0) { StringToFloat(s, &x0); }
            if (index == 1) { StringToFloat(s, &y0); }
            if (index == 2) { StringToFloat(s, &x1); }
            if (index == 3) { StringToFloat(s, &y1); }
            if (index == 4) { StringToFloat(s, &x2); }
            if (index == 5) { StringToFloat(s, &y2); }
            ++index;
        }

        gfx::Point cur_point(0, 0);
        if (method == "M") {
            final_path.moveTo(x0, y0);
            cur_point.SetPoint(x0, y0);
        } else if (method == "m") {
            final_path.rMoveTo(x0, y0);
            cur_point.SetPoint(cur_point.x() + x0, cur_point.y() + y0);
        }

        if (method == "L") {
            final_path.lineTo(x0, y0);
        } else if (method == "l") {
            final_path.rLineTo(x0, y0);
        }

        if (method == "C") {
            final_path.cubicTo(x0, y0, x1, y1, x2, y2);
        } else if (method == "c") {
            final_path.rCubicTo(x0, y0, x1, y1, x2, y2);
        }

        if (method == "Q") {
            final_path.quadTo(x0, y0, x1, y1);
        } else if (method == "q") {
            final_path.rQuadTo(x0, y0, x1, y1);
        }

        if (method == "H") {
            final_path.lineTo(x0, cur_point.y());
        } else if (method == "h") {
            final_path.rLineTo(x0, 0.f);
        }

        if (method == "V") {
            final_path.lineTo(cur_point.x(), x0);
        } else if (method == "v") {
            final_path.rLineTo(0.f, x0);
        }

        if (method == "Z") {
            final_path.close();
        } else if (method == "z") {
            final_path.close();
        }
    }

    bool SVGAPathEntity::StringToFloat(const std::string& str, float* val) {
        float result;
        std::istringstream ss(str);
        if (!(ss >> result)) {
            return false;
        }
        *val = result;
        return true;
    }

}