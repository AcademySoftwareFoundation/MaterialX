// Open Shading Language : Copyright (c) 2009-2017 Sony Pictures Imageworks Inc., et al.
// https://github.com/imageworks/OpenShadingLanguage/blob/master/LICENSE
//
// MaterialX specification (c) 2017 Lucasfilm Ltd.
// http://www.materialx.org/

#pragma once

#include "color4.h"
#include "vector2.h"
#include "vector4.h"
#include "matrix33.h"

//
// Support functions for OSL implementations of the MaterialX nodes.
//

float mx_ternary(int expr, float v1, float v2) { if (expr) return v1; else return v2; }
int mx_ternary(int expr, int v1, int v2) { if (expr) return v1; else return v2; }
color mx_ternary(int expr, color v1, color v2) { if (expr) return v1; else return v2; }
color4 mx_ternary(int expr, color4 v1, color4 v2) { if (expr) return v1; else return v2; }
vector mx_ternary(int expr, vector v1, vector v2) { if (expr) return v1; else return v2; }
vector2 mx_ternary(int expr, vector2 v1, vector2 v2) { if (expr) return v1; else return v2; }
vector4 mx_ternary(int expr, vector4 v1, vector4 v2) { if (expr) return v1; else return v2; }
matrix mx_ternary(int expr, matrix v1, matrix v2) { if (expr) return v1; else return v2; }
matrix33 mx_ternary(int expr, matrix33 v1, matrix33 v2) { if (expr) return v1; else return v2; }


matrix33 mx_add(matrix33 a, matrix33 b)
{
    return matrix33(matrix(
        a.m[0][0]+b.m[0][0], a.m[0][1]+b.m[0][1], a.m[0][2]+b.m[0][2], 0.0,
        a.m[1][0]+b.m[1][0], a.m[1][1]+b.m[1][1], a.m[1][2]+b.m[1][2], 0.0,
        a.m[2][0]+b.m[2][0], a.m[2][1]+b.m[2][1], a.m[2][2]+b.m[2][2], 0.0,
        0.0, 0.0, 0.0, 1.0));
}

matrix33 mx_add(matrix33 a, float b)
{
    return matrix33(matrix(
        a.m[0][0]+b, a.m[0][1]+b, a.m[0][2]+b, 0.0,
        a.m[1][0]+b, a.m[1][1]+b, a.m[1][2]+b, 0.0,
        a.m[2][0]+b, a.m[2][1]+b, a.m[2][2]+b, 0.0,
        0.0, 0.0, 0.0, 1.0));
}

matrix mx_add(matrix a, matrix b)
{
    return matrix(
        a[0][0]+b[0][0], a[0][1]+b[0][1], a[0][2]+b[0][2], a[0][3]+b[0][3],
        a[1][0]+b[1][0], a[1][1]+b[1][1], a[1][2]+b[1][2], a[1][3]+b[1][3],
        a[2][0]+b[2][0], a[2][1]+b[2][1], a[2][2]+b[2][2], a[2][3]+b[2][3],
        a[3][0]+b[3][0], a[3][1]+b[3][1], a[3][2]+b[3][2], a[3][3]+b[3][3]);
}

matrix mx_add(matrix a, float b)
{
    return matrix(
        a[0][0]+b, a[0][1]+b, a[0][2]+b, a[0][3]+b,
        a[1][0]+b, a[1][1]+b, a[1][2]+b, a[1][3]+b,
        a[2][0]+b, a[2][1]+b, a[2][2]+b, a[2][3]+b,
        a[3][0]+b, a[3][1]+b, a[3][2]+b, a[3][3]+b);
}


matrix33 mx_subtract(matrix33 a, matrix33 b)
{
    return matrix33(matrix(
        a.m[0][0]-b.m[0][0], a.m[0][1]-b.m[0][1], a.m[0][2]-b.m[0][2], 0.0,
        a.m[1][0]-b.m[1][0], a.m[1][1]-b.m[1][1], a.m[1][2]-b.m[1][2], 0.0,
        a.m[2][0]-b.m[2][0], a.m[2][1]-b.m[2][1], a.m[2][2]-b.m[2][2], 0.0,
        0.0, 0.0, 0.0, 1.0));
}

