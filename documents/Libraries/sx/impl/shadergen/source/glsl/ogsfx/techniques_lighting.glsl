technique Main
{
    pass p0
    {
        VertexShader(in AppData, out VertexOutput VS_OUT) = { MathFunctions, VS };
        PixelShader(in VertexOutput PS_IN, out PixelOutput) = { MathFunctions, LightingFunctions, PS };
    }
}
