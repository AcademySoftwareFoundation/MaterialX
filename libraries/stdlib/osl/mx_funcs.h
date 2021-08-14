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


///////////////////////////////////////////////////////////////////////////
// This file contains lots of functions helpful in the implementation of
// the MaterialX nodes.
///////////////////////////////////////////////////////////////////////////


// Define mx_convert_type
//   float -> colvecN
color mx_convert (float a) { return color(a); }
color4 mx_convert (float a) { return color4(a,a); }
vector mx_convert (float a) { return vector(a); }
vector2 mx_convert (float a) { return vector2(a,a); }
vector4 mx_convert (float a) { return vector4(a,a,a,a); }
//   colN <-> vecN
vector mx_convert (color a) { return (vector)a; }
vector4 mx_convert (color4 a) { return vector4 (a.rgb[0], a.rgb[1], a.rgb[2], a.a); }
color mx_convert (vector a) { return (color)a; }
color4 mx_convert (vector4 a) { return color4 (color(a.x,a.y,a.z), a.w); }
//   col3 <-> col4
color mx_convert (color4 a) { return a.rgb; }
color4 mx_convert (color a) { return color4(a,1.0); }

// Define mx_add() overloaded for all MX types.
float mx_add (float a, float b) { return a+b; }
point mx_add (point a, point b) { return a+b; }
point mx_add (point a, float b) { return a+b; }
vector mx_add (vector a, vector b) { return a+b; }
vector mx_add (vector a, float b) { return a+b; }
vector2 mx_add (vector2 a, vector2 b) { return a+b; }
vector2 mx_add (vector2 a, float b) { return a+b; }
vector4 mx_add (vector4 a, vector4 b) { return a+b; }
vector4 mx_add (vector4 a, float b) { return a+b; }
color mx_add (color a, color b) { return a+b; }
color mx_add (color a, float b) { return a+b; }
color4 mx_add (color4 a, color4 b) { return a+b; }
color4 mx_add (color4 a, float b) { return a+b; }
closure color mx_add (closure color a, closure color b) { return a+b; }

matrix33 mx_add(matrix33 a, matrix33 b)
{
    return matrix33 (matrix(
        a.m[0][0]+b.m[0][0], a.m[0][1]+b.m[0][1], a.m[0][2]+b.m[0][2], 0.0,
        a.m[1][0]+b.m[1][0], a.m[1][1]+b.m[1][1], a.m[1][2]+b.m[1][2], 0.0,
        a.m[2][0]+b.m[2][0], a.m[2][1]+b.m[2][1], a.m[2][2]+b.m[2][2], 0.0,
        0.0, 0.0, 0.0, 1.0));
}

matrix33 mx_add(matrix33 a, float b)
{
    return matrix33 (matrix(a.m[0][0]+b, a.m[0][1]+b, a.m[0][2]+b, 0.0,
                            a.m[1][0]+b, a.m[1][1]+b, a.m[1][2]+b, 0.0,
                            a.m[2][0]+b, a.m[2][1]+b, a.m[2][2]+b, 0.0,
                            0.0, 0.0, 0.0, 1.0));
}

matrix mx_add(matrix a, matrix b)
{
    return matrix (a[0][0]+b[0][0], a[0][1]+b[0][1], a[0][2]+b[0][2], a[0][3]+b[0][3],
                   a[1][0]+b[1][0], a[1][1]+b[1][1], a[1][2]+b[1][2], a[1][3]+b[1][3],
                   a[2][0]+b[2][0], a[2][1]+b[2][1], a[2][2]+b[2][2], a[2][3]+b[2][3],
                   a[3][0]+b[3][0], a[3][1]+b[3][1], a[3][2]+b[3][2], a[3][3]+b[3][3]);
}

