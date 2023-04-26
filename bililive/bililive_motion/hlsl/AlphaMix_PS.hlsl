cbuffer TEXTURE_INFO : register(b0)
{
    // x : left, y : top, z : right, w : bottom
    uint4 RGB_TEXT_RECT;
    uint4 ALPHA_TEXT_RECT;
    // x : width, y : height
    uint2 YUV_TEXT_SIZE;
    uint2 TEXT_SIZE;
};

Texture2D TEXT_RGB : register(t0);

SamplerState BILINEAR_SAMPLE : register(s0);

float4 main(float4 pos : SV_POSITION) : SV_Target
{
    int2 rgb_size = int2(RGB_TEXT_RECT.z - RGB_TEXT_RECT.x, RGB_TEXT_RECT.w - RGB_TEXT_RECT.y);

    float2 uv = pos.xy / float2(rgb_size.xy);
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

    return float4(rgb.bgr, alpha.r);
}