matrix33 mx_subtract(matrix33 a, float b)
{
    return matrix33(matrix(
        a.m[0][0]-b, a.m[0][1]-b, a.m[0][2]-b, 0.0,
        a.m[1][0]-b, a.m[1][1]-b, a.m[1][2]-b, 0.0,
        a.m[2][0]-b, a.m[2][1]-b, a.m[2][2]-b, 0.0,
        0.0, 0.0, 0.0, 1.0));
}

matrix mx_subtract(matrix a, matrix b)
{
   return matrix(
       a[0][0]-b[0][0], a[0][1]-b[0][1], a[0][2]-b[0][2], a[0][3]-b[0][3],
       a[1][0]-b[1][0], a[1][1]-b[1][1], a[1][2]-b[1][2], a[1][3]-b[1][3],
       a[2][0]-b[2][0], a[2][1]-b[2][1], a[2][2]-b[2][2], a[2][3]-b[2][3],
       a[3][0]-b[3][0], a[3][1]-b[3][1], a[3][2]-b[3][2], a[3][3]-b[3][3]);
}

matrix mx_subtract(matrix a, float b)
{
    return matrix(
        a[0][0]-b, a[0][1]-b, a[0][2]-b, a[0][3]-b,
        a[1][0]-b, a[1][1]-b, a[1][2]-b, a[1][3]-b,
        a[2][0]-b, a[2][1]-b, a[2][2]-b, a[2][3]-b,
        a[3][0]-b, a[3][1]-b, a[3][2]-b, a[3][3]-b);
}


float mx_extract(color in, int index)
{
    return in[index];
}

float mx_extract(color4 in, int index)
{
    if (index == 0) return in.rgb.r;
    else if (index == 1) return in.rgb.g;
    else if (index == 2) return in.rgb.b;
    else return in.a;
}

float mx_extract(vector2 in, int index)
{
    if (index == 0) return in.x;
    else return in.y;
}

float mx_extract(vector in, int index)
{
    return in[index];
}

float mx_extract(vector4 in, int index)
{
    if (index == 0) return in.x;
    else if (index == 1) return in.y;
    else if (index == 2) return in.z;
    else return in.w;
}


float mx_remap(float in, float inLow, float inHigh, float outLow, float outHigh, int doClamp)
{
      float x = (in - inLow)/(inHigh-inLow);
      if (doClamp == 1) {
           x = clamp(x, 0, 1);
      }
      return outLow + (outHigh - outLow) * x;
}

color mx_remap(color in, color inLow, color inHigh, color outLow, color outHigh, int doClamp)
{
      color x = (in - inLow) / (inHigh - inLow);
      if (doClamp == 1) {
           x = clamp(x, 0, 1);
      }
      return outLow + (outHigh - outLow) * x;
}

color mx_remap(color in, float inLow, float inHigh, float outLow, float outHigh, int doClamp)
{
      color x = (in - inLow) / (inHigh - inLow);
      if (doClamp == 1) {
           x = clamp(x, 0, 1);
      }
      return outLow + (outHigh - outLow) * x;
}

color4 mx_remap(color4 c, color4 inLow, color4 inHigh, color4 outLow, color4 outHigh, int doClamp)
{
      return color4(mx_remap(c.rgb, inLow.rgb, inHigh.rgb, outLow.rgb, outHigh.rgb, doClamp),
                    mx_remap(c.a, inLow.a, inHigh.a, outLow.a, outHigh.a, doClamp));
}

color4 mx_remap(color4 c, float inLow, float inHigh, float outLow, float outHigh, int doClamp)
{
    color4 c4_inLow = color4(color(inLow), inLow);
    color4 c4_inHigh = color4(color(inHigh), inHigh);
    color4 c4_outLow = color4(color(outLow), outLow);
    color4 c4_outHigh = color4(color(outHigh), outHigh);
    return mx_remap(c, c4_inLow, c4_inHigh, c4_outLow, c4_outHigh, doClamp);
}