matrix mx_add(matrix a, float b)
{
    return matrix (a[0][0]+b, a[0][1]+b, a[0][2]+b, a[0][3]+b,
                   a[1][0]+b, a[1][1]+b, a[1][2]+b, a[1][3]+b,
                   a[2][0]+b, a[2][1]+b, a[2][2]+b, a[2][3]+b,
                   a[3][0]+b, a[3][1]+b, a[3][2]+b, a[3][3]+b);
}


// Define mx_sub() overloaded for all MX types.
float mx_sub (float a, float b) { return a-b; }
point mx_sub (point a, point b) { return a-b; }
point mx_sub (point a, float b) { return a-b; }
vector mx_sub (vector a, vector b) { return a-b; }
vector mx_sub (vector a, float b) { return a-b; }
vector2 mx_sub (vector2 a, vector2 b) { return a-b; }
vector2 mx_sub (vector2 a, float b) { return a-b; }
vector4 mx_sub (vector4 a, vector4 b) { return a-b; }
vector4 mx_sub (vector4 a, float b) { return a-b; }
color mx_sub (color a, color b) { return a-b; }
color mx_sub (color a, float b) { return a-b; }
color4 mx_sub (color4 a, color4 b) { return a-b; }
color4 mx_sub (color4 a, float b) { return a-b; }

matrix33 mx_sub (matrix33 a, matrix33 b)
{
    return matrix33 (matrix(
        a.m[0][0]-b.m[0][0], a.m[0][1]-b.m[0][1], a.m[0][2]-b.m[0][2], 0.0,
        a.m[1][0]-b.m[1][0], a.m[1][1]-b.m[1][1], a.m[1][2]-b.m[1][2], 0.0,
        a.m[2][0]-b.m[2][0], a.m[2][1]-b.m[2][1], a.m[2][2]-b.m[2][2], 0.0,
        0.0, 0.0, 0.0, 1.0));
}

matrix33 mx_sub (matrix33 a, float b)
{
    return matrix33 (matrix(
        a.m[0][0]-b, a.m[0][1]-b, a.m[0][2]-b, 0.0,
        a.m[1][0]-b, a.m[1][1]-b, a.m[1][2]-b, 0.0,
        a.m[2][0]-b, a.m[2][1]-b, a.m[2][2]-b, 0.0,
        0.0, 0.0, 0.0, 1.0));
}

matrix mx_sub (matrix a, matrix b)
{
    return matrix(a[0][0]-b[0][0], a[0][1]-b[0][1], a[0][2]-b[0][2], a[0][3]-b[0][3],
                  a[1][0]-b[1][0], a[1][1]-b[1][1], a[1][2]-b[1][2], a[1][3]-b[1][3],
                  a[2][0]-b[2][0], a[2][1]-b[2][1], a[2][2]-b[2][2], a[2][3]-b[2][3],
                  a[3][0]-b[3][0], a[3][1]-b[3][1], a[3][2]-b[3][2], a[3][3]-b[3][3]);
}

matrix mx_sub (matrix a, float b)
{
    return matrix (a[0][0]-b, a[0][1]-b, a[0][2]-b, a[0][3]-b,
                   a[1][0]-b, a[1][1]-b, a[1][2]-b, a[1][3]-b,
                   a[2][0]-b, a[2][1]-b, a[2][2]-b, a[2][3]-b,
                   a[3][0]-b, a[3][1]-b, a[3][2]-b, a[3][3]-b);
}



// remap `in` from [inLow, inHigh] to [outLow, outHigh], optionally clamping
// to the new range.
//
float remap(float in, float inLow, float inHigh, float outLow, float outHigh, int doClamp)
{
      float x = (in - inLow)/(inHigh-inLow);
      if (doClamp == 1) {
           x = clamp(x, 0, 1);
      }
      return outLow + (outHigh - outLow) * x;
}

color remap(color in, color inLow, color inHigh, color outLow, color outHigh, int doClamp)
{
      color x = (in - inLow) / (inHigh - inLow);
      if (doClamp == 1) {
           x = clamp(x, 0, 1);
      }
      return outLow + (outHigh - outLow) * x;
}

