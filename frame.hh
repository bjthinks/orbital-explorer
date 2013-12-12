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

// Small aggregate to encapsulate viewports.
// FIXME: Name conflict needs resolution.

struct Frameview
{
  Frameview(int new_left, int new_bottom, int new_width, int new_height)
    : left(new_left), bottom(new_bottom), width(new_width), height(new_height)
  {}
  // Compose two viewports: inner = outer * inner_relative_to_outer
  Frameview operator*(const Frameview &rhs)
  {
    return Frameview(left + rhs.left, bottom + rhs.bottom,
                     std::max(0, std::min(rhs.width, width - rhs.left)),
                     std::max(0, std::min(rhs.height, height - rhs.bottom)));
  }
  int left, bottom, width, height;
};

// Time to make a fun class hierarchy!

// (Forward declarations)
class Container;

// A Frame may have a parent, which is a Container.
// All Frames respond to being drawn.

class Frame : public Uncopyable
{
public:
  Frame(Container *p);
  virtual ~Frame();
  virtual void draw(Frameview view) = 0;

protected:
  Container *parent;
};

// A Container is a Frame which groups together its children.

class Container : public Frame
{
public:
  Container(Container *p);
  ~Container();
  void addChild(Frame *c);
  void deleteChild(Frame *c);
  void draw(Frameview view);

private:
  std::list<Frame *> children;
};

// Triangles are the basic drawing primitive.

class Triangle : public Frame
{
public:
  Triangle(Container *p);
  void x(Vector<2> v);
  void y(Vector<2> v);
  void z(Vector<2> v);
  void color(Color c);
  void draw(Frameview view);

private:
  static Program *triangleProg;
  static VertexArrayObject *triangleVAO;
  Vector<2> xx, yy, zz;
  Color cc;
};

// Quads are implemented as a pair of triangles.

class Quad : public Container
{
public:
  Quad(Container *p);
  void x(Vector<2> v);
  void y(Vector<2> v);
  void z(Vector<2> v);
  void w(Vector<2> v);
  void color(Color c);

private:
  Triangle s, t;
};

// Implementations of "simple" functions follow.

inline Frame::Frame(Container *p)
  : parent(p)
{
  if (parent)
    parent->addChild(this);
}

inline Frame::~Frame()
{
  if (parent)
    parent->deleteChild(this);
}

inline Container::Container(Container *p)
  : Frame(p)
{}

inline Container::~Container()
{
  if (children.size() != 0)
    throw std::logic_error("Container destructed but has children");
}

inline void Container::addChild(Frame *c)
{
  children.push_front(c);
}

inline void Container::deleteChild(Frame *c)
{
  children.remove(c);
}

inline void Container::draw(Frameview view)
{
  for (std::list<Frame *>::iterator i = children.begin();
       i != children.end(); ++i)
    (*i)->draw(view);
}

inline void Triangle::x(Vector<2> v)
{
  xx = v;
}

inline void Triangle::y(Vector<2> v)
{
  yy = v;
}

inline void Triangle::z(Vector<2> v)
{
  zz = v;
}

inline void Triangle::color(Color c)
{
  cc = c;
}

inline Quad::Quad(Container *p)
  : Container(p),
    s(this),
    t(this)
{}

inline void Quad::x(Vector<2> v)
{
  s.x(v);
  t.x(v);
}

inline void Quad::y(Vector<2> v)
{
  s.y(v);
}

inline void Quad::z(Vector<2> v)
{
  s.z(v);
  t.y(v);
}

inline void Quad::w(Vector<2> v)
{
  t.z(v);
}

inline void Quad::color(Color c)
{
  s.color(c);
  t.color(c);
}

/*
// A Window is a Container which restricts the viewable area.

class Window : public Container
{
public:
  Window(Window *p)
    : Frame(p),
      left_offset(0),
      bottom_offset(0),
      my_width(0),
      my_height(0)
  {
    if (parent) {
      my_width = parent->width();
      my_height = parent->height();
    }
  }
  ~Window()
  {
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
  void resize(int new_width, int new_height)
  {
    my_width = new_width;
    my_height = new_height;
  }
  int width()
  {
    return my_width;
  }
  int height()
  {
    return my_height;
  }

protected:

private:
  int left_offset;
  int bottom_offset;
  int my_width;
  int my_height;
};
*/

#endif
