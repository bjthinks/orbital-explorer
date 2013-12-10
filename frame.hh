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

#ifndef FRAME_HH
#define FRAME_HH

#include <list>

#include "util.hh"
#include "oopengl.hh"
#include "vector.hh"
#include "color.hh"

void initFrames();

class Frame : public Uncopyable
{
public:
  Frame(Frame *p)
    : parent(p),
      left_offset(0),
      bottom_offset(0)
  {
    if (parent) parent->addChild(this);
  }
  ~Frame()
  {
    if (children.size() != 0)
      throw std::logic_error("Frame destroyed but has children");
    if (parent) parent->delChild(this);
  }
  virtual void color(Color c)
  {
    for (std::list<Frame *>::iterator i = children.begin();
         i != children.end(); ++i)
      (*i)->color(c);
  }
  virtual void draw()
  {
    for (std::list<Frame *>::iterator i = children.begin();
         i != children.end(); ++i)
      (*i)->draw();
  }
  void moveto(int new_left, int new_bottom)
  {
    left_offset = new_left;
    bottom_offset = new_bottom;
  }
  int left()
  {
    if (parent)
      return left_offset + parent->left();
    else
      return left_offset;
  }
  int bottom()
  {
    if (parent)
      return bottom_offset + parent->bottom();
    else
      return bottom_offset;
  }

protected:
  void addChild(Frame *c)
  {
    children.push_front(c);
  }
  void delChild(Frame *c)
  {
    children.remove(c);
  }

private:
  Frame *parent;
  std::list<Frame *> children;
  int left_offset;
  int bottom_offset;
};

class Triangle : public Frame
{
public:
  Triangle(Frame *p);
  void x(double x0, double x1);
  void y(double y0, double y1);
  void z(double z0, double z1);
  void color(Color c);
  void draw();

private:
  static Program *triangleProg;
  static VertexArrayObject *triangleVAO;
  Vector<2> xx, yy, zz;
  Color cc;
  Vector<2> deviceToWindow(double x0, double x1);
};

class Quad : public Frame
{
public:
  Quad(Frame *p);
  void x(double x0, double x1);
  void y(double y0, double y1);
  void z(double z0, double z1);
  void w(double w0, double w1);

private:
  Triangle s, t;
};

#endif
