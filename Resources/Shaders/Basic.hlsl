cbuffer cbPerObject : register(b0)
{
    float4x4 M;
};

cbuffer cbPerPass : register(b1)
{
    float4x4 VP;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PSInput VSMain(float4 position : POSITION, float4 color : COLOR)
{
    PSInput result;

    float4 worldPos = mul(position, M);
    result.position = mul(worldPos, VP);
    result.color = color;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}