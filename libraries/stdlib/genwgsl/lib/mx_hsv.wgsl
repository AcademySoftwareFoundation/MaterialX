// Color transform functions (WGSL port of genglsl/lib/mx_hsv.glsl)
//
// These functions are modified versions of the color operators found in
// Open Shading Language:
// github.com/imageworks/OpenShadingLanguage/blob/master/src/liboslexec/opcolor.cpp
//
// It contains the subset of color operators needed to implement the MaterialX
// standard library. The modifications are conversions from C++ to GLSL to WGSL.
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

fn mx_hsvtorgb(hsv: vec3f) -> vec3f {
    // Reference for this technique: Foley & van Dam
    let h = hsv.x;
    let s = hsv.y;
    let v = hsv.z;

    if (s < 0.0001) {
        return vec3f(v, v, v);
    }

    let hue = 6.0 * (h - floor(h));  // expand to [0..6)
    let hi = i32(trunc(hue));
    let f = hue - f32(hi);
    let p = v * (1.0 - s);
    let q = v * (1.0 - s * f);
    let t = v * (1.0 - s * (1.0 - f));

    if (hi == 0) { return vec3f(v, t, p); }
    if (hi == 1) { return vec3f(q, v, p); }
    if (hi == 2) { return vec3f(p, v, t); }
    if (hi == 3) { return vec3f(p, q, v); }
    if (hi == 4) { return vec3f(t, p, v); }
    return vec3f(v, p, q);
}

fn mx_rgbtohsv(c: vec3f) -> vec3f {
    // See Foley & van Dam
    let r = c.x;
    let g = c.y;
    let b = c.z;
    let mincomp = min(r, min(g, b));
    let maxcomp = max(r, max(g, b));
    let delta = maxcomp - mincomp;  // chroma

    let v = maxcomp;
    var s: f32;
    if (maxcomp > 0.0) {
        s = delta / maxcomp;
    } else {
        s = 0.0;
    }

    var h: f32;
    if (s <= 0.0) {
        h = 0.0;
    } else {
        if (r >= maxcomp)      { h = (g - b) / delta; }
        else if (g >= maxcomp) { h = 2.0 + (b - r) / delta; }
        else                   { h = 4.0 + (r - g) / delta; }
        h *= (1.0 / 6.0);
        if (h < 0.0) { h += 1.0; }
    }
    return vec3f(h, s, v);
}
