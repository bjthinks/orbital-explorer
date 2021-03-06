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

#include <vector>
#include <cstdlib>
#include <cmath>
#include <complex>

#include "glprocs.hh"
#include "render.hh"
#include "vector.hh"
#include "matrix.hh"
#include "transform.hh"
#include "wavefunction.hh"
#include "tetrahedralize.hh"
#include "oopengl.hh"
#include "viewport.hh"
#include "camera.hh"
#include "controls.hh"
#include "solid.hh"
#include "cloud.hh"
#include "final.hh"

using namespace std;

// Textures
static Texture *solidRGBTex, *solidDepthTex, *cloudDensityTex;

// This records the number of primitives, not the number of indices
static unsigned num_tetrahedra;

// The function to visualize
static Orbital *orbital = NULL;

// Subdivision of space into tetrahedra
static TetrahedralSubdivision *ts = NULL;

// Classes representing render stages
static Solid *solid = NULL;
static Cloud *cloud = NULL;
static Final *final = NULL;

void initialize()
{
  solidRGBTex = new Texture(GL_RGB8, GL_RGB);
  solidDepthTex = new Texture(GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT);
  cloudDensityTex = new Texture(GL_RGBA16F, GL_RGBA);

  solid = new Solid(solidRGBTex, solidDepthTex);
  cloud = new Cloud(solidDepthTex, cloudDensityTex);
  final = new Final(solidRGBTex, cloudDensityTex);

  glClearColor(0., 0., 0., 0.);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

  GetGLError();
}

void resizeTextures(const Viewport &viewport)
{
  int width = viewport.getWidth();
  int height = viewport.getHeight();

  // Resize statically-sized textures
  solidRGBTex->resize(width, height);
  solidDepthTex->resize(width, height);
  cloudDensityTex->resize(width, height);

  GetGLError();
}

void display(const Viewport &viewport, const Camera &camera)
{
  static bool need_full_redraw = true;

  int width = viewport.getWidth();
  int height = viewport.getHeight();

  static int num_points = 0;
  bool just_started = false;

  // Are we just starting up, or did the wave function change?
  Orbital newOrbital = getOrbital();
  // Did the detail level change?
  static int saved_detail = 0;
  int detail = getDetail();
  if (!orbital || *orbital != newOrbital || saved_detail != detail) {
    saved_detail = detail;

    // Stop any running thread
    if (ts)
      ts->kill();
    // delete checks for NULL, so we don't have to
    delete orbital;
    delete ts;
    orbital = new Orbital(newOrbital);
    ts = new TetrahedralSubdivision(*orbital, orbital->radius());
    num_points = 0;

    // Golden ratio
    const double phi = (1.0 + sqrt(5.0)) / 2.0;
    // 500, 800, 1300, 2100, 3400, 5500, 8900, 14400, 23300, 37700
    int v = 100 * int(pow(phi, double(detail) + 4.0) / sqrt(5.0) + 0.5);
    ts->runUntil(v);
    just_started = true;
  }

  // Only suck down new vertices and tetrahedra if at least 100 more
  // have been calculated -- because locking the mutex for the time it
  // takes to suck down primitives slows down subdivision substantially
  if ((ts->isRunning() && ts->numVertices() > num_points + 100) ||
      ts->isFinished() || just_started) {
    // Must get indices first, because subdivision may be in progress
    std::vector<unsigned> indices = ts->tetrahedronVertexIndices();
    std::vector<Vector<3> > positions = ts->vertexPositions();
    cloud->setPrimitives(positions, indices, orbital);

    num_points = positions.size();
    num_tetrahedra = indices.size() / 4;
    setVerticesTetrahedra(int(num_points), int(num_tetrahedra));

    need_full_redraw = true;
  }

  GetGLError();

  double near = 1.0;
  double far =
    camera.getRadius() + std::max(1.0, (orbital->radius())) * sqrt(3.0);
  Matrix<4,4> viewMatrix = camera.viewMatrix();
  Matrix<4,4> mvpm = viewport.projMatrix(near, far) * viewMatrix;
  Vector<4> camera_position = inverse(viewMatrix) * basisVector<4>(3);

  static Matrix<4,4> old_mvpm;
  if (mvpm != old_mvpm)
    need_full_redraw = true;
  old_mvpm = mvpm;

  double brightness = pow(1.618, getBrightness());
  if (orbital->square)
    brightness *= brightness;
  static double old_brightness = 0.0;
  if (brightness != old_brightness)
    need_full_redraw = true;
  old_brightness = brightness;

  static int old_width = 0;
  static int old_height = 0;
  if (width != old_width || height != old_height)
    need_full_redraw = true;
  old_width = width;
  old_height = height;

  GetGLError();

  if (need_full_redraw) {
    solid->draw(mvpm, width, height);
    cloud->draw(mvpm, width, height, near, far, camera_position, brightness);
    need_full_redraw = false;
  }
  final->draw(width, height);

  glFinish();

  GetGLError();
}

void cleanup()
{}
