// Noise Library (WGSL port of genglsl/lib/mx_noise.glsl)
//
// This library is a modified version of the noise library found in
// Open Shading Language:
// github.com/imageworks/OpenShadingLanguage/blob/master/src/include/OSL/oslnoise.h
//
// It contains the subset of noise types needed to implement the MaterialX
// standard library. The modifications are conversions from C++ to GLSL to WGSL.
// Produced results should be identical to the OSL noise functions.
//
// Original copyright notice:
// ------------------------------------------------------------------------
// Copyright (c) 2009-2010 Sony Pictures Imageworks Inc., et al.
// All Rights Reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// * Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
// * Neither the name of Sony Pictures Imageworks nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// ------------------------------------------------------------------------

fn mx_select(b: bool, t: f32, f: f32) -> f32 {
    return select(f, t, b);
}

fn mx_negate_if(val: f32, b: bool) -> f32 {
    return select(val, -val, b);
}

fn mx_floor(x: f32) -> i32 {
    return i32(floor(x));
}

// Return mx_floor as well as the fractional remainder.
fn mx_floorfrac(x: f32, i: ptr<function, i32>) -> f32 {
    *i = mx_floor(x);
    return x - f32(*i);
}

// ---------------------------------------------------------------------------
// Bilinear / trilinear interpolation
// ---------------------------------------------------------------------------

fn mx_bilerp_f32(v0: f32, v1: f32, v2: f32, v3: f32, s: f32, t: f32) -> f32 {
    let s1 = 1.0 - s;
    return (1.0 - t) * (v0 * s1 + v1 * s) + t * (v2 * s1 + v3 * s);
}

fn mx_bilerp_vec3(v0: vec3f, v1: vec3f, v2: vec3f, v3: vec3f, s: f32, t: f32) -> vec3f {
    let s1 = 1.0 - s;
    return (1.0 - t) * (v0 * s1 + v1 * s) + t * (v2 * s1 + v3 * s);
}

fn mx_trilerp_f32(v0: f32, v1: f32, v2: f32, v3: f32,
              v4: f32, v5: f32, v6: f32, v7: f32,
              s: f32, t: f32, r: f32) -> f32 {
    let s1 = 1.0 - s;
    let t1 = 1.0 - t;
    let r1 = 1.0 - r;
    return r1 * (t1 * (v0 * s1 + v1 * s) + t * (v2 * s1 + v3 * s)) +
            r * (t1 * (v4 * s1 + v5 * s) + t * (v6 * s1 + v7 * s));
}

fn mx_trilerp_vec3(v0: vec3f, v1: vec3f, v2: vec3f, v3: vec3f,
                v4: vec3f, v5: vec3f, v6: vec3f, v7: vec3f,
                s: f32, t: f32, r: f32) -> vec3f {
    let s1 = 1.0 - s;
    let t1 = 1.0 - t;
    let r1 = 1.0 - r;
    return r1 * (t1 * (v0 * s1 + v1 * s) + t * (v2 * s1 + v3 * s)) +
            r * (t1 * (v4 * s1 + v5 * s) + t * (v6 * s1 + v7 * s));
}

// ---------------------------------------------------------------------------
// Gradient functions
// ---------------------------------------------------------------------------
// 2D and 3D gradient functions — perform a dot product against a randomly
// chosen vector. The gradient vector is not normalized, but this only affects
// the overall "scale" of the result, so we account for it by multiplying in
// the corresponding "perlin" function.

// 2D gradient: 8 possible directions (+-1,+-2) and (+-2,+-1)
fn mx_gradient_float_2d(hash: u32, x: f32, y: f32) -> f32 {
    let h = hash & 7u;
    let u = select(y, x, h < 4u);
    let v = 2.0 * select(x, y, h < 4u);
    return mx_negate_if(u, bool(h & 1u)) + mx_negate_if(v, bool(h & 2u));
}