color remap(color in, float inLow, float inHigh, float outLow, float outHigh, int doClamp)
{
      color x = (in - inLow) / (inHigh - inLow);
      if (doClamp == 1) {
           x = clamp(x, 0, 1);
      }
      return outLow + (outHigh - outLow) * x;
}

color4 remap(color4 c, color4 inLow, color4 inHigh, color4 outLow, color4 outHigh, int doClamp)
{
      return color4(remap(c.rgb, inLow.rgb, inHigh.rgb, outLow.rgb, outHigh.rgb, doClamp),
                    remap(c.a, inLow.a, inHigh.a, outLow.a, outHigh.a, doClamp));
}

color4 remap(color4 c, float inLow, float inHigh, float outLow, float outHigh, int doClamp)
{
    color4 c4_inLow = color4(color(inLow), inLow);
    color4 c4_inHigh = color4(color(inHigh), inHigh);
    color4 c4_outLow = color4(color(outLow), outLow);
    color4 c4_outHigh = color4(color(outHigh), outHigh);
    return remap(c, c4_inLow, c4_inHigh, c4_outLow, c4_outHigh, doClamp);
}

vector2 remap(vector2 in, vector2 inLow, vector2 inHigh, vector2 outLow, vector2 outHigh, int doClamp)
{
    return vector2 (remap(in.x, inLow.x, inHigh.x, outLow.x, outHigh.x, doClamp),
                    remap(in.y, inLow.y, inHigh.y, outLow.y, outHigh.y, doClamp));
}

vector2 remap(vector2 in, float inLow, float inHigh, float outLow, float outHigh, int doClamp)
{
    return vector2 (remap(in.x, inLow, inHigh, outLow, outHigh, doClamp),
                    remap(in.y, inLow, inHigh, outLow, outHigh, doClamp));
}

vector4 remap(vector4 in, vector4 inLow, vector4 inHigh, vector4 outLow, vector4 outHigh, int doClamp)
{
    return vector4 (remap(in.x, inLow.x, inHigh.x, outLow.x, outHigh.x, doClamp),
                    remap(in.y, inLow.y, inHigh.y, outLow.y, outHigh.y, doClamp),
                    remap(in.z, inLow.z, inHigh.z, outLow.z, outHigh.z, doClamp),
                    remap(in.w, inLow.w, inHigh.w, outLow.w, outHigh.w, doClamp));
}

vector4 remap(vector4 in, float inLow, float inHigh, float outLow, float outHigh, int doClamp)
{
    return vector4 (remap(in.x, inLow, inHigh, outLow, outHigh, doClamp),
                    remap(in.y, inLow, inHigh, outLow, outHigh, doClamp),
                    remap(in.z, inLow, inHigh, outLow, outHigh, doClamp),
                    remap(in.w, inLow, inHigh, outLow, outHigh, doClamp));
}

float catmull_rom_curve(int i0, int i1, int i2, int i3, color values[], float along, int c)
{
        // geometry matrix (tension = 0.5):
        //
        // [  0.0  1.0  0.0  0.0 ]
        // [ -0.5  0.0  0.5  0.0 ]
        // [  1.0 -2.5  2.0 -0.5 ]
        // [ -0.5  1.5 -1.5  0.5 ]

        float M12 =  1.0;
        float M21 = -0.5;
        float M23 =  0.5;
        float M31 =  1.0;
        float M32 = -2.5;
        float M33 =  2.0;
        float M34 = -0.5;
        float M41 = -0.5;
        float M42 =  1.5;
        float M43 = -1.5;
        float M44 =  0.5;

        float v0 = values[i0][c];
        float v1 = values[i1][c];
        float v2 = values[i2][c];
        float v3 = values[i3][c];

        float c1 = M12 * v1;
        float c2 = M21 * v0 + M23 * v2;
        float c3 = M31 * v0 + M32 * v1 + M33 * v2 + M34 * v3;
        float c4 = M41 * v0 + M42 * v1 + M43 * v2 + M44 * v3;

        return ((c4 * along + c3) * along + c2) * along + c1;
}
    
