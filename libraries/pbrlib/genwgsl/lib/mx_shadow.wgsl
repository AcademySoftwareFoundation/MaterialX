// Variance shadow mapping utilities.
// https://developer.nvidia.com/gpugems/gpugems3/part-ii-light-and-shadows/chapter-8-summed-area-variance-shadow-maps

fn mx_variance_shadow_occlusion(moments: vec2f, fragmentDepth: f32) -> f32 {
    let MIN_VARIANCE = 0.00001;

    // One-tailed inequality valid if fragmentDepth > moments.x.
    var p: f32;
    if (fragmentDepth <= moments.x) {
        p = 1.0;
    } else {
        p = 0.0;
    }

    // Compute variance.
    let variance = max(moments.y - mx_square_f32(moments.x), MIN_VARIANCE);

    // Compute probabilistic upper bound.
    let d = fragmentDepth - moments.x;
    let pMax = variance / (variance + mx_square_f32(d));
    return max(p, pMax);
}
