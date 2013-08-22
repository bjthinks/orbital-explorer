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

layout(lines_adjacency) in;
layout(triangle_strip,max_vertices=21) out;

uniform vec2 nearfar;

in vec4 inverted_position[4];
in vec3 integrand[4];

noperspective out float one_over_w_front;
noperspective out float one_over_w_back;
noperspective out vec3 integrand_over_w_front;
noperspective out vec3 integrand_over_w_back;
noperspective out vec2 texPosition;

void swap4(inout vec4 a, inout vec4 b)
{
  vec4 temp = a;
  a = b;
  b = temp;
}

void swap3(inout vec3 a, inout vec3 b)
{
  vec3 temp = a;
  a = b;
  b = temp;
}

void outputVertex(vec4 nfront, vec4 nback, vec3 ifront, vec3 iback)
{
  one_over_w_front = nfront.w;
  one_over_w_back  = nback.w;
  integrand_over_w_front = ifront * nfront.w;
  integrand_over_w_back = iback * nback.w;
  gl_Position = vec4(nfront.xy, 0.0, 1.0);
  texPosition = (nfront.xy + 1.0) / 2.0;
  EmitVertex();
}

void outputSimpleVertex(vec4 n, vec3 i)
{
  one_over_w_front = one_over_w_back  = n.w;
  integrand_over_w_front = integrand_over_w_back = i * n.w;
  gl_Position = vec4(n.xy, 0.0, 1.0);
  texPosition = (n.xy + 1.0) / 2.0;
  EmitVertex();
}

void good_case(vec4 n0, vec4 n1, vec4 n2, vec4 n3,
               vec3 i0, vec3 i1, vec3 i2, vec3 i3)
{
  // We need the point on the big face that intersects the middle point
  // in screen cooordinates.  To do this, we need to solve the following
  // system of equations in the xy plane:
  // n0 = n1 + t (n2 - n1) + u (n3 - n1)
  // Rearranging, we get:
  // (n2 - n1) t + (n3 - n1) u = n0 - n1
  // Which we write abstractly as:
  // A * X = B, where
  mat2 A = mat2((n2 - n1).xy, (n3 - n1).xy);
  vec2 B = (n0 - n1).xy;
  vec2 X = inverse(A) * B;
  float t = X[0];
  float u = X[1];
  vec4 weighted_x1 = n1 * (1 - t - u);
  vec4 weighted_x2 = n2 * t;
  vec4 weighted_x3 = n3 * u;
  vec4 ny = weighted_x1 + weighted_x2 + weighted_x3;
  vec3 iy = weighted_x1.w * i1 + weighted_x2.w * i2 + weighted_x3.w * i3;
  iy /= ny.w;
  if (n0.z > ny.z) {
    swap4(n0, ny);
    swap3(i0, iy);
  }

  // Draw three triangles.
  // x[1] - x[2] - y
  outputSimpleVertex(n1, i1);
  outputSimpleVertex(n2, i2);
  outputVertex(n0, ny, i0, iy);

  // x[2] - y - x[3]
  outputSimpleVertex(n3, i3);

  // y - x[3] - x[1]
  outputSimpleVertex(n1, i1);
  EndPrimitive();
}

void bad_case(vec4 n0, vec4 n1, vec4 n2, vec4 n3,
              vec3 i0, vec3 i1, vec3 i2, vec3 i3)
{
  // Compute the intersection point of segment 01 with segment 23
  // We need to solve this system of equations in the xy plane:
  // n0 + t (n1 - n0) = n2 + u (n3 - n2)
  // which, by algebra, is:
  // (n1 - n0) t + (n2 - n3) u = n2 - n0
  // Which we will write abstractly as:
  // A * X = B
  // Note that doing this calculation with x[].n instead of x[].v
  // will give us correct x, y, z values regardless of w.
  mat2 A = mat2((n1 - n0).xy, (n2 - n3).xy);
  vec2 B = (n2 - n0).xy;
  vec2 X = inverse(A) * B; // NOTE: need to handle ill-conditioned case
  float t = X[0];
  float u = X[1];

  // We get two answers; one is the point on the front edge which
  // appears to intersect the back edge in screen coordinates, and
  // the other is the point on the back edge which appears to inter-
  // sect the front edge in screen coordinates.  We don't know which
  // is which, but we can tell the difference by looking at the z
  // values.
  // The two intersection points are:
  vec4 weighted_x0 = n0 * (1 - t);
  vec4 weighted_x1 = n1 * t;
  vec4 ny0 = weighted_x0 + weighted_x1;
  vec3 iy0 = weighted_x0.w * i0 + weighted_x1.w * i1;
  iy0 /= ny0.w;

  vec4 weighted_x2 = n2 * (1 - u);
  vec4 weighted_x3 = n3 * u;
  vec4 ny1 = weighted_x2 + weighted_x3;
  vec3 iy1 = weighted_x2.w * i2 + weighted_x3.w * i3;
  iy1 /= ny1.w;

  // Make sure y0 is front and y1 is back
  if (ny0.z > ny1.z) {
    swap4(ny0, ny1);
    swap3(iy0, iy1);
  }

  // Draw four triangles.
  // x[0] - x[2] - y
  outputSimpleVertex(n0, i0);
  outputSimpleVertex(n2, i2);
  outputVertex(ny0, ny1, iy0, iy1);

  // x[2] - y - x[1]
  outputSimpleVertex(n1, i1);

  // A null triangle: y - x[1] - y
  outputVertex(ny0, ny1, iy0, iy1);

  // x[1] - y - x[3]
  outputSimpleVertex(n3, i3);

  // y - x[3] - x[0]
  outputSimpleVertex(n0, i0);
  EndPrimitive();
}