vector2 mx_remap(vector2 in, vector2 inLow, vector2 inHigh, vector2 outLow, vector2 outHigh, int doClamp)
{
    return vector2(mx_remap(in.x, inLow.x, inHigh.x, outLow.x, outHigh.x, doClamp),
                   mx_remap(in.y, inLow.y, inHigh.y, outLow.y, outHigh.y, doClamp));
}

vector2 mx_remap(vector2 in, float inLow, float inHigh, float outLow, float outHigh, int doClamp)
{
    return vector2(mx_remap(in.x, inLow, inHigh, outLow, outHigh, doClamp),
                   mx_remap(in.y, inLow, inHigh, outLow, outHigh, doClamp));
}

vector4 mx_remap(vector4 in, vector4 inLow, vector4 inHigh, vector4 outLow, vector4 outHigh, int doClamp)
{
    return vector4(mx_remap(in.x, inLow.x, inHigh.x, outLow.x, outHigh.x, doClamp),
                   mx_remap(in.y, inLow.y, inHigh.y, outLow.y, outHigh.y, doClamp),
                   mx_remap(in.z, inLow.z, inHigh.z, outLow.z, outHigh.z, doClamp),
                   mx_remap(in.w, inLow.w, inHigh.w, outLow.w, outHigh.w, doClamp));
}

vector4 mx_remap(vector4 in, float inLow, float inHigh, float outLow, float outHigh, int doClamp)
{
    return vector4(mx_remap(in.x, inLow, inHigh, outLow, outHigh, doClamp),
                   mx_remap(in.y, inLow, inHigh, outLow, outHigh, doClamp),
                   mx_remap(in.z, inLow, inHigh, outLow, outHigh, doClamp),
                   mx_remap(in.w, inLow, inHigh, outLow, outHigh, doClamp));
}


float mx_contrast(float in, float amount, float pivot)
{
    float out = in - pivot;
    out *= amount;
    out += pivot;
    return out;
}

color mx_contrast(color in, color amount, color pivot)
{
    color out = in - pivot;
    out *= amount;
    out += pivot;
    return out;
}

color mx_contrast(color in, float amount, float pivot)
{
    color out = in - pivot;
    out *= amount;
    out += pivot;
    return out;
}

color4 mx_contrast(color4 c, color4 amount, color4 pivot)
{
    return color4(mx_contrast(c.rgb, amount.rgb, pivot.rgb),
                  mx_contrast(c.a, amount.a, pivot.a));
}

color4 mx_contrast(color4 c, float amount, float pivot)
{
    return mx_contrast(c, color4(color(amount), amount), color4(color(pivot), pivot));
}

vector2 mx_contrast(vector2 in, vector2 amount, vector2 pivot)
{
    return vector2 (mx_contrast(in.x, amount.x, pivot.x),
                    mx_contrast(in.y, amount.y, pivot.y));
}

vector2 mx_contrast(vector2 in, float amount, float pivot)
{
    return mx_contrast(in, vector2(amount, amount), vector2(pivot, pivot));
}

vector4 mx_contrast(vector4 in, vector4 amount, vector4 pivot)
{
    return vector4(mx_contrast(in.x, amount.x, pivot.x),
                   mx_contrast(in.y, amount.y, pivot.y),
                   mx_contrast(in.z, amount.z, pivot.z),
                   mx_contrast(in.w, amount.w, pivot.w));
}

vector4 mx_contrast(vector4 in, float amount, float pivot)
{
    return vector4(mx_contrast(in.x, amount, pivot),
                   mx_contrast(in.y, amount, pivot),
                   mx_contrast(in.z, amount, pivot),
                   mx_contrast(in.w, amount, pivot));
}


