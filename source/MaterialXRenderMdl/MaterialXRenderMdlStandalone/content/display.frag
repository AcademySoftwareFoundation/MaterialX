/******************************************************************************
 * Copyright 2025 NVIDIA Corporation. All rights reserved.
 *****************************************************************************/

// MaterialXRenderMdlStandalone/content/display.frag

#version 450

layout(location = 0) out vec4 FragColor;

layout(rgba32f, set = 0, binding = 0) uniform readonly restrict image2D uBeautyBuffer;

void main()
{
    ivec2 uv = ivec2(gl_FragCoord.xy);

    // Flip image because Vulkan uses the bottom-left corner as the origin,
    // but the rendering code assumed the origin to be the top-left corner.
    uv.y = imageSize(uBeautyBuffer).y - uv.y - 1;

    // Fetch from the accumulation buffer
    vec3 color = imageLoad(uBeautyBuffer, uv).xyz;

    // Apply reinhard tone mapping
    const float burn_out = 0.1;
    color *= (vec3(1.0) + color * burn_out) / (vec3(1.0) + color);

    // Apply gamma correction
    color = pow(clamp(color, 0.0, 1.0), vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