float fritsch_carlson_curve(int i0, int i1, int i2, int i3, color values[], float positions[], int last, float along, int c)
{
        float i1_tangent = 0;
        float i2_tangent = 0;

        float i0_interval = positions[i1] - positions[i0];
        float i1_interval = positions[i2] - positions[i1];
        float i2_interval = positions[i3] - positions[i2];

        float i0_delta = (values[i1][c] - values[i0][c]) / i0_interval;
        float i1_delta = (values[i2][c] - values[i1][c]) / i1_interval;
        float i2_delta = (values[i3][c] - values[i2][c]) / i2_interval;

        if (i1 > 0 && (i1_delta * i0_delta) >= 0.0) {
            i1_tangent = 3.0 * (i0_interval + i1_interval)
                        / ((2.0 * i1_interval + i0_interval) / i0_delta
                        + (i1_interval + 2.0  * i0_interval) / i1_delta);
        }

        if (i2 < (last) && (i2_delta * i1_delta) >= 0.0) {
                        i2_tangent = 3.0 * (i1_interval + i2_interval)
                        / ((2.0 * i2_interval + i1_interval) / i1_delta
                        + (i2_interval + 2.0 * i1_interval) / i2_delta);
        }

        float invint = 1.0 / i1_interval;
        float si = i1_delta;
        float ci = (3.0 * si - 2.0 * i1_tangent - i2_tangent) * invint;
        float di = (i1_tangent + i2_tangent - 2.0 * si) * invint * invint;

        float t = along * i1_interval;
        float t2 = t * t;
        float t3 = t * t2;
        return values[i1][c] + i1_tangent * t + ci * t2 + di * t3;
}

float hermite_curve(int i0, int i1, int i2, int i3, color values[], float along, int c)
{
    // Tension: 1 is high, 0 normal, -1 is low
    // Bias: 0 is even,
    //    positive is towards first segment,
    //    negative towards the other

    float tension = 0.5;
    float bias = 0.0;

    float y0 = values[i0][c];
    float y1 = values[i1][c];
    float y2 = values[i2][c];
    float y3 = values[i3][c];

    float mu2 = along * along;
    float mu3 = mu2 * along;
    float m0  = (y1-y0)*(1+bias)*(1-tension)/2 + (y2-y1)*(1-bias)*(1-tension)/2;
    float m1  = (y2-y1)*(1+bias)*(1-tension)/2 + (y3-y2)*(1-bias)*(1-tension)/2;
    float a0 =  2*mu3 - 3*mu2 + 1;
    float a1 =    mu3 - 2*mu2 + along;
    float a2 =    mu3 -   mu2;
    float a3 = -2*mu3 + 3*mu2;

    return(a0 * y1 + a1 * m0 + a2 * m1 + a3 * y2);
}

