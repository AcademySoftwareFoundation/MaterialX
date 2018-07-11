void sx_range(vec3 value, float inMin, float inMax, float outMin, float outMax, out vec3 result)
{
    result = ((value - vec3(inMin)) / (vec3(inMax) - vec3(inMin))) * (vec3(outMax) - vec3(outMin)) + outMin;
}
