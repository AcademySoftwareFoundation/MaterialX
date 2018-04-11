/*
This noise library is a modified version of the noise library found
in Open Shading Language. The modifications are mainly conversions 
from C++ to GLSL.

Original library:
github.com/imageworks/OpenShadingLanguage/blob/master/src/include/OSL/oslnoise.h

Original copyright notice:
---------------------------------------------------------------------
Copyright (c) 2009-2010 Sony Pictures Imageworks Inc., et al.
All Rights Reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
* Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
* Neither the name of Sony Pictures Imageworks nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
---------------------------------------------------------------------
*/

float scale1(float v) { return 0.2500f * v; }
float scale2(float v) { return 0.6616f * v; }
float scale3(float v) { return 0.9820f * v; }
float scale4(float v) { return 0.8344f * v; }

vec3 scale1(vec3 v) { return 0.2500f * v; }
vec3 scale2(vec3 v) { return 0.6616f * v; }
vec3 scale3(vec3 v) { return 0.9820f * v; }
vec3 scale4(vec3 v) { return 0.8344f * v; }

float bilerp(float v0, float v1, float v2, float v3, float s, float t)
{
    float s1 = 1.0f - s;
    return (1.0f - t) * (v0*s1 + v1*s) + t * (v2*s1 + v3*s);
}
vec3 bilerp(vec3 v0, vec3 v1, vec3 v2, vec3 v3, float s, float t)
{
    float s1 = 1.0f - s;
    return (1.0f - t) * (v0*s1 + v1*s) + t * (v2*s1 + v3*s);
}

float trilerp(float v0, float v1, float v2, float v3, float v4, float v5, float v6, float v7, float s, float t, float r)
{
    float s1 = 1.0 - s;
    float t1 = 1.0 - t;
    float r1 = 1.0 - r;
    return (r1*(t1*(v0*s1 + v1*s) + t*(v2*s1 + v3*s)) +
            r*(t1*(v4*s1 + v5*s) + t*(v6*s1 + v7*s)));
}
vec3 trilerp(vec3 v0, vec3 v1, vec3 v2, vec3 v3, vec3 v4, vec3 v5, vec3 v6, vec3 v7, float s, float t, float r)
{
    float s1 = 1.0 - s;
    float t1 = 1.0 - t;
    float r1 = 1.0 - r;
    return (r1*(t1*(v0*s1 + v1*s) + t*(v2*s1 + v3*s)) +
            r*(t1*(v4*s1 + v5*s) + t*(v6*s1 + v7*s)));
}

float select(bool b, float t, float f)
{
    return b ? t : f;
}

float negate_if(float val, bool b)
{
    return b ? -val : val;
}

// return the greatest integer <= x
int quick_floor(float x)
{
    return int(x) - ((x < 0) ? 1 : 0);
}

// floorfrac return quick_floor as well as the fractional remainder
float floorfrac(float x, out int i)
{
    i = quick_floor(x);
    return x - i;
}