float curve_interpolate(float in, float positions[], color values[], int interpolationMode[], int c)
{
    // Curve interpolation utility adapted for MaterialX from Sean Willis based on:
    // ColorCurves.osl, by Zap Andersson
    //    https://github.com/ADN-DevTech/3dsMax-OSL-Shaders/blob/master/3ds%20Max%20Shipping%20Shaders/ColorCurves.osl
    //
    int l = arraylength(positions);
    int last = l-1;
    // clip floor
    if (in < 0)
        return 0;
    // clip ceil
    if (in > 1)
        return 0;

    // Out of range below
    if (in < positions[0])
        return values[0][c];    


    if (in > positions[last])
        return values[last][c];

    // Find the segment
    int i;
    for (i = 0; i < last; i++)
    {
        if (in >= positions[i] and in <= positions[i+1])
            break;
    }

    int i0 = max(0, i-1);
    int i1 = i;
    int i2 = i+1;
    int i3 = min(last, i+2);

    // Get interpolationMode
    int interp = interpolationMode[i1];

    float delta = positions[i2] - positions[i1];
    // Segment length zero (or less) Return start
    if (delta <= 0.0)
        return values[i1][c];

    // Figure out where in the range we are....
    float along = (in - positions[i1]) / delta;
    // Simple linear interpolation
    if (interp == 0)
        return mix(values[i1][c], values[i2][c], along);
    if (interp == 1)
        return catmull_rom_curve(i0, i1, i2, i3, values, along, c);
    if (interp == 2)
        return fritsch_carlson_curve(i0, i1, i2, i3, values, positions, last, along, c);
    if (interp == 3)
        return hermite_curve(i0, i1, i2, i3, values, along, c);
}

float fgamma(float in, float g)
{
    return sign(in) * pow(abs(in), g);
}

color fgamma(color in, color g)
{
    return sign(in) * pow(abs(in), g);
}

color fgamma(color in, float g)
{
    return sign(in) * pow(abs(in), g);
}

color4 fgamma(color4 a, color4 b)
{
    return color4(fgamma(a.rgb, b.rgb), fgamma(a.a, b.a));
}

color4 fgamma(color4 a, float b)
{
    return fgamma(a, color4(color(b), b));
}

vector2 fgamma(vector2 in, vector2 g)
{
    return vector2 (fgamma(in.x, g.x), fgamma(in.y, g.y));
}

vector2 fgamma(vector2 in, float g)
{
    return vector2 (fgamma(in.x, g), fgamma(in.y, g));
}

vector4 fgamma(vector4 in, vector4 g)
{
    return vector4 (fgamma(in.x, g.x),
                    fgamma(in.y, g.y),
                    fgamma(in.z, g.z),
                    fgamma(in.w, g.w));
}

vector4 fgamma(vector4 in, float g)
{
    return vector4 (fgamma(in.x, g),
                    fgamma(in.y, g),
                    fgamma(in.z, g),
                    fgamma(in.w, g));
}



//
// contrast scales the input around a central `pivot` value.
//
float contrast(float in, float amount, float pivot)
{
    float out = in - pivot;
    out *= amount;
    out += pivot;
    return out;
}

color contrast(color in, color amount, color pivot)
{
    color out = in - pivot;
    out *= amount;
    out += pivot;
    return out;
}

color contrast(color in, float amount, float pivot)
{
    color out = in - pivot;
    out *= amount;
    out += pivot;
    return out;
}

color4 contrast(color4 c, color4 amount, color4 pivot)
{
    return color4(contrast(c.rgb, amount.rgb, pivot.rgb),
                  contrast(c.a, amount.a, pivot.a));
}

color4 contrast(color4 c, float amount, float pivot)
{
    return contrast(c, color4(color(amount), amount), color4(color(pivot), pivot));
}

vector2 contrast(vector2 in, vector2 amount, vector2 pivot)
{
    return vector2 (contrast(in.x, amount.x, pivot.x),
                    contrast(in.y, amount.y, pivot.y));
}

vector2 contrast(vector2 in, float amount, float pivot)
{
    return contrast(in, vector2(amount, amount), vector2(pivot, pivot));
}

vector4 contrast(vector4 in, vector4 amount, vector4 pivot)
{
    return vector4 (contrast(in.x, amount.x, pivot.x),
                    contrast(in.y, amount.y, pivot.y),
                    contrast(in.z, amount.z, pivot.z),
                    contrast(in.w, amount.w, pivot.w));
}

vector4 contrast(vector4 in, float amount, float pivot)
{
    return vector4 (contrast(in.x, amount, pivot),
                    contrast(in.y, amount, pivot),
                    contrast(in.z, amount, pivot),
                    contrast(in.w, amount, pivot));
}



