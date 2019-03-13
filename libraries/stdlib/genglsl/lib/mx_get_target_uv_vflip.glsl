//
// Function to transform uv-coordinates before texture sampling
//
vec2 mx_get_target_uv(vec2 uv)
{
   return vec2(uv.x, 1.0 - uv.y);
}
