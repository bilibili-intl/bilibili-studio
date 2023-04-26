#pragma once

#include <algorithm>

#include "obs/obs_proxy/live_mask/iterator_util.hpp"
#include "obs/obs_proxy/live_mask/lib/live-mask-svgbin.hpp"
// ..\..\third_party\potrace\dist\$(Platform)\$(Configuration)\include
#include "potracelib.h"

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/numeric/ublas/matrix.hpp>

namespace potrace_svgbin
{
    using namespace svgbin;

    using matrix4 = boost::numeric::ublas::c_matrix<double, 4, 4>;
    using ident_matrix4 = boost::numeric::ublas::identity_matrix<double>;
    using vector3 = boost::numeric::ublas::c_vector<double, 3>;
    using vector4 = boost::numeric::ublas::c_vector<double, 4>;
    using zero_vector4 = boost::numeric::ublas::zero_vector<double>;
    using boost::numeric::ublas::prod;
    using boost::numeric::ublas::inner_prod;
    using boost::numeric::ublas::outer_prod;

    vector4 make_vector3(double x, double y, double z)
    {
        vector3 r;
        r(0) = x; r(1) = y; r(2) = z;
        return r;
    }

    vector4 make_vector4(double x, double y, double z, double w)
    {
        vector4 r;
        r(0) = x; r(1) = y; r(2) = z; r(3) = w;
        return r;
    }

    vector3 cross(vector3 a, vector3 b)
    {
        return make_vector3(
            a(1)*b(2) - a(2)*b(1),
            a(2)*b(0) - a(0)*b(2),
            a(0)*b(1) - a(1)*b(0)
        );
    }

    struct encode_ctx
    {
        encode_ctx()
            : rel_orig{ 0.0, 0.0 }
            , transform(ident_matrix4(4))
        {
        }

        potrace_dpoint_t rel_orig;
        matrix4 transform;
    };

    void transform_pt(int cnt, potrace_dpoint_t output[], potrace_dpoint_t input[], const matrix4& transform)
    {
        for (int i = 0; i < cnt; ++i)
        {
            auto vec = make_vector4(input[i].x, input[i].y, 0, 1.0);
            vec = prod(vec, transform);
            output[i].x = vec(0);
            output[i].y = vec(1);
        }
    }

    template<class It, class T>
    void encode(encode_ctx& ctx, It& it, T x)
    {
        uint8_t buf[VARINT64_MAX_BYTES];
        uint8_t* p = write_proc(x, buf, sizeof(buf));
        std::copy(buf, p, it);
        try_advance(it, p - buf);
    }

    template<class It>
    void encode_moveto(encode_ctx& ctx, It& it, potrace_dpoint_t pt)
    {
        potrace_dpoint_t tmp;
        transform_pt(1, &tmp, &pt, ctx.transform);
        encode(ctx, it, (int64_t)SVG_KEY_PATH_MOVETO);
        encode(ctx, it, (int64_t)tmp.x); encode(ctx, it, (int64_t)tmp.y);
        ctx.rel_orig = tmp;
    }

    template<class It>
    void encode_lineto(encode_ctx& ctx, It& it, potrace_dpoint_t pt)
    {
        potrace_dpoint_t tmp;
        transform_pt(1, &tmp, &pt, ctx.transform);
        encode(ctx, it, (int64_t)SVG_KEY_PATH_LINETO);
        encode(ctx, it, (int64_t)tmp.x); encode(ctx, it, (int64_t)tmp.y);
        ctx.rel_orig = tmp;
    }

    template<class It>
    void encode_curveto(encode_ctx& ctx, It& it, potrace_dpoint_t pt[3])
    {
        potrace_dpoint_t tmp[3];
        transform_pt(3, tmp, pt, ctx.transform);
        encode(ctx, it, (int64_t)SVG_KEY_PATH_CURVETO);
        encode(ctx, it, (int64_t)tmp[0].x); encode(ctx, it, (int64_t)tmp[0].y);
        encode(ctx, it, (int64_t)tmp[1].x); encode(ctx, it, (int64_t)tmp[1].y);
        encode(ctx, it, (int64_t)tmp[2].x); encode(ctx, it, (int64_t)tmp[2].y);
        ctx.rel_orig = tmp[2];
    }

