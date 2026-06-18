// SpriteBatch.hlsl
cbuffer PassCB : register(b0)
{
    float4x4 gViewProj;
};

Texture2D gTex[] : register(t0);
SamplerState gSamp : register(s0);

struct VSIn
{
    float2 pos    : POSITION;
    float2 uv     : TEXCOORD;

    float2 iPos   : I_POS;
    float2 iSize  : I_SIZE;
    float4 iUvRect: I_UVRECT;
    float4 iColor : I_COLOR;
    uint   iTex: I_TEX;
	float  iDepth : I_DEPTH;
};

struct VSOut
{
    float4 pos   : SV_POSITION;
    float2 uv    : TEXCOORD;
    float4 color : COLOR;
    uint   tex : TEXID;
};


VSOut VSMain(VSIn vin)
{
    VSOut o;
    float2 world = vin.iPos + vin.pos * vin.iSize;

    o.pos   = mul(float4(world, vin.iDepth, 1.0f), gViewProj);

    float2 uv = lerp(vin.iUvRect.xy, vin.iUvRect.zw, vin.uv);
    o.uv    = uv;

    o.color = vin.iColor;
    o.tex = vin.iTex;
    return o;
}

float4 PSTextMain(VSOut pin) : SV_TARGET
{
    float tex = gTex[pin.tex].Sample(gSamp, pin.uv).r;
    return float4(pin.color.rgb, tex * pin.color.a);
}

float4 PSClipMain(VSOut pin) : SV_TARGET
{
    float4 c = gTex[pin.tex].Sample(gSamp, pin.uv) * pin.color;
	clip(c.a - 0.5f);
	return float4(c.rgb, 1.0f);
}

float4 PSMain(VSOut pin) : SV_TARGET
{
    float4 tex = gTex[pin.tex].Sample(gSamp, pin.uv);
    return tex * pin.color;
}