vector2 mx_noise(string noisetype, float x, float y)
{
    color cnoise = (color) noise(noisetype, x, y);
    return vector2 (cnoise[0], cnoise[1]);
}

color4 mx_noise(string noisetype, float x, float y)
{
    color cnoise = (color) noise(noisetype, x, y);
    float fnoise = (float) noise(noisetype, x + 19, y + 73);
    return color4 (cnoise, fnoise);
}

vector4 mx_noise(string noisetype, float x, float y)
{
    color cnoise = (color) noise(noisetype, x, y);
    float fnoise = (float) noise(noisetype, x + 19, y + 73);
    return vector4 (cnoise[0], cnoise[1], cnoise[2], fnoise);
}

vector2 mx_noise(string noisetype, point position)
{
    color cnoise = (color) noise(noisetype, position);
    return vector2 (cnoise[0], cnoise[1]);
}

color4 mx_noise(string noisetype, point position)
{
    color cnoise = (color) noise(noisetype, position);
    float fnoise = (float) noise(noisetype, position+vector(19,73,29));
    return color4 (cnoise, fnoise);
}

vector4 mx_noise(string noisetype, point position)
{
    color cnoise = (color) noise(noisetype, position);
    float fnoise = (float) noise(noisetype, position+vector(19,73,29));
    return vector4 (cnoise[0], cnoise[1], cnoise[2], fnoise);
}


float mx_fbm(point position, int octaves, float lacunarity, float diminish, string noisetype)
{
    float out = 0;
    float amp = 1.0;
    point p = position;

    for (int i = 0;  i < octaves;  i += 1) {
        out += amp * noise(noisetype, p);
        amp *= diminish;
        p *= lacunarity;
    }
    return out;
}

color mx_fbm(point position, int octaves, float lacunarity, float diminish, string noisetype)
{
    color out = 0;
    float amp = 1.0;
    point p = position;

    for (int i = 0;  i < octaves;  i += 1) {
        out += amp * (color)noise(noisetype, p);
        amp *= diminish;
        p *= lacunarity;
    }
    return out;
}

vector2 mx_fbm(point position, int octaves, float lacunarity, float diminish, string noisetype)
{
    return vector2((float) mx_fbm(position, octaves, lacunarity, diminish, noisetype),
                   (float) mx_fbm(position+point(19, 193, 17), octaves, lacunarity, diminish, noisetype));
}

color4 mx_fbm(point position, int octaves, float lacunarity, float diminish, string noisetype)
{
    color c = (color) mx_fbm(position, octaves, lacunarity, diminish, noisetype);
    float f = (float) mx_fbm(position+point(19, 193, 17), octaves, lacunarity, diminish, noisetype);
    return color4 (c, f);
}

vector4 mx_fbm(point position, int octaves, float lacunarity, float diminish, string noisetype)
{
    color c = (color) mx_fbm(position, octaves, lacunarity, diminish, noisetype);
    float f = (float) mx_fbm(position+point(19, 193, 17), octaves, lacunarity, diminish, noisetype);
    return vector4 (c[0], c[1], c[2], f);
}

vector2 mx_worley_cell_position(int x, int y, int xoff, int yoff, float jitter)
{
    vector  tmp = cellnoise(x+xoff, y+yoff);
    vector2 off = vector2(tmp.x, tmp.y);
    off -= 0.5;
    off *= jitter;
    off += 0.5;
    return vector2(x, y) + off;
}

vector mx_worley_cell_position(int x, int y, int z, int xoff, int yoff, int zoff, float jitter)
{
    vector off = cellnoise(vector(x+xoff, y+yoff, z+zoff));
    off -= 0.5;
    off *= jitter;
    off += 0.5;
    return vector(x,y,z) + off;
}

float mx_worley_distance(vector2 p, int x, int y, int X, int Y, float jitter, int metric)
{
    vector2 cellpos = mx_worley_cell_position(x,y,X,Y,jitter);
    vector2 diff = cellpos - p;

    if (metric == 2)
        return abs(diff.x) + abs(diff.y);     // Manhattan distance
    if (metric == 3)
        return max(abs(diff.x), abs(diff.y)); // Chebyshev distance
    return diff.x*diff.x + diff.y*diff.y;       // Euclidean or distance^2
}

