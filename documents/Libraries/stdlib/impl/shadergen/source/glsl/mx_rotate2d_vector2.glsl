void mx_rotate2d_vector2(vec2 _in, float amount, vec2 center, out vec2 result)
{
    float rotationRadians = radians(amount);
    float sa = sin(rotationRadians);
    float ca = cos(rotationRadians);
    vec2 v = _in + center;
    result = vec2(ca*v.x + sa*v.y, -sa*v.x + ca*v.y) - center;
}