// 3D gradient: vectors pointing to the edges of the cube
fn mx_gradient_float_3d(hash: u32, x: f32, y: f32, z: f32) -> f32 {
    let h = hash & 15u;
    let u = select(y, x, h < 8u);
    let v = select(select(z, x, (h == 12u) || (h == 14u)), y, h < 4u);
    return mx_negate_if(u, bool(h & 1u)) + mx_negate_if(v, bool(h & 2u));
}

// 2D vec3 gradient
fn mx_gradient_vec3_2d(hash: vec3<u32>, x: f32, y: f32) -> vec3f {
    return vec3f(
        mx_gradient_float_2d(hash.x, x, y),
        mx_gradient_float_2d(hash.y, x, y),
        mx_gradient_float_2d(hash.z, x, y)
    );
}

// 3D vec3 gradient
fn mx_gradient_vec3_3d(hash: vec3<u32>, x: f32, y: f32, z: f32) -> vec3f {
    return vec3f(
        mx_gradient_float_3d(hash.x, x, y, z),
        mx_gradient_float_3d(hash.y, x, y, z),
        mx_gradient_float_3d(hash.z, x, y, z)
    );
}

// Scaling factors to normalize the result of gradients above.
// Experimentally calculated: 2D = 0.6616, 3D = 0.9820.
fn mx_gradient_scale2d_f32(v: f32) -> f32 { return 0.6616 * v; }
fn mx_gradient_scale3d_f32(v: f32) -> f32 { return 0.9820 * v; }
fn mx_gradient_scale2d_vec3(v: vec3f) -> vec3f { return 0.6616 * v; }
fn mx_gradient_scale3d_vec3(v: vec3f) -> vec3f { return 0.9820 * v; }

// ---------------------------------------------------------------------------
// Bob Jenkins hash (bjmix / bjfinal)
// ---------------------------------------------------------------------------

// Bitwise circular rotation left by k bits (32-bit unsigned).
fn mx_rotl32(x: u32, k: i32) -> u32 {
    return (x << u32(k)) | (x >> u32(32 - k));
}

fn mx_bjmix(a: ptr<function, u32>, b: ptr<function, u32>, c: ptr<function, u32>) {
    *a -= *c; *a ^= mx_rotl32(*c,  4); *c += *b;
    *b -= *a; *b ^= mx_rotl32(*a,  6); *a += *c;
    *c -= *b; *c ^= mx_rotl32(*b,  8); *b += *a;
    *a -= *c; *a ^= mx_rotl32(*c, 16); *c += *b;
    *b -= *a; *b ^= mx_rotl32(*a, 19); *a += *c;
    *c -= *b; *c ^= mx_rotl32(*b,  4); *b += *a;
}

// Mix up and combine the bits of a, b, and c (returns a hash of those three
// original values without changing them).
fn mx_bjfinal(a: u32, b: u32, c: u32) -> u32 {
    var a_ = a; var b_ = b; var c_ = c;
    c_ ^= b_; c_ -= mx_rotl32(b_, 14);
    a_ ^= c_; a_ -= mx_rotl32(c_, 11);
    b_ ^= a_; b_ -= mx_rotl32(a_, 25);
    c_ ^= b_; c_ -= mx_rotl32(b_, 16);
    a_ ^= c_; a_ -= mx_rotl32(c_,  4);
    b_ ^= a_; b_ -= mx_rotl32(a_, 14);
    c_ ^= b_; c_ -= mx_rotl32(b_, 24);
    return c_;
}

// Convert a 32-bit integer into a floating point number in [0,1].
fn mx_bits_to_01(bits: u32) -> f32 {
    return f32(bits) / f32(0xffffffffu);
}