float mx_worley_distance(vector p, int x, int y, int z, int X, int Y, int Z, float jitter, int metric)
{
    vector cellpos = mx_worley_cell_position(x,y,z,X,Y,Z,jitter);
    vector diff = cellpos - p;
    if (metric == 2)
        return abs(diff[0]) + abs(diff[1]);     // Manhattan distance
    if (metric == 3)
        return max(abs(diff[0]), abs(diff[1])); // Chebyshev distance
    return dot(diff, diff);                     // Eucldean or distance^2
}

void mx_sort_distance(float dist, output vector2 result)
{
    if (dist < result.x)
    {
        result.y = result.x;
        result.x = dist;
    }
    else if (dist < result.y)
    {
        result.y = dist;
    }
}

void mx_sort_distance(float dist, output vector result)
{
    if (dist < result[0])
    {
        result[2] = result[1];
        result[1] = result[0];
        result[0] = dist;
    }
    else if (dist < result[1])
    {
        result[2] = result[1];
        result[1] = dist;
    }
    else if (dist < result[2])
    {
        result[2] = dist;
    }
}

// return floor as well as the fractional remainder
float mx_floorfrac(float x, output int i)
{
    i = (int)floor(x);
    return x - float(i);
}

float mx_worley_noise_float(vector2 p, float jitter, int style, int metric)
{
    int X, Y;
    float sqdist = 1e6;
    vector2 localpos = vector2(mx_floorfrac(p.x, X), mx_floorfrac(p.y, Y));
    vector2 minpos = vector2(0.0, 0.0);

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float dist = mx_worley_distance(localpos, x, y, X, Y, jitter, metric);
            vector2 cellpos = mx_worley_cell_position(x, y, X, Y, jitter) - localpos;
            if (dist < sqdist)
            {
                sqdist = dist;
                minpos = cellpos;
            }
        }
    }
    if (style == 1)
    {
        vector2 tmpP = minpos + p;
        return cellnoise(tmpP.x, tmpP.y);
    }
    else
    {
        if (metric == 0)
            sqdist = sqrt(sqdist);
        return sqdist;
    }
}

vector2 mx_worley_noise_vector2(vector2 p, float jitter, int style, int metric)
{
    int X, Y;
    vector2 sqdist = vector2(1e6, 1e6);
    vector2 localpos = vector2(mx_floorfrac(p.x, X), mx_floorfrac(p.y, Y));
    vector2 minpos = vector2(0.0, 0.0);

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float dist = mx_worley_distance(localpos, x, y, X, Y, jitter, metric);
            vector2 cellpos = mx_worley_cell_position(x, y, X, Y, jitter) - localpos;
            if (dist < sqdist.x)
            {
                sqdist.y = sqdist.x;
                sqdist.x = dist;
                minpos = cellpos;
            }
            else if (dist < sqdist.y)
            {
                sqdist.y = dist;
            }
        }
    }

    if (style == 1)
    {
        vector2 tmpP = minpos + p;
        vector tmp = cellnoise(tmpP.x, tmpP.y);
        return vector2(tmp.x, tmp.y);
    }
    else
    {
        if (metric == 0)
            sqdist = sqrt(sqdist);
        return sqdist;
    }
}