// Return +/- 1, depending on the orientation of triangle a -> b -> c
// in the xy plane (ignoring z and w)
float triangle_orientation(vec4 a, vec4 b, vec4 c)
{
  vec2 ab = b.xy - a.xy;
  vec2 ac = c.xy - a.xy;
  return sign(ab.x * ac.y - ab.y * ac.x);
}

void handle_tetrahedron(vec4 n0, vec4 n1, vec4 n2, vec4 n3,
                        vec3 i0, vec3 i1, vec3 i2, vec3 i3)
{
  // For each face of the tetrahedron, an orientation on that face
  // is induced by the orientation of the input tetrahedron.  Compute
  // whether those orientations are equal to or opposite from the
  // orientation in screen coordinates.
  // The orientation will be the same for front faces and reversed
  // for back faces (or vice versa, depending on whether the
  // tetrahedron is left or right handed).
  // Surprisingly, we DON'T need to know the handedness of the input
  // tetrahedron for our rendering calculations!
  // The induced orientations are:
  // Face 0: 3 -> 2 -> 1
  float orient0 = triangle_orientation(n3, n2, n1);
  // Face 1: 0 -> 2 -> 3
  float orient1 = triangle_orientation(n0, n2, n3);
  // Face 2: 3 -> 1 -> 0
  float orient2 = triangle_orientation(n3, n1, n0);
  // Face 3: 0 -> 1 -> 2
  float orient3 = triangle_orientation(n0, n1, n2);

  float orient01 = orient0 * orient1;
  float orient23 = orient2 * orient3;
  float orient = orient01 * orient23;

  // For now, discard tetrahedra where a face is seen exactly edge-on.
  // The sign() function should return 0.0 in that case, but exercise
  // an abundance of caution with floating point math.
  if (orient > -0.5 && orient < 0.5)
    return;

  // We call a tetrahedron good if its projection onto the canvas
  // has a triangular convex hull with one vertex in the interior.
  // It is conversely bad if the convex hull is a quadrilateral.
  // A tetrahedron is good iff an odd number of faces have flipped
  // orientation, i.e. there are either three front and one rear
  // faces or vice versa.
  bool good = orient < 0.;

  if (good) {
    // The vertex in the center is the one whose orientN value is
    // of a different sign.  Figure out the majority sign and which
    // vertex is in the middle.
    if (orient01 > 0.) {
      // 0 and 1 have the same sign; either 2 or 3 is opposite
      float majority = orient0;
      if (orient2 * majority < 0.) {
        swap4(n0, n2);
        swap3(i0, i2);
      } else {
        swap4(n0, n3);
        swap3(i0, i3);
      }
    } else {
      // 2 and 3 have the same sign; either 0 or 1 is opposite
      float majority = orient2;
      if (orient0 * majority < 0.)
        ;
      else {
        swap4(n0, n1);
        swap3(i0, i1);
      }
    }

    good_case(n0, n1, n2, n3,
              i0, i1, i2, i3);

  } else {
    float orient02 = orient0 * orient2;

    // We assume here that two of the orientations are positive, and
    // two are negative.  The other possibility, that all four have
    // the same sign, would indicate an error; it's unclear if that
    // can happen at all, but it's probably worth checking for.
    if (orient01 > 0. && orient23 > 0. && orient02 > 0.)
         return;

    // Reorder so that vertices 0 and 1 have the same sign
    if (orient01 > 0.)
      // 0 and 1 already match, do nothng
      ;
    else if (orient02 > 0.) {
      // 0 and 2 match, so swap 1 with 2
      swap4(n1, n2);
      swap3(i1, i2);
    } else {
      // 0 and 3 match, so swap 1 with 3
      swap4(n1, n3);
      swap3(i1, i3);
    }

    bad_case(n0, n1, n2, n3, i0, i1, i2, i3);
  }
}

vec4 vector_inverse(vec4 a)
{
  float w = a.w;
  a.w = 1.0;
  a /= w;
  return a;
}