vector2 noise (string noisetype, float x, float y)
{
    color cnoise = (color) noise (noisetype, x, y);
    return vector2 (cnoise[0], cnoise[1]);
}

color4 noise (string noisetype, float x, float y)
{
    color cnoise = (color) noise (noisetype, x, y);
    float fnoise = (float) noise (noisetype, x + 19, y + 73);
    return color4 (cnoise, fnoise);
}

vector4 noise (string noisetype, float x, float y)
{
    color cnoise = (color) noise (noisetype, x, y);
    float fnoise = (float) noise (noisetype, x + 19, y + 73);
    return vector4 (cnoise[0], cnoise[1], cnoise[2], fnoise);
}


vector2 noise (string noisetype, point position)
{
    color cnoise = (color) noise (noisetype, position);
    return vector2 (cnoise[0], cnoise[1]);
}

color4 noise (string noisetype, point position)
{
    color cnoise = (color) noise (noisetype, position);
    float fnoise = (float) noise (noisetype, position+vector(19,73,29));
    return color4 (cnoise, fnoise);
}

vector4 noise (string noisetype, point position)
{
    color cnoise = (color) noise (noisetype, position);
    float fnoise = (float) noise (noisetype, position+vector(19,73,29));
    return vector4 (cnoise[0], cnoise[1], cnoise[2], fnoise);
}



vector2 cellnoise (float x, float y)
{
    color cnoise = (color) cellnoise (x, y);
    return vector2 (cnoise[0], cnoise[1]);
}

color4 cellnoise (float x, float y)
{
    color cnoise = (color) cellnoise (x, y);
    float fnoise = (float) cellnoise (x + 19, y + 73);
    return color4 (cnoise, fnoise);
}

vector4 cellnoise (float x, float y)
{
    color cnoise = (color) cellnoise (x, y);
    float fnoise = (float) cellnoise (x + 19, y + 73);
    return vector4 (cnoise[0], cnoise[1], cnoise[2], fnoise);
}



vector2 cellnoise (point position)
{
    color cnoise = (color) cellnoise (position);
    return vector2 (cnoise[0], cnoise[1]);
}

color4 cellnoise (point position)
{
    color cnoise = (color) cellnoise (position);
    float fnoise = (float) cellnoise (position+vector(19,73,29));
    return color4 (cnoise, fnoise);
}

vector4 cellnoise (point position)
{
    color cnoise = (color) cellnoise (position);
    float fnoise = (float) cellnoise (position+vector(19,73,29));
    return vector4 (cnoise[0], cnoise[1], cnoise[2], fnoise);
}



//
// fractional Brownian motion
//
float fBm( point position, int octaves, float lacunarity, float diminish, string noisetype)
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

color fBm( point position, int octaves, float lacunarity, float diminish, string noisetype)
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

vector2 fBm( point position, int octaves, float lacunarity, float diminish, string noisetype)
{
    return vector2 ((float) fBm (position, octaves, lacunarity, diminish, noisetype),
                    (float) fBm (position+point(19, 193, 17), octaves, lacunarity, diminish, noisetype));
}

color4 fBm( point position, int octaves, float lacunarity, float diminish, string noisetype)
{
    color c = (color) fBm (position, octaves, lacunarity, diminish, noisetype);
    float f = (float) fBm (position+point(19, 193, 17), octaves, lacunarity, diminish, noisetype);
    return color4 (c, f);
}

vector4 fBm( point position, int octaves, float lacunarity, float diminish, string noisetype)
{
    color c = (color) fBm (position, octaves, lacunarity, diminish, noisetype);
    float f = (float) fBm (position+point(19, 193, 17), octaves, lacunarity, diminish, noisetype);
    return vector4 (c[0], c[1], c[2], f);
}






