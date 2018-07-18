#include "stdlib/sx-glsl/hsv.glsl"

void sx_colorcorrect(vec3 value, float mask, float gamma, float hueShift, float saturation,
                     float contrast, float contrastPivot, float exposure, vec3 multiply, vec3 add,
                     bool invert, bool alphaIsLuminance, float alphaMultiply, float alphaAdd,
                     bool invertAlpha, out vec3 result)
{
    if (mask <= 0.0)
    {
        result = value;
    }
    else
    {
        vec4 color = vec4(value, 1.0);

        if (invert)
        {
            color.rgb = vec3(1.0, 1.0, 1.0) - color.rgb;
        }

        if (invertAlpha)
        {
            color.a = 1.0 - color.a;
        }
        if (alphaIsLuminance)
        {
            const vec3 W = vec3(0.2125, 0.7154, 0.0721);
            color.a = dot(color.rgb, W);
        }

        if (gamma != 1.0)
        {
            float invGamma = 1.0 / gamma;
            color.r = pow(color.r, invGamma);
            color.g = pow(color.g, invGamma);
            color.b = pow(color.b, invGamma);
        }

        if (hueShift != 0.0 || saturation != 1.0)
        {
            vec3 hsv = rgb2hsv(color.rgb);

            // apply hue, keep it in 0..1
            hsv.r += hueShift;
            hsv.r = hsv.r - floor(hsv.r);

            // apply saturation
            hsv.g *= saturation;
            color.rgb = hsv2rgb(hsv);
        }

        if (contrast != 1.0)
        {
            color.rgb = (color.rgb - vec3(contrastPivot)) * contrast + vec3(contrastPivot);
        }

        if (exposure != 0.0)
        {
            color.rgb *= pow(2.0, exposure);
        }

        if (multiply != vec3(1.0, 1.0, 1.0))
        {
            color.rgb *= multiply;
        }

        if (add != vec3(0.0, 0.0, 0.0))
        {
            color.rgb += add;
        }

        color.a = color.a * alphaMultiply + alphaAdd;

        if (mask < 1.0)
        {
            color.rgb = mix(vec3(mask), value, color.rgb);
        }

        result = color.rgb;
    }
}
