void mx_srgb_texture_to_linear_color3(vec3 _in, out vec3 result)
{
    vec4 outColor = vec4(_in, 0.);
    vec4 breakPnt = vec4(0.03928571566939354, 0.03928571566939354, 0.03928571566939354, 1.);
    vec4 slope = vec4(0.07738015800714493, 0.07738015800714493, 0.07738015800714493, 1.);
    vec4 scale = vec4(0.9478672742843628, 0.9478672742843628, 0.9478672742843628, 1.);
    vec4 offset = vec4(0.05213269963860512, 0.05213269963860512, 0.05213269963860512, 0.);
    vec4 gamma = vec4(2.4, 2.4, 2.4, 1.0);
    vec4 isAboveBreak = vec4(greaterThan(outColor, breakPnt));
    vec4 linSeg = outColor * slope;
    vec4 powSeg = pow( max( vec4(0., 0., 0., 0.), scale * outColor + offset), gamma);
    result = (isAboveBreak * powSeg + ( vec4(1., 1., 1., 1.) - isAboveBreak ) * linSeg).rgb;
}