// Quintic fade curve: 6t^5 - 15t^4 + 10t^3
fn mx_fade(t: f32) -> f32 {
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

// ---------------------------------------------------------------------------
// Hash functions
// ---------------------------------------------------------------------------
// WGSL has no function overloading, so we suffix by argument count:
//   mx_hash_int_i1(x), mx_hash_int_i2(x,y), mx_hash_int_i3(x,y,z), etc.

fn mx_hash_int_i1(x: i32) -> u32 {
    let len = 1u;
    let seed = 0xdeadbeefu + (len << 2u) + 13u;
    return mx_bjfinal(seed + u32(x), seed, seed);
}

fn mx_hash_int_i2(x: i32, y: i32) -> u32 {
    let len = 2u;
    var a = 0xdeadbeefu + (len << 2u) + 13u;
    var b = a; var c = a;
    a += u32(x);
    b += u32(y);
    return mx_bjfinal(a, b, c);
}

fn mx_hash_int_i3(x: i32, y: i32, z: i32) -> u32 {
    let len = 3u;
    var a = 0xdeadbeefu + (len << 2u) + 13u;
    var b = a; var c = a;
    a += u32(x);
    b += u32(y);
    c += u32(z);
    return mx_bjfinal(a, b, c);
}

fn mx_hash_int_i4(x: i32, y: i32, z: i32, xx: i32) -> u32 {
    let len = 4u;
    var a = 0xdeadbeefu + (len << 2u) + 13u;
    var b = a; var c = a;
    a += u32(x);
    b += u32(y);
    c += u32(z);
    mx_bjmix(&a, &b, &c);
    a += u32(xx);
    return mx_bjfinal(a, b, c);
}

fn mx_hash_int_i5(x: i32, y: i32, z: i32, xx: i32, yy: i32) -> u32 {
    let len = 5u;
    var a = 0xdeadbeefu + (len << 2u) + 13u;
    var b = a; var c = a;
    a += u32(x);
    b += u32(y);
    c += u32(z);
    mx_bjmix(&a, &b, &c);
    a += u32(xx);
    b += u32(yy);
    return mx_bjfinal(a, b, c);
}

// vec3 hash from 2 int args
fn mx_hash_vec3_i2(x: i32, y: i32) -> vec3<u32> {
    let h = mx_hash_int_i2(x, y);
    return vec3<u32>(
        (h      ) & 0xFFu,
        (h >> 8 ) & 0xFFu,
        (h >> 16) & 0xFFu
    );
}

// vec3 hash from 3 int args
fn mx_hash_vec3_i3(x: i32, y: i32, z: i32) -> vec3<u32> {
    let h = mx_hash_int_i3(x, y, z);
    return vec3<u32>(
        (h      ) & 0xFFu,
        (h >> 8 ) & 0xFFu,
        (h >> 16) & 0xFFu
    );
}

// ---------------------------------------------------------------------------
// Perlin noise
// ---------------------------------------------------------------------------

// 2D Perlin noise (float output)
fn mx_perlin_noise_float_2d(p: vec2f) -> f32 {
    var X: i32; var Y: i32;
    let fx = mx_floorfrac(p.x, &X);
    let fy = mx_floorfrac(p.y, &Y);
    let u = mx_fade(fx);
    let v = mx_fade(fy);
    let result = mx_bilerp_f32(
        mx_gradient_float_2d(mx_hash_int_i2(X,   Y  ), fx,       fy      ),
        mx_gradient_float_2d(mx_hash_int_i2(X+1, Y  ), fx - 1.0, fy      ),
        mx_gradient_float_2d(mx_hash_int_i2(X,   Y+1), fx,       fy - 1.0),
        mx_gradient_float_2d(mx_hash_int_i2(X+1, Y+1), fx - 1.0, fy - 1.0),
        u, v);
    return mx_gradient_scale2d_f32(result);
}

// 3D Perlin noise (float output)
fn mx_perlin_noise_float_3d(p: vec3f) -> f32 {
    var X: i32; var Y: i32; var Z: i32;
    let fx = mx_floorfrac(p.x, &X);
    let fy = mx_floorfrac(p.y, &Y);
    let fz = mx_floorfrac(p.z, &Z);
    let u = mx_fade(fx);
    let v = mx_fade(fy);
    let w = mx_fade(fz);
    let result = mx_trilerp_f32(
        mx_gradient_float_3d(mx_hash_int_i3(X,   Y,   Z  ), fx,       fy,       fz      ),
        mx_gradient_float_3d(mx_hash_int_i3(X+1, Y,   Z  ), fx - 1.0, fy,       fz      ),
        mx_gradient_float_3d(mx_hash_int_i3(X,   Y+1, Z  ), fx,       fy - 1.0, fz      ),
        mx_gradient_float_3d(mx_hash_int_i3(X+1, Y+1, Z  ), fx - 1.0, fy - 1.0, fz      ),
        mx_gradient_float_3d(mx_hash_int_i3(X,   Y,   Z+1), fx,       fy,       fz - 1.0),
        mx_gradient_float_3d(mx_hash_int_i3(X+1, Y,   Z+1), fx - 1.0, fy,       fz - 1.0),
        mx_gradient_float_3d(mx_hash_int_i3(X,   Y+1, Z+1), fx,       fy - 1.0, fz - 1.0),
        mx_gradient_float_3d(mx_hash_int_i3(X+1, Y+1, Z+1), fx - 1.0, fy - 1.0, fz - 1.0),
        u, v, w);
    return mx_gradient_scale3d_f32(result);
}

// 2D Perlin noise (vec3 output)
fn mx_perlin_noise_vec3_2d(p: vec2f) -> vec3f {
    var X: i32; var Y: i32;
    let fx = mx_floorfrac(p.x, &X);
    let fy = mx_floorfrac(p.y, &Y);
    let u = mx_fade(fx);
    let v = mx_fade(fy);
    let result = mx_bilerp_vec3(
        mx_gradient_vec3_2d(mx_hash_vec3_i2(X,   Y  ), fx,       fy      ),
        mx_gradient_vec3_2d(mx_hash_vec3_i2(X+1, Y  ), fx - 1.0, fy      ),
        mx_gradient_vec3_2d(mx_hash_vec3_i2(X,   Y+1), fx,       fy - 1.0),
        mx_gradient_vec3_2d(mx_hash_vec3_i2(X+1, Y+1), fx - 1.0, fy - 1.0),
        u, v);
    return mx_gradient_scale2d_vec3(result);
}

// 3D Perlin noise (vec3 output)
fn mx_perlin_noise_vec3_3d(p: vec3f) -> vec3f {
    var X: i32; var Y: i32; var Z: i32;
    let fx = mx_floorfrac(p.x, &X);
    let fy = mx_floorfrac(p.y, &Y);
    let fz = mx_floorfrac(p.z, &Z);
    let u = mx_fade(fx);
    let v = mx_fade(fy);
    let w = mx_fade(fz);
    let result = mx_trilerp_vec3(
        mx_gradient_vec3_3d(mx_hash_vec3_i3(X,   Y,   Z  ), fx,       fy,       fz      ),
        mx_gradient_vec3_3d(mx_hash_vec3_i3(X+1, Y,   Z  ), fx - 1.0, fy,       fz      ),
        mx_gradient_vec3_3d(mx_hash_vec3_i3(X,   Y+1, Z  ), fx,       fy - 1.0, fz      ),
        mx_gradient_vec3_3d(mx_hash_vec3_i3(X+1, Y+1, Z  ), fx - 1.0, fy - 1.0, fz      ),
        mx_gradient_vec3_3d(mx_hash_vec3_i3(X,   Y,   Z+1), fx,       fy,       fz - 1.0),
        mx_gradient_vec3_3d(mx_hash_vec3_i3(X+1, Y,   Z+1), fx - 1.0, fy,       fz - 1.0),
        mx_gradient_vec3_3d(mx_hash_vec3_i3(X,   Y+1, Z+1), fx,       fy - 1.0, fz - 1.0),
        mx_gradient_vec3_3d(mx_hash_vec3_i3(X+1, Y+1, Z+1), fx - 1.0, fy - 1.0, fz - 1.0),
        u, v, w);
    return mx_gradient_scale3d_vec3(result);
}

// ---------------------------------------------------------------------------
// Cell noise
// ---------------------------------------------------------------------------

// f32 input → f32 output
fn mx_cell_noise_float_f32(p: f32) -> f32 {
    let ix = mx_floor(p);
    return mx_bits_to_01(mx_hash_int_i1(ix));
}

// vec2 input → f32 output
fn mx_cell_noise_float_vec2(p: vec2f) -> f32 {
    let ix = mx_floor(p.x);
    let iy = mx_floor(p.y);
    return mx_bits_to_01(mx_hash_int_i2(ix, iy));
}

// vec3 input → f32 output
fn mx_cell_noise_float_vec3(p: vec3f) -> f32 {
    let ix = mx_floor(p.x);
    let iy = mx_floor(p.y);
    let iz = mx_floor(p.z);
    return mx_bits_to_01(mx_hash_int_i3(ix, iy, iz));
}

// vec4 input → f32 output
fn mx_cell_noise_float_vec4(p: vec4f) -> f32 {
    let ix = mx_floor(p.x);
    let iy = mx_floor(p.y);
    let iz = mx_floor(p.z);
    let iw = mx_floor(p.w);
    return mx_bits_to_01(mx_hash_int_i4(ix, iy, iz, iw));
}

// f32 input → vec3 output
fn mx_cell_noise_vec3_f32(p: f32) -> vec3f {
    let ix = mx_floor(p);
    return vec3f(
        mx_bits_to_01(mx_hash_int_i2(ix, 0)),
        mx_bits_to_01(mx_hash_int_i2(ix, 1)),
        mx_bits_to_01(mx_hash_int_i2(ix, 2))
    );
}

// vec2 input → vec3 output
fn mx_cell_noise_vec3_vec2(p: vec2f) -> vec3f {
    let ix = mx_floor(p.x);
    let iy = mx_floor(p.y);
    return vec3f(
        mx_bits_to_01(mx_hash_int_i3(ix, iy, 0)),
        mx_bits_to_01(mx_hash_int_i3(ix, iy, 1)),
        mx_bits_to_01(mx_hash_int_i3(ix, iy, 2))
    );
}

// vec3 input → vec3 output
fn mx_cell_noise_vec3_vec3(p: vec3f) -> vec3f {
    let ix = mx_floor(p.x);
    let iy = mx_floor(p.y);
    let iz = mx_floor(p.z);
    return vec3f(
        mx_bits_to_01(mx_hash_int_i4(ix, iy, iz, 0)),
        mx_bits_to_01(mx_hash_int_i4(ix, iy, iz, 1)),
        mx_bits_to_01(mx_hash_int_i4(ix, iy, iz, 2))
    );
}

// vec4 input → vec3 output
fn mx_cell_noise_vec3_vec4(p: vec4f) -> vec3f {
    let ix = mx_floor(p.x);
    let iy = mx_floor(p.y);
    let iz = mx_floor(p.z);
    let iw = mx_floor(p.w);
    return vec3f(
        mx_bits_to_01(mx_hash_int_i5(ix, iy, iz, iw, 0)),
        mx_bits_to_01(mx_hash_int_i5(ix, iy, iz, iw, 1)),
        mx_bits_to_01(mx_hash_int_i5(ix, iy, iz, iw, 2))
    );
}

// ---------------------------------------------------------------------------
// Fractal noise (fBm)
// ---------------------------------------------------------------------------

fn mx_fractal2d_noise_float(p: vec2f, octaves: i32, lacunarity: f32, diminish: f32) -> f32 {
    var result = 0.0;
    var amplitude = 1.0;
    var pp = p;
    for (var i = 0; i < octaves; i++) {
        result += amplitude * mx_perlin_noise_float_2d(pp);
        amplitude *= diminish;
        pp *= lacunarity;
    }
    return result;
}

fn mx_fractal2d_noise_vec3(p: vec2f, octaves: i32, lacunarity: f32, diminish: f32) -> vec3f {
    var result = vec3f(0.0);
    var amplitude = 1.0;
    var pp = p;
    for (var i = 0; i < octaves; i++) {
        result += amplitude * mx_perlin_noise_vec3_2d(pp);
        amplitude *= diminish;
        pp *= lacunarity;
    }
    return result;
}

fn mx_fractal2d_noise_vec2(p: vec2f, octaves: i32, lacunarity: f32, diminish: f32) -> vec2f {
    return vec2f(
        mx_fractal2d_noise_float(p, octaves, lacunarity, diminish),
        mx_fractal2d_noise_float(p + vec2f(19.0, 193.0), octaves, lacunarity, diminish)
    );
}

fn mx_fractal2d_noise_vec4(p: vec2f, octaves: i32, lacunarity: f32, diminish: f32) -> vec4f {
    let c = mx_fractal2d_noise_vec3(p, octaves, lacunarity, diminish);
    let f = mx_fractal2d_noise_float(p + vec2f(19.0, 193.0), octaves, lacunarity, diminish);
    return vec4f(c, f);
}

fn mx_fractal3d_noise_float(p: vec3f, octaves: i32, lacunarity: f32, diminish: f32) -> f32 {
    var result = 0.0;
    var amplitude = 1.0;
    var pp = p;
    for (var i = 0; i < octaves; i++) {
        result += amplitude * mx_perlin_noise_float_3d(pp);
        amplitude *= diminish;
        pp *= lacunarity;
    }
    return result;
}

fn mx_fractal3d_noise_vec3(p: vec3f, octaves: i32, lacunarity: f32, diminish: f32) -> vec3f {
    var result = vec3f(0.0);
    var amplitude = 1.0;
    var pp = p;
    for (var i = 0; i < octaves; i++) {
        result += amplitude * mx_perlin_noise_vec3_3d(pp);
        amplitude *= diminish;
        pp *= lacunarity;
    }
    return result;
}

fn mx_fractal3d_noise_vec2(p: vec3f, octaves: i32, lacunarity: f32, diminish: f32) -> vec2f {
    return vec2f(
        mx_fractal3d_noise_float(p, octaves, lacunarity, diminish),
        mx_fractal3d_noise_float(p + vec3f(19.0, 193.0, 17.0), octaves, lacunarity, diminish)
    );
}

fn mx_fractal3d_noise_vec4(p: vec3f, octaves: i32, lacunarity: f32, diminish: f32) -> vec4f {
    let c = mx_fractal3d_noise_vec3(p, octaves, lacunarity, diminish);
    let f = mx_fractal3d_noise_float(p + vec3f(19.0, 193.0, 17.0), octaves, lacunarity, diminish);
    return vec4f(c, f);
}

// ---------------------------------------------------------------------------
// Worley noise
// ---------------------------------------------------------------------------

// 2D cell position for Worley noise
fn mx_worley_cell_position_2d(x: i32, y: i32, xoff: i32, yoff: i32, jitter: f32) -> vec2f {
    let tmp = mx_cell_noise_vec3_vec2(vec2f(f32(x + xoff), f32(y + yoff)));
    var off = vec2f(tmp.x, tmp.y);
    off -= 0.5;
    off *= jitter;
    off += 0.5;
    return vec2f(f32(x), f32(y)) + off;
}

// 3D cell position for Worley noise
fn mx_worley_cell_position_3d(x: i32, y: i32, z: i32, xoff: i32, yoff: i32, zoff: i32, jitter: f32) -> vec3f {
    var off = mx_cell_noise_vec3_vec3(vec3f(f32(x + xoff), f32(y + yoff), f32(z + zoff)));
    off -= 0.5;
    off *= jitter;
    off += 0.5;
    return vec3f(f32(x), f32(y), f32(z)) + off;
}

// 2D Worley distance
fn mx_worley_distance_2d(p: vec2f, x: i32, y: i32, xoff: i32, yoff: i32, jitter: f32, metric: i32) -> f32 {
    let cellpos = mx_worley_cell_position_2d(x, y, xoff, yoff, jitter);
    let diff = cellpos - p;
    if (metric == 2) {
        return abs(diff.x) + abs(diff.y);             // Manhattan
    }
    if (metric == 3) {
        return max(abs(diff.x), abs(diff.y));          // Chebyshev
    }
    return dot(diff, diff);                            // Euclidean^2
}

// 3D Worley distance
fn mx_worley_distance_3d(p: vec3f, x: i32, y: i32, z: i32, xoff: i32, yoff: i32, zoff: i32, jitter: f32, metric: i32) -> f32 {
    let cellpos = mx_worley_cell_position_3d(x, y, z, xoff, yoff, zoff, jitter);
    let diff = cellpos - p;
    if (metric == 2) {
        return abs(diff.x) + abs(diff.y) + abs(diff.z); // Manhattan
    }
    if (metric == 3) {
        return max(max(abs(diff.x), abs(diff.y)), abs(diff.z)); // Chebyshev
    }
    return dot(diff, diff);                              // Euclidean^2
}

// 2D Worley noise (float output)
fn mx_worley_noise_float_2d(p: vec2f, jitter: f32, style: i32, metric: i32) -> f32 {
    var X: i32; var Y: i32;
    let localpos = vec2f(mx_floorfrac(p.x, &X), mx_floorfrac(p.y, &Y));
    var sqdist = 1e6;
    var minpos = vec2f(0.0, 0.0);
    for (var x = -1; x <= 1; x++) {
        for (var y = -1; y <= 1; y++) {
            let dist = mx_worley_distance_2d(localpos, x, y, X, Y, jitter, metric);
            let cellpos = mx_worley_cell_position_2d(x, y, X, Y, jitter) - localpos;
            if (dist < sqdist) {
                sqdist = dist;
                minpos = cellpos;
            }
        }
    }
    if (style == 1) {
        return mx_cell_noise_float_vec2(minpos + p);
    } else {
        if (metric == 0) { sqdist = sqrt(sqdist); }
        return sqdist;
    }
}

// 2D Worley noise (vec2 output)
fn mx_worley_noise_vec2_2d(p: vec2f, jitter: f32, style: i32, metric: i32) -> vec2f {
    var X: i32; var Y: i32;
    let localpos = vec2f(mx_floorfrac(p.x, &X), mx_floorfrac(p.y, &Y));
    var sqdist = vec2f(1e6, 1e6);
    var minpos = vec2f(0.0, 0.0);
    for (var x = -1; x <= 1; x++) {
        for (var y = -1; y <= 1; y++) {
            let dist = mx_worley_distance_2d(localpos, x, y, X, Y, jitter, metric);
            let cellpos = mx_worley_cell_position_2d(x, y, X, Y, jitter) - localpos;
            if (dist < sqdist.x) {
                sqdist.y = sqdist.x;
                sqdist.x = dist;
                minpos = cellpos;
            } else if (dist < sqdist.y) {
                sqdist.y = dist;
            }
        }
    }
    if (style == 1) {
        let tmp = mx_cell_noise_vec3_vec2(minpos + p);
        return vec2f(tmp.x, tmp.y);
    } else {
        if (metric == 0) { sqdist = sqrt(sqdist); }
        return sqdist;
    }
}

// 2D Worley noise (vec3 output)
fn mx_worley_noise_vec3_2d(p: vec2f, jitter: f32, style: i32, metric: i32) -> vec3f {
    var X: i32; var Y: i32;
    let localpos = vec2f(mx_floorfrac(p.x, &X), mx_floorfrac(p.y, &Y));
    var sqdist = vec3f(1e6, 1e6, 1e6);
    var minpos = vec2f(0.0, 0.0);
    for (var x = -1; x <= 1; x++) {
        for (var y = -1; y <= 1; y++) {
            let dist = mx_worley_distance_2d(localpos, x, y, X, Y, jitter, metric);
            let cellpos = mx_worley_cell_position_2d(x, y, X, Y, jitter) - localpos;
            if (dist < sqdist.x) {
                sqdist.z = sqdist.y;
                sqdist.y = sqdist.x;
                sqdist.x = dist;
                minpos = cellpos;
            } else if (dist < sqdist.y) {
                sqdist.z = sqdist.y;
                sqdist.y = dist;
            } else if (dist < sqdist.z) {
                sqdist.z = dist;
            }
        }
    }
    if (style == 1) {
        return mx_cell_noise_vec3_vec2(minpos + p);
    } else {
        if (metric == 0) { sqdist = sqrt(sqdist); }
        return sqdist;
    }
}

// 3D Worley noise (float output)
fn mx_worley_noise_float_3d(p: vec3f, jitter: f32, style: i32, metric: i32) -> f32 {
    var X: i32; var Y: i32; var Z: i32;
    let localpos = vec3f(mx_floorfrac(p.x, &X), mx_floorfrac(p.y, &Y), mx_floorfrac(p.z, &Z));
    var sqdist = 1e6;
    var minpos = vec3f(0.0, 0.0, 0.0);
    for (var x = -1; x <= 1; x++) {
        for (var y = -1; y <= 1; y++) {
            for (var z = -1; z <= 1; z++) {
                let dist = mx_worley_distance_3d(localpos, x, y, z, X, Y, Z, jitter, metric);
                let cellpos = mx_worley_cell_position_3d(x, y, z, X, Y, Z, jitter) - localpos;
                if (dist < sqdist) {
                    sqdist = dist;
                    minpos = cellpos;
                }
            }
        }
    }
    if (style == 1) {
        return mx_cell_noise_float_vec3(minpos + p);
    } else {
        if (metric == 0) { sqdist = sqrt(sqdist); }
        return sqdist;
    }
}

// 3D Worley noise (vec2 output)
fn mx_worley_noise_vec2_3d(p: vec3f, jitter: f32, style: i32, metric: i32) -> vec2f {
    var X: i32; var Y: i32; var Z: i32;
    let localpos = vec3f(mx_floorfrac(p.x, &X), mx_floorfrac(p.y, &Y), mx_floorfrac(p.z, &Z));
    var sqdist = vec2f(1e6, 1e6);
    var minpos = vec3f(0.0, 0.0, 0.0);
    for (var x = -1; x <= 1; x++) {
        for (var y = -1; y <= 1; y++) {
            for (var z = -1; z <= 1; z++) {
                let dist = mx_worley_distance_3d(localpos, x, y, z, X, Y, Z, jitter, metric);
                let cellpos = mx_worley_cell_position_3d(x, y, z, X, Y, Z, jitter) - localpos;
                if (dist < sqdist.x) {
                    sqdist.y = sqdist.x;
                    sqdist.x = dist;
                    minpos = cellpos;
                } else if (dist < sqdist.y) {
                    sqdist.y = dist;
                }
            }
        }
    }
    if (style == 1) {
        let tmp = mx_cell_noise_vec3_vec3(minpos + p);
        return vec2f(tmp.x, tmp.y);
    } else {
        if (metric == 0) { sqdist = sqrt(sqdist); }
        return sqdist;
    }
}

// 3D Worley noise (vec3 output)
fn mx_worley_noise_vec3_3d(p: vec3f, jitter: f32, style: i32, metric: i32) -> vec3f {
    var X: i32; var Y: i32; var Z: i32;
    let localpos = vec3f(mx_floorfrac(p.x, &X), mx_floorfrac(p.y, &Y), mx_floorfrac(p.z, &Z));
    var sqdist = vec3f(1e6, 1e6, 1e6);
    var minpos = vec3f(0.0, 0.0, 0.0);
    for (var x = -1; x <= 1; x++) {
        for (var y = -1; y <= 1; y++) {
            for (var z = -1; z <= 1; z++) {
                let dist = mx_worley_distance_3d(localpos, x, y, z, X, Y, Z, jitter, metric);
                let cellpos = mx_worley_cell_position_3d(x, y, z, X, Y, Z, jitter) - localpos;
                if (dist < sqdist.x) {
                    sqdist.z = sqdist.y;
                    sqdist.y = sqdist.x;
                    sqdist.x = dist;
                    minpos = cellpos;
                } else if (dist < sqdist.y) {
                    sqdist.z = sqdist.y;
                    sqdist.y = dist;
                } else if (dist < sqdist.z) {
                    sqdist.z = dist;
                }
            }
        }
    }
    if (style == 1) {
        return mx_cell_noise_vec3_vec3(minpos + p);
    } else {
        if (metric == 0) { sqdist = sqrt(sqdist); }
        return sqdist;
    }
}