vector mx_worley_noise_vector3(vector2 p, float jitter, int style, int metric)
{
    int X, Y;
    vector sqdist = vector(1e6, 1e6, 1e6);
    vector2 localpos = vector2(mx_floorfrac(p.x, X), mx_floorfrac(p.y, Y));
    vector2 minpos = vector2(0.0, 0.0);

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float dist = mx_worley_distance(localpos, x, y, X, Y, jitter, metric);
            vector2 cellpos = mx_worley_cell_position(x, y, X, Y, jitter) - localpos;
            if (dist < sqdist.x)
            {
                sqdist.z = sqdist.y;
                sqdist.y = sqdist.x;
                sqdist.x = dist;
                minpos = cellpos;
            }
            else if (dist < sqdist.y)
            {
                sqdist.z = sqdist.y;
                sqdist.y = dist;
            }
            else if (dist < sqdist.z)
            {
                sqdist.z = dist;
            }
        }
    }
    if (style == 1)
    {
        vector2 tmpP = minpos + p;
        return cellnoise(tmpP.x, tmpP.y);
    }
    else
    {
        if (metric == 0)
            sqdist = sqrt(sqdist);
        return sqdist;
    }
}

float mx_worley_noise_float(vector p, float jitter, int style, int metric)
{
    int X, Y, Z;
    vector seed = p;
    float sqdist = 1e6;
    vector localpos = vector(mx_floorfrac(p.x, X), mx_floorfrac(p.y, Y), mx_floorfrac(p.z, Z));
    vector minpos = vector(0.0, 0.0, 0.0);
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            for (int z = -1; z <= 1; ++z)
            {
                float dist = mx_worley_distance(localpos, x, y, z, X, Y, Z, jitter, metric);
                vector cellpos = mx_worley_cell_position(x, y, z, X, Y, Z, jitter) - localpos;
                if(dist < sqdist)
                {
                    sqdist = dist;
                    minpos = cellpos;
                }
            }
        }
    }
    if (style == 1)
        return cellnoise(minpos + p);
    else
    {
        if (metric == 0)
            sqdist = sqrt(sqdist);
        return sqdist;
    }
}

vector2 mx_worley_noise_vector2(vector p, float jitter, int style, int metric)
{
    int X, Y, Z;
    vector2 sqdist = vector2(1e6, 1e6);
    vector localpos = vector(mx_floorfrac(p.x, X), mx_floorfrac(p.y, Y), mx_floorfrac(p.z, Z));
    vector minpos = vector(0.0, 0.0, 0.0);

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            for (int z = -1; z <= 1; ++z)
            {
                float dist = mx_worley_distance(localpos, x, y, z, X, Y, Z, jitter, metric);
                vector cellpos = mx_worley_cell_position(x, y, z, X, Y, Z, jitter) - localpos;
                if (dist < sqdist.x)
                {
                    sqdist.y = sqdist.x;
                    sqdist.x = dist;
                    minpos = cellpos;
                }
                else if (dist < sqdist.y)
                {
                    sqdist.y = dist;
                }
            }
        }
    }
    if (style == 1)
    {
        vector tmp = cellnoise(minpos + p);
        return vector2(tmp.x,tmp.y);
    }
    else
    {
        if (metric == 0)
            sqdist = sqrt(sqdist);
        return sqdist;
    }
}

vector mx_worley_noise_vector3(vector p, float jitter, int style, int metric)
{
    int X, Y, Z;
    vector sqdist = 1e6;
    vector localpos = vector(mx_floorfrac(p.x, X), mx_floorfrac(p.y, Y), mx_floorfrac(p.z, Z));
    vector minpos = vector(0.0, 0.0, 0.0);

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            for (int z = -1; z <= 1; ++z)
            {
                float dist = mx_worley_distance(localpos, x, y, z, X, Y, Z, jitter, metric);
                vector cellpos = mx_worley_cell_position(x, y, z, X, Y, Z, jitter) - localpos;
                if (dist < sqdist.x)
                {
                    sqdist.z = sqdist.y;
                    sqdist.y = sqdist.x;
                    sqdist.x = dist;
                    minpos = cellpos;
                }
                else if (dist < sqdist.y)
                {
                    sqdist.z = sqdist.y;
                    sqdist.y = dist;
                }
                else if (dist < sqdist.z)
                {
                    sqdist.z = dist;
                }
            }
        }
    }
    if (style == 1)
        return cellnoise(minpos + p);
    else
    {
        if (metric == 0)
            sqdist = sqrt(sqdist);
        return sqdist;
    }
}
