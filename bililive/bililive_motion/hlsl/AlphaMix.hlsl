cbuffer TEXTURE_INFO : register(b0)
{
    // x : left, y : top, z : right, w : bottom
    uint4 RGB_TEXT_RECT;
    uint4 ALPHA_TEXT_RECT;
    // x : width, y : height
    uint2 YUV_TEXT_SIZE;
};

Texture2D TEXT_RGB : register(t0);

SamplerState BILINEAR_SAMPLE : register(s0);

RWTexture2D<float4> OUT_PUT_RGBA : register(u0);

[numthreads(16, 16, 1)]
void main(uint3 dt_id : SV_DispatchThreadID)
{
    int2 rgb_size = int2(RGB_TEXT_RECT.z - RGB_TEXT_RECT.x, RGB_TEXT_RECT.w - RGB_TEXT_RECT.y);

    if (dt_id.x >= rgb_size.x ||
        dt_id.y >= rgb_size.y)
    {
        return;
    }

    float2 uv = float2(dt_id.xy) / float2(rgb_size.xy);
    int2 alpha_size = int2(ALPHA_TEXT_RECT.z - ALPHA_TEXT_RECT.x, ALPHA_TEXT_RECT.w - ALPHA_TEXT_RECT.y);

    float2 rgb_uv_bias = float2(RGB_TEXT_RECT.xy) / float2(YUV_TEXT_SIZE.xy);
    float2 rgb_uv = (float2(rgb_size) / float2(YUV_TEXT_SIZE.xy)) * uv + rgb_uv_bias;

    float2 alpha_uv_bias = float2(ALPHA_TEXT_RECT.xy) / float2(YUV_TEXT_SIZE.xy);
    float2 alpha_uv = (float2(alpha_size) / float2(YUV_TEXT_SIZE.xy)) * uv + alpha_uv_bias;

    float3 rgb = TEXT_RGB.SampleLevel(BILINEAR_SAMPLE, rgb_uv, 0).rgb;
    float3 alpha = TEXT_RGB.SampleLevel(BILINEAR_SAMPLE, alpha_uv, 0).rrr;

    //skia format alpha 透明需要预乘，
    //参考 ui\gfx\codec\png_codec.cc ConvertRGBARowToSkia函数
    if (alpha.r != 1.0)
    {
        rgb = clamp(rgb / alpha, 0.0, 1.0);
        rgb = rgb * alpha;
    }

    OUT_PUT_RGBA[dt_id.xy] = float4(rgb.bgr, alpha.r);
}


