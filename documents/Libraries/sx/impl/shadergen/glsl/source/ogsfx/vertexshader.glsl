// ---------------------------------- Vertex shader ----------------------------------------

// Data from application to vertex buffer
attribute AppData
{
    vec3 inPosition : POSITION;
    vec3 inNormal   : NORMAL;
    vec3 inTangent  : TANGENT;
    vec3 inBinormal : BINORMAL;
    vec2 inUV       : TEXCOORD0;
};

// Data passed from vertex shader to pixel shader
attribute VertexOutput
{
    vec3 WorldPosition : POSITION;
    vec3 WorldNormal   : NORMAL;
    vec3 WorldTangent  : TANGENT;
    vec3 WorldBinormal : BINORMAL;
    vec2 UV0           : TEXCOORD1;
    vec3 WorldView     : TEXCOORD2;
    float FrontFacing  : FACE;
};

// Vertex shader
GLSLShader VS
{
    void main()
    {
        vec4 Po = vec4(inPosition.xyz,1);
        vec4 Pw = gWorldXf * Po;
        VS_OUT.WorldPosition = Pw.xyz;
        VS_OUT.WorldView     = normalize(gViewIXf[3].xyz - Pw.xyz);
        VS_OUT.WorldNormal   = normalize((gWorldITXf * vec4(inNormal,0)).xyz);
        VS_OUT.WorldTangent  = normalize((gWorldITXf * vec4(inTangent,0)).xyz);
        VS_OUT.WorldBinormal = normalize((gWorldITXf * vec4(inBinormal,0)).xyz);
        VS_OUT.UV0 = inUV;
        VS_OUT.FrontFacing = dot(VS_OUT.WorldNormal, VS_OUT.WorldView);
        if (VS_OUT.FrontFacing < 0.0)
        {
            VS_OUT.WorldNormal = -VS_OUT.WorldNormal;
        }
        vec4 hpos = gWvpXf * Po;
        gl_Position = hpos;
    }
}
