#version 300 es

precision mediump float;

// Uniform block: PrivateUniforms
uniform mat4 u_worldMatrix;
uniform mat4 u_viewProjectionMatrix;
uniform mat4 u_worldInverseTransposeMatrix;

// Inputs block: VertexInputs
in vec3 position;
in vec3 normal;
in vec3 tangent;

out vec3 normalWorld;
out vec3 tangentWorld;
out vec3 positionWorld;

void main()
{
    vec4 hPositionWorld = u_worldMatrix * vec4(position, 1.0);
    gl_Position = u_viewProjectionMatrix * hPositionWorld;
    normalWorld = (u_worldInverseTransposeMatrix * vec4(normal,0.0)).xyz;
    tangentWorld = (u_worldInverseTransposeMatrix * vec4(tangent,0.0)).xyz;
    positionWorld = hPositionWorld.xyz;
}