float swizzle_float (float in[4], string channels)
{
    float out;
    float outF[4];
    int c_len = strlen(channels);

    for (int i=0; i<c_len; i++) {
        string ch = substr(channels, i, 1);
        if (ch == "r" || ch == "x")
            outF[i] = in[0];
        else if (ch == "g" || ch == "y")
            outF[i] = in[1];
        else if (ch == "b" || ch == "z")
            outF[i] = in[2];
        else if (ch == "a" || ch == "w")
            outF[i] = in[3];
        else if(ch == "1")
            outF[i] = 1;
        else
            outF[i] = 0;
    }
    out = outF[0];
    return out;
}



color swizzle_color (float in[4], string channels)
{
    color out;
    float outF[4];
    int c_len = strlen(channels);

    for (int i=0; i<c_len; i++) {
        string ch = substr(channels, i, 1);
        if (ch == "r" || ch == "x")
            outF[i] = in[0];
        else if (ch == "g" || ch == "y")
            outF[i] = in[1];
        else if (ch == "b" || ch == "z")
            outF[i] = in[2];
        else if (ch == "a" || ch == "w")
            outF[i] = in[3];
        else if(ch == "1")
            outF[i] = 1;
        else
            outF[i] = 0;
    }
    return color(outF[0],outF[1],outF[2]);
}



vector swizzle_vector (float in[4], string channels)
{
    vector out;
    float outF[4];
    int c_len = strlen(channels);

    for (int i=0; i<c_len; i++) {
        string ch = substr(channels, i, 1);
        if (ch == "r" || ch == "x")
            outF[i] = in[0];
        else if (ch == "g" || ch == "y")
            outF[i] = in[1];
        else if (ch == "b" || ch == "z")
            outF[i] = in[2];
        else if (ch == "a" || ch == "w")
            outF[i] = in[3];
        else if(ch == "1")
            outF[i] = 1;
        else
            outF[i] = 0;
    }
    return vector(outF[0],outF[1],outF[2]);
}



color4 swizzle_color4 (float in[4], string channels)
{
    color4  out;
    float outF[4];
    int c_len = strlen(channels);

    for (int i=0; i<c_len; i++) {
        string ch = substr(channels, i, 1);
        if (ch == "r" || ch == "x")
            outF[i] = in[0];
        else if (ch == "g" || ch == "y")
            outF[i] = in[1];
        else if (ch == "b" || ch == "z")
            outF[i] = in[2];
        else if (ch == "a" || ch == "w")
            outF[i] = in[3];
        else if(ch == "1")
            outF[i] = 1;
        else
            outF[i] = 0;
    }
    out.rgb = color(outF[0],outF[1],outF[2]);
    out.a = outF[3];

    return out;
}


vector2 swizzle_vector2 (float in[4], string channels)
{
    vector2  out;
    float outF[4];
    int c_len = strlen(channels);

    for (int i=0; i<c_len; i++) {
        string ch = substr(channels, i, 1);
        if (ch == "r" || ch == "x")
            outF[i] = in[0];
        else if (ch == "g" || ch == "y")
            outF[i] = in[1];
        else if (ch == "b" || ch == "z")
            outF[i] = in[2];
        else if (ch == "a" || ch == "w")
            outF[i] = in[3];
        else if(ch == "1")
            outF[i] = 1;
        else
            outF[i] = 0;
    }
    out.x = outF[0];
    out.y = outF[1];

    return out;
}



vector4 swizzle_vector4 (float in[4], string channels)
{
    vector4  out;
    float outF[4];
    int c_len = strlen(channels);

    for (int i=0; i<c_len; i++) {
        string ch = substr(channels, i, 1);
        if (ch == "r" || ch == "x")
            outF[i] = in[0];
        else if (ch == "g" || ch == "y")
            outF[i] = in[1];
        else if (ch == "b" || ch == "z")
            outF[i] = in[2];
        else if (ch == "a" || ch == "w")
            outF[i] = in[3];
        else if(ch == "1")
            outF[i] = 1;
        else
            outF[i] = 0;
    }
    out.x = outF[0];
    out.y = outF[1];
    out.z = outF[2];
    out.w = outF[3];
    return out;
}


