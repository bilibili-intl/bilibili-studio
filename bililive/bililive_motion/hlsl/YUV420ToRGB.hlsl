cbuffer TEXTURE_INFO : register(b0)
{
    // x : left, y : top, z : right, w : bottom
    uint4 RGB_TEXT_RECT;
    uint4 ALPHA_TEXT_RECT;
    // x : width, y : height
    uint2 YUV_TEXT_SIZE;
};

Texture2D TEXT_Y : register(t0);
Texture2D TEXT_U : register(t1);
Texture2D TEXT_V : register(t2);

SamplerState BILINEAR_SAMPLE : register(s0);

RWTexture2D<float4> OUT_PUT_RGB : register(u0);


float3 YUVToRGB(float3 yuv)
{
  // BT.601 coefs
    static const float3 yuvCoef_r = { 1.164f, 0.000f, 1.596f };
    static const float3 yuvCoef_g = { 1.164f, -0.392f, -0.813f };
    static const float3 yuvCoef_b = { 1.164f, 2.017f, 0.000f };
    yuv -= float3(0.0625f, 0.5f, 0.5f);
    return saturate(float3(
    dot(yuv, yuvCoef_r),
    dot(yuv, yuvCoef_g),
    dot(yuv, yuvCoef_b)
    ));
}

[numthreads(16, 16, 1)]
void main(uint3 dt_id : SV_DispatchThreadID)
{
    if (dt_id.x >= YUV_TEXT_SIZE.x ||
        dt_id.y >= YUV_TEXT_SIZE.y)
    {
        return;
    }

    float2 uv = float2(dt_id.xy) / float2(YUV_TEXT_SIZE.xy);

    float y = TEXT_Y.SampleLevel(BILINEAR_SAMPLE, uv, 0);
    float u = TEXT_U.SampleLevel(BILINEAR_SAMPLE, uv, 0);
    float v = TEXT_V.SampleLevel(BILINEAR_SAMPLE, uv, 0);

    float3 yuv = float3(y, u, v);

    OUT_PUT_RGB[dt_id.xy] = float4(YUVToRGB(yuv), 1.0);
}


