#include "lib/mx_closure_type.glsl"

void mx_multiply_edf_float(ClosureData closureData, EDF in1, float in2, out EDF result)
{
    result = in1 * in2;
}