// Perlin 'fade' function.
float fade(float t)
{
   return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

// 1,2,3 and 4 dimensional gradient functions - perform a dot product against a
// randomly chosen vector. Note that the gradient vector is not normalized, but
// this only affects the overal "scale" of the result, so we simply account for
// the scale by multiplying in the corresponding "perlin" function.
// These factors were experimentally calculated to be:
//    1D:   0.188
//    2D:   0.507
//    3D:   0.936
//    4D:   0.870
float grad(uint hash, float x, float y)
{
    // 8 possible directions (+-1,+-2) and (+-2,+-1)
    uint h = hash & 7;
    float u = select(h<4, x, y);
    float v = 2.0f * select(h<4, y, x);
    // compute the dot product with (x,y).
    return negate_if(u, bool(h&1)) + negate_if(v, bool(h&2));
}
float grad(uint hash, float x, float y, float z)
{
    // use vectors pointing to the edges of the cube
    uint h = hash & 15;
    float u = select(h<8, x, y);
    float v = select(h<4, y, select((h==12)||(h==14), x, z));
    return negate_if(u, bool(h&1)) + negate_if(v, bool(h&2));
}
vec3 grad(uvec3 hash, float x, float y)
{
    return vec3(grad(hash.x, x, y), grad(hash.y, x, y), grad(hash.z, x, y));
}
vec3 grad(uvec3 hash, float x, float y, float z)
{
    return vec3(grad(hash.x, x, y, z), grad(hash.y, x, y, z), grad(hash.z, x, y, z));
}

/// Bitwise circular rotation left by k bits (for 32 bit unsigned integers)
uint rotl32(uint x, int k)
{
    return (x<<k) | (x>>(32-k));
}

// Mix up and combine the bits of a, b, and c (doesn't change them, but
// returns a hash of those three original values).
uint bjfinal(uint a, uint b, uint c /*=0xdeadbeef*/)
{
    c ^= b; c -= rotl32(b,14);
    a ^= c; a -= rotl32(c,11);
    b ^= a; b -= rotl32(a,25);
    c ^= b; c -= rotl32(b,16);
    a ^= c; a -= rotl32(c,4);
    b ^= a; b -= rotl32(a,14);
    c ^= b; c -= rotl32(b,24);
    return c;
}

uint inthash(int x, int y)
{
    uint a, b, c, len = 2;
    a = b = c = 0xdeadbeef + (len << 2) + 13;
    switch (len)
    {
        case 2 : b += y;
        case 1 : a += x;
        c = bjfinal(a, b, c);
        case 0:
            break;
    }
    return c;
}

uint inthash(int x, int y, int z)
{
    uint a, b, c, len = 3;
    a = b = c = 0xdeadbeef + (len << 2) + 13;
    switch (len)
    {
        case 3 : c += z;
        case 2 : b += y;
        case 1 : a += x;
        c = bjfinal(a, b, c);
        case 0:
            break;
    }
    return c;
}

uvec3 hash3(int x, int y)
{
    uint h = inthash(x, y);
    // we only need the low-order bits to be random, so split out
    // the 32 bit result into 3 parts for each channel
    uvec3 result;
    result.x = (h      ) & 0xFF;
    result.y = (h >> 8 ) & 0xFF;
    result.z = (h >> 16) & 0xFF;
    return result;
}

uvec3 hash3(int x, int y, int z)
{
    uint h = inthash(x, y, z);
    // we only need the low-order bits to be random, so split out
    // the 32 bit result into 3 parts for each channel
    uvec3 result;
    result.x = (h      ) & 0xFF;
    result.y = (h >> 8 ) & 0xFF;
    result.z = (h >> 16) & 0xFF;
    return result;
}


float perlin_noise1(float x, float y)
{
    int X, Y; 
    float fx = floorfrac(x, X);
    float fy = floorfrac(y, Y);
    float u = fade(fx);
    float v = fade(fy);
    float result = bilerp(grad(inthash(X  , Y  ), fx    , fy     ),
                          grad(inthash(X+1, Y  ), fx-1.0, fy     ),
                          grad(inthash(X  , Y+1), fx    , fy-1.0),
                          grad(inthash(X+1, Y+1), fx-1.0, fy-1.0), 
                          u, v);
    return scale2(result);
}

float perlin_noise1(float x, float y, float z)
{
    int X, Y, Z; 
    float fx = floorfrac(x, X);
    float fy = floorfrac(y, Y);
    float fz = floorfrac(z, Z);
    float u = fade(fx);
    float v = fade(fy);
    float w = fade(fz);
    float result = trilerp(grad(inthash(X  , Y  , Z  ), fx     , fy     , fz     ),
                           grad(inthash(X+1, Y  , Z  ), fx-1.0f, fy     , fz     ),
                           grad(inthash(X  , Y+1, Z  ), fx     , fy-1.0f, fz     ),
                           grad(inthash(X+1, Y+1, Z  ), fx-1.0f, fy-1.0f, fz     ),
                           grad(inthash(X  , Y  , Z+1), fx     , fy     , fz-1.0f),
                           grad(inthash(X+1, Y  , Z+1), fx-1.0f, fy     , fz-1.0f),
                           grad(inthash(X  , Y+1, Z+1), fx     , fy-1.0f, fz-1.0f),
                           grad(inthash(X+1, Y+1, Z+1), fx-1.0f, fy-1.0f, fz-1.0f),
                           u, v, w);
    return scale3(result);
}

vec3 perlin_noise3(float x, float y)
{
    int X, Y; 
    float fx = floorfrac(x, X);
    float fy = floorfrac(y, Y);
    float u = fade(fx);
    float v = fade(fy);
    vec3 result = bilerp(grad(hash3(X  , Y  ), fx    , fy     ),
                         grad(hash3(X+1, Y  ), fx-1.0, fy     ),
                         grad(hash3(X  , Y+1), fx    , fy-1.0f),
                         grad(hash3(X+1, Y+1), fx-1.0, fy-1.0f), 
                         u, v);
    return scale2(result);
}

vec3 perlin_noise3(float x, float y, float z)
{
    int X, Y, Z; 
    float fx = floorfrac(x, X);
    float fy = floorfrac(y, Y);
    float fz = floorfrac(z, Z);
    float u = fade(fx);
    float v = fade(fy);
    float w = fade(fz);
    vec3 result = trilerp(grad(hash3(X  , Y  , Z  ), fx     , fy     , fz     ),
                          grad(hash3(X+1, Y  , Z  ), fx-1.0f, fy     , fz     ),
                          grad(hash3(X  , Y+1, Z  ), fx     , fy-1.0f, fz     ),
                          grad(hash3(X+1, Y+1, Z  ), fx-1.0f, fy-1.0f, fz     ),
                          grad(hash3(X  , Y  , Z+1), fx     , fy     , fz-1.0f),
                          grad(hash3(X+1, Y  , Z+1), fx-1.0f, fy     , fz-1.0f),
                          grad(hash3(X  , Y+1, Z+1), fx     , fy-1.0f, fz-1.0f),
                          grad(hash3(X+1, Y+1, Z+1), fx-1.0f, fy-1.0f, fz-1.0f),
                          u, v, w);
    return scale3(result);
}
