void mx_disjointover_color2(vec2 fg, vec2 bg, float mixval, out vec2 result)
{
    float summedAlpha = fg.y + bg.y;

    if (summedAlpha <= 1)
    {
        result.x = fg.x + bg.x;
    }
    else
    {
        if (abs(bg.y) < M_FLOAT_EPS)
        {
            result.x = 0.0;
        }
        else
        {
            result.x = fg.x + ((bg.x * (1 - fg.y)) / bg.y);
        }
    }
    result.y = min(summedAlpha, 1.0);

    result.x = result.x * mixval + (1.0 - mixval) * bg.x;
    result.y = result.y * mixval + (1.0 - mixval) * bg.y;
}
