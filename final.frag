/*
 * This file is part of the Electron Orbital Explorer. The Electron
 * Orbital Explorer is distributed under the Simplified BSD License
 * (also called the "BSD 2-Clause License"), in hopes that these
 * rendering techniques might be used by other programmers in
 * applications such as scientific visualization, video gaming, and so
 * on. If you find value in this software and use its technologies for
 * another purpose, I would love to hear back from you at bjthinks (at)
 * gmail (dot) com. If you improve this software and agree to release
 * your modifications under the below license, I encourage you to fork
 * the development tree on github and push your modifications. The
 * Electron Orbital Explorer's development URL is:
 * https://github.com/bjthinks/orbital-explorer
 * (This paragraph is not part of the software license and may be
 * removed.)
 *
 * Copyright (c) 2013, Brian W. Johnson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * + Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * + Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#version 150

in vec2 coord;
out vec3 RGB;
uniform sampler2D solidData;
uniform sampler2D cloudData;
uniform mat2x2 color_trans;
uniform bool use_color;
uniform float brightness;

vec2 uv_white = vec2(0.19784, 0.46832);
mat3 uv_to_XYZ = mat3(9, 0, -3,
                      0, 4, -20,
                      0, 0,  12);
mat3 XYZ_to_RGB = mat3(+3.2406, -0.9689, +0.0557,
                       -1.5372,  1.8758, -0.2040,
                       -0.4986,  0.0415,  1.0570);
mat3 uv_to_RGB = XYZ_to_RGB * uv_to_XYZ;

float distance_to_gamut_edge(vec2 uv_from_white, float brightness)
{
  vec4 cos_coeffs0 = vec4(0.103516,  0.060547, 0.013672, 0.007812);
  vec4 cos_coeffs1 = vec4(0.066406,  0.011718, 0.005859, 0.000000);
  vec4 ph_coeffs0 = vec4( 0.000000,  0.589049, 1.767146, 4.908738);
  vec4 ph_coeffs1 = vec4(-0.196350, -1.570796, 3.141593, 0.000000);

  float angle = atan(uv_from_white.y, uv_from_white.x);
  vec4 multiples_of_angle = vec4(0., 1., 2., 3.) * angle;
  vec4 cos_values0 = cos(multiples_of_angle + ph_coeffs0);
  vec4 cos_values1 = cos(multiples_of_angle + ph_coeffs1);
  float t = pow(2.0 * brightness, 1.625);
  return
    dot(cos_values0, cos_coeffs0) * (1.0 - t) +
    dot(cos_values1, cos_coeffs1) * t;
}

void main(void)
{
  // The input is integrated (real, imag, mag) from rendering multiple
  // tetrahedra with additive blending.
  vec3 integrated_rim = texture(cloudData, coord).xyz;

  // Extract u, v, and Y from the input.

  // Integral of intensity (Y) along line of sight.
  float integrated_Y = integrated_rim.z;

  // Integral of intensity-scaled chromaticity (u * Y and v * Y), divided
  // by total intensity (Y), gives intensity-weighted chromaticity.
  // These are the pre-scaled uv values, offset so white point is origin.
  // Maximum magnitude of this vector is 1.
  vec2 pre_uv;
  if (integrated_Y > 0)
    pre_uv = integrated_rim.xy / integrated_Y;
  else
    pre_uv = vec2(0, 0);

  // Color rotation
  pre_uv = pre_uv * color_trans;

  // Brightness adjustment.
  integrated_Y *= brightness;

  // Exponential fall-off of intensity. Has no effect on chromaticity.
  // This takes into account that nearby "particles" of cloud or fog
  // will invariably block some fraction of farther-away "particles".
  float cloud_Y = 1 - exp(-integrated_Y);

  // Make the maximum Y value be 0.5. This keeps us within a well-saturated
  // region of the sRGB gamut.
  cloud_Y *= 0.5;

  // Scale uv so that it is in-gamut
  // FIXME: This is not correct for solid objects
  vec2 cloud_uv;
  if (use_color && integrated_Y > 0.0)
    cloud_uv = distance_to_gamut_edge(pre_uv, cloud_Y) * pre_uv;
  else
    cloud_uv = vec2(0, 0);

  // Translate the origin to the white point.
  cloud_uv += uv_white;

  // Convert CIE (u,v) color coordinates (as per CIELUV) to (x,y)
  vec2 cloud_xy = vec2(9.0, 4.0) * cloud_uv;
  cloud_xy /= dot(vec3(6.0, -16.0, 12.0), vec3(cloud_uv, 1.0));

  // Solid object blending
  vec3 solid_xyY = texture(solidData, coord).xyz;
  // Diminish solid illuminance by the amount of cloud attenuation in
  // front of it.
  float solid_Y = solid_xyY[2];
  solid_Y *= 1 - cloud_Y;
  // Compute final Y value
  float Y = solid_Y + cloud_Y;
  // Take weighted average of xy values
  vec3 xyz;
  xyz.xy = (solid_xyY.xy * solid_Y + cloud_xy * cloud_Y) / Y;
  xyz.z = 1 - xyz.x - xyz.y;

  // Convert xyz to XYZ
  vec3 XYZ = (Y / xyz.y) * xyz;

  // Convert XYZ to linear (i.e. pre-gamma) RGB values
  mat3 XYZ_to_linear_RGB = mat3(+3.2406, -0.9689,  0.0557,
                                -1.5372,  1.8758, -0.2040,
                                -0.4986,  0.0415,  1.0570);
  vec3 linear_RGB = XYZ_to_linear_RGB * XYZ;

  // Gamut clamping - keeping hue constant, desaturate towards an equally
  // intense grey until the RGB values are in [0,1]. This operation
  // effectively happens to the CIE XYZ values, and we are using the fact
  // that XYZ and pre-gamma RGB are related by a linear transformation to
  // streamline the calculation.
  vec3 grey_RGB = vec3(Y);
  vec3 RGB_overshoot = max(linear_RGB - vec3(1.0), vec3(0.0));
  vec3 blet = RGB_overshoot / (linear_RGB - grey_RGB); // NaN problem here?
  float t = max(blet.r, max(blet.g, blet.b));
  linear_RGB = mix(linear_RGB, grey_RGB, t);
  //if (any(greaterThan(RGB_overshoot, vec3(0.001))))
  //linear_RGB = vec3(0);

  // Gamma correction is performed by GL_FRAMEBUFFER_SRGB
  RGB = linear_RGB;
}