void intersect_nearclip(out vec4 nc, out vec3 ic,
                        vec4 px, vec3 ix, vec4 py, vec3 iy)
{
  float near = nearfar[0];
  float t = (near - py.w) / (px.w - py.w);
  nc = vector_inverse(t * px + (1-t) * py);
  ic = t * ix + (1-t) * iy;
}

void main(void)
{
  vec4 p0 = gl_in[0].gl_Position;
  vec4 p1 = gl_in[1].gl_Position;
  vec4 p2 = gl_in[2].gl_Position;
  vec4 p3 = gl_in[3].gl_Position;

  // Number of vertices clipped by the near plane
  int num_clipped = 0;

  // Count how many vertices are clipped
  float near = nearfar[0];
  if (p0.w < near)
    ++num_clipped;
  if (p1.w < near)
    ++num_clipped;
  if (p2.w < near)
    ++num_clipped;
  if (p3.w < near)
    ++num_clipped;

  if (num_clipped == 4)
    return;

  vec4 n0 = inverted_position[0];
  vec4 n1 = inverted_position[1];
  vec4 n2 = inverted_position[2];
  vec4 n3 = inverted_position[3];

  vec3 i0 = integrand[0];
  vec3 i1 = integrand[1];
  vec3 i2 = integrand[2];
  vec3 i3 = integrand[3];

  if (num_clipped == 0) {
    handle_tetrahedron(n0, n1, n2, n3, i0, i1, i2, i3);
    return;
  }

  // Handle the near clipping plane. This is the "slow" execution path.

  // Move clipped vertices to the front of the array using a "sorting network"
  if (p0.w >= near && p2.w < near) {
    swap4(p0, p2);
    swap4(n0, n2);
    swap3(i0, i2);
  }
  if (p1.w >= near && p3.w < near) {
    swap4(p1, p3);
    swap4(n1, n3);
    swap3(i1, i3);
  }
  if (p0.w >= near && p1.w < near) {
    swap4(p0, p1);
    swap4(n0, n1);
    swap3(i0, i1);
  }
  if (p2.w >= near && p3.w < near) {
    swap4(p2, p3);
    swap4(n2, n3);
    swap3(i2, i3);
  }
  if (p1.w >= near && p2.w < near) {
    swap4(p1, p2);
    swap4(n1, n2);
    swap3(i1, i2);
  }

  switch (num_clipped) {

  case 1:

    {
      // One vertex clipped:
      // Must determine the intersections of the 0-to-i segments
      // with the near clipping plane, and construct three new tetrahedra
      // to render.

      vec4 n03;
      vec3 i03;
      intersect_nearclip(n03, i03, p0, i0, p3, i3);
      handle_tetrahedron(n1, n2, n3, n03,
                         i1, i2, i3, i03);

      vec4 n02;
      vec3 i02;
      intersect_nearclip(n02, i02, p0, i0, p2, i2);
      handle_tetrahedron(n1, n2, n02, n03,
                         i1, i2, i02, i03);

      vec4 n01;
      vec3 i01;
      intersect_nearclip(n01, i01, p0, i0, p1, i1);
      handle_tetrahedron(n1, n01, n02, n03,
                         i1, i01, i02, i03);
    }

    return;

  case 2:

    {
      // Two vertices clipped:
      // Must determine the intersections of the (0,1)-to-(2,3) segments
      // with the near clipping plane, and construct three new tetrahedra
      // to render.
      vec4 n02;
      vec3 i02;
      intersect_nearclip(n02, i02, p0, i0, p2, i2);

      vec4 n12;
      vec3 i12;
      intersect_nearclip(n12, i12, p1, i1, p2, i2);

      handle_tetrahedron(n2, n3, n02, n12,
                         i2, i3, i02, i12);

      vec4 n03;
      vec3 i03;
      intersect_nearclip(n03, i03, p0, i0, p3, i3);

      handle_tetrahedron(n3, n02, n12, n03,
                         i3, i02, i12, i03);

      vec4 n13;
      vec3 i13;
      intersect_nearclip(n13, i13, p1, i1, p3, i3);

      handle_tetrahedron(n3, n12, n03, n13,
                         i3, i12, i03, i13);
    }

    return;

  case 3:

    // Three vertices clipped:
    // Must replace 0-2 with their projections toward 3 at
    // the point they intersect the near clipping plane.
    vec4 n03;
    vec3 i03;
    intersect_nearclip(n03, i03, p0, i0, p3, i3);

    vec4 n13;
    vec3 i13;
    intersect_nearclip(n13, i13, p1, i1, p3, i3);

    vec4 n23;
    vec3 i23;
    intersect_nearclip(n23, i23, p2, i2, p3, i3);

    handle_tetrahedron(n03, n13, n23, n3,
                       i03, i13, i23, i3);

    return;

  default:

    return;

  }
}
