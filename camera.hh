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

#ifndef CAMERA_HH
#define CAMERA_HH

#include "quaternion.hh"
#include "matrix.hh"
#include "widget.hh"
#include "config.hh"
#include "viewport.hh"

class Camera
{
public:
  Camera() : rotation(1.0), radius(4.0) {}
  Matrix<4,4> viewMatrix() const;
  double getRadius() const { return radius; }
  void rotate(double x, double y);
  void spin(double s);
  void zoom(double f);

private:
  Quaternion rotation;
  double radius;
};

class CameraController : public Element
{
public:
  CameraController(Container &e, Camera &cam, const Viewport &viewport)
    : Element(e), camera(cam), view(viewport)
  {}
  void draw(Region) {}
  bool handleDrag(const Drag &d)
  {
    if (d.buttons() == LeftButton) {
      camera.rotate(double(d.xrel()) / view.getWidth(),
                    double(d.yrel()) / view.getHeight());
      return true;
    }
    if (d.buttons() == RightButton) {
      camera.spin(-double(d.xrel()) / view.getWidth());
      camera.zoom( double(d.yrel()) / view.getHeight());
      return true;
    }
    return false;
  }
  bool handleWheel(const Wheel &w)
  {
    camera.zoom(-DISCRETE_ZOOM_SIZE * w.direction());
    return true;
  }

private:
  Camera &camera;
  const Viewport &view;
};

#endif
