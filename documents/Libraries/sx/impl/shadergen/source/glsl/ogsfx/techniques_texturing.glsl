technique Main
{
    pass p0
    {
        VertexShader(in AppData, out VertexOutput VS_OUT) = VS;
        PixelShader(in VertexOutput PS_IN, out PixelOutput) = PS;
    }
}
