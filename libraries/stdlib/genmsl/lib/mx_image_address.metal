bool mx_image_coord_out_of_bounds(vec2 uv, int uaddressmode, int vaddressmode)
{
    // MaterialX address mode enum: constant=0, clamp=1, periodic=2, mirror=3.
    return (uaddressmode == 0 && (uv.x < 0.0 || uv.x > 1.0)) ||
           (vaddressmode == 0 && (uv.y < 0.0 || uv.y > 1.0));
}
