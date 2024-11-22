/******************************************************************************
 * Copyright 2025 NVIDIA Corporation. All rights reserved.
 *****************************************************************************/

// MaterialXRenderMdlStandalone/content/display.vert

#version 450

void main()
{ 
    vec2 uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(uv * 2.0 - 1.0, 0.0, 1.0);
}