//
// setup_missing_color_alpha() implements all the type permutations for
// setting up missingColor and missingAlpha given the default_value (and its
// specific type).
//

void setup_missing_color_alpha (float default_value,
              output color missingColor, output float missingAlpha)
{
    missingColor = default_value;
    missingAlpha = 1;
}

void setup_missing_color_alpha (color default_value,
              output color missingColor, output float missingAlpha)
{
    missingColor = default_value;
    missingAlpha = 1;
}


void setup_missing_color_alpha (vector default_value,
              output color missingColor, output float missingAlpha)
{
    missingColor = vector (default_value);
    missingAlpha = 1;
}

void setup_missing_color_alpha (vector2 default_value,
              output color missingColor, output float missingAlpha)
{
    missingColor = color (default_value.x, default_value.y, 0);
    missingAlpha = 1;
}

void setup_missing_color_alpha (vector4 default_value,
              output color missingColor, output float missingAlpha)
{
    missingColor = color (default_value.x, default_value.y, default_value.z);
    missingAlpha = default_value.w;
}

void setup_missing_color_alpha (color4 default_value,
              output color missingColor, output float missingAlpha)
{
    missingColor = color (default_value.rgb);
    missingAlpha = default_value.a;
}



//
// combine() combines an up to 4 floats, or an rgb and alpha, into the given
// return type, in a way that makes as much sense as possible.
//
float combine (float a, float b, float c, float d)
{
    return a;
}

color combine (float a, float b, float c, float d)
{
    return color (a, b, c);
}

vector combine (float a, float b, float c, float d)
{
    return vector (a, b, c);
}

vector2 combine (float a, float b, float c, float d)
{
    return vector2 (a, b);
}

color4 combine (float a, float b, float c, float d)
{
    return color4 (color(a,b,c), d);
}

vector4 combine (float a, float b, float c, float d)
{
    return vector4 (a, b, c, d);
}


float combine (color rgb, float alpha)
{
    return rgb[0];
}

color combine (color rgb, float alpha)
{
    return rgb;
}

vector combine (color rgb, float alpha)
{
    return (vector)rgb;
}

vector2 combine (color rgb, float alpha)
{
    return vector2 (rgb[0], rgb[1]);
}

color4 combine (color rgb, float alpha)
{
    return color4 (rgb, alpha);
}

vector4 combine (color rgb, float alpha)
{
    return vector4 (rgb[0], rgb[1], rgb[2], alpha);
}


//
// extract(in,index) returns one indexed float from the aggregate.
//

float extract (vector2 in, int index)
{
    return index == 0 ? in.x : in.y;
}


float extract (color in, int index)
{
    return in[index];
}

float extract (vector in, int index)
{
    return in[index];
}


float extract (color4 in, int index)
{
    return index < 3 ? in.rgb[index] : in.a;
}

float extract (vector4 in, int index)
{
    float r;
    if      (index == 0) r = in.x;
    else if (index == 2) r = in.y;
    else if (index == 3) r = in.z;
    else                 r = in.w;
    return r;
}



// DEPRECATED: MatrialX <= 1.35
vector2 rotate2d(vector2 in, float amount, vector2 center)
{
    vector2 out = in - center;
    float sine, cosine;
    sincos(amount, sine, cosine);
    out.x = in.x * cosine - in.y * sine;
    out.y = in.y * cosine + in.x * sine;
    out = out + center;
    return out;
}

vector2 rotate (vector2 in, float amount,
                vector axis /*unused in the 2D case*/)
{
    vector2 out = in;
    float sine, cosine;
    sincos(amount, sine, cosine);
    out.x = in.x * cosine - in.y * sine;
    out.y = in.y * cosine + in.x * sine;
    out = out;
    return out;
}