    template<class It>
    void encode_rmoveto(encode_ctx& ctx, It& it, potrace_dpoint_t pt)
    {
        potrace_dpoint_t tmp;
        transform_pt(1, &tmp, &pt, ctx.transform);
        auto& orig = ctx.rel_orig;
        encode(ctx, it, (int64_t)SVG_KEY_PATH_RMOVETO);
        encode(ctx, it, (int64_t)(tmp.x - orig.x)); encode(ctx, it, (int64_t)(tmp.y - orig.y));
        orig = tmp;
    }

    template<class It>
    void encode_rlineto(encode_ctx& ctx, It& it, potrace_dpoint_t pt)
    {
        potrace_dpoint_t tmp;
        transform_pt(1, &tmp, &pt, ctx.transform);
        auto& orig = ctx.rel_orig;
        encode(ctx, it, (int64_t)SVG_KEY_PATH_RLINETO);
        encode(ctx, it, (int64_t)(tmp.x - orig.x)); encode(ctx, it, (int64_t)(tmp.y - orig.y));
        orig = tmp;
    }

    template<class It>
    void encode_rcurveto(encode_ctx& ctx, It& it, potrace_dpoint_t pt[3])
    {
        potrace_dpoint_t tmp[3];
        transform_pt(3, tmp, pt, ctx.transform);
        auto& orig = ctx.rel_orig;
        encode(ctx, it, (int64_t)SVG_KEY_PATH_RCURVETO);
        encode(ctx, it, (int64_t)(tmp[0].x - orig.x)); encode(ctx, it, (int64_t)(tmp[0].y - orig.y));
        encode(ctx, it, (int64_t)(tmp[1].x - orig.x)); encode(ctx, it, (int64_t)(tmp[1].y - orig.y));
        encode(ctx, it, (int64_t)(tmp[2].x - orig.x)); encode(ctx, it, (int64_t)(tmp[2].y - orig.y));
        orig = tmp[2];
    }

    template<class It>
    void encode_close(encode_ctx& ctx, It it)
    {
        encode(ctx, it, (int64_t)SVG_KEY_PATH_CLOSE);
    }


    template<class It>
    void encode_path(encode_ctx& ctx, potrace_path_t* path_it, It& it)
    {
        { // move to last point
            encode_moveto(ctx, it, path_it->curve.c[path_it->curve.n - 1][2]);
        }
        for (int i = 0; i < path_it->curve.n; ++i)
        {
            if (path_it->curve.tag[i] == POTRACE_CORNER)
            {
                encode_lineto(ctx, it, path_it->curve.c[i][1]);
                encode_lineto(ctx, it, path_it->curve.c[i][2]);
            }
            else if (path_it->curve.tag[i] == POTRACE_CURVETO)
            {
                encode_curveto(ctx, it, path_it->curve.c[i]);
            }
        }

        encode_close(ctx, it);

        // child
        potrace_path_t* child = path_it->childlist;
        while (child != nullptr)
        {
            encode_path(ctx, child, it);
            child = child->sibling;
        }
    }

    template<class It>
    bool encode_header(encode_ctx& ctx, It& it, float width, float height,
        float trans_x = 0.0f, float trans_y = 0.0f,
        float scale_x = 1.0f, float scale_y = 1.0f)
    {
        // version
        encode(ctx, it, (int64_t)SVG_KEY_VERSION);
        encode(ctx, it, (int64_t)SVG_BIN_VERSION);

        // meta
        encode(ctx, it, (int64_t)SVG_KEY_META);
        // width, height
        encode(ctx, it, width); encode(ctx, it, height);
        // trans_x, trans_y
        encode(ctx, it, trans_x); encode(ctx, it, trans_y);
        // scale_x, scale_y
        encode(ctx, it, scale_x); encode(ctx, it, scale_y);

        return true;
    }

    template<class It>
    bool encode_potracestate(encode_ctx& ctx, potrace_state_t* state, It& it)
    {
        auto path_it = state->plist;
        while(path_it)
        {
            // encode path
            potrace_dpoint_t cur = path_it->curve.c[path_it->curve.n - 1][2];
            encode_path(ctx, path_it, it);
            
            path_it = path_it->sibling;
        }

        return true;
    }
};
