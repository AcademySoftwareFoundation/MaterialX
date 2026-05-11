void mx_rotate_vector3(vec3 _in, float amount, vec3 axis, out vec3 result)
{
    // Based on https://en.wikipedia.org/wiki/Rodrigues%27_rotation_formula, where the
    // Wikipedia formula follows v' = M * v and MaterialX follows v' = v * M, thus the
    // order of parameters to cross are reversed.

    axis = normalize(axis);
    float rotationRadians = mx_radians(amount);
    float s = mx_sin(rotationRadians);
    float c = mx_cos(rotationRadians);
    float oc = 1.0 - c;
    result = _in * c + cross(_in, axis) * s + axis * dot(axis, _in) * oc;
}
