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
#include "font.hh"
#include "shaders.hh"

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

// A Window is a Container which constrains drawing to a smaller area.

class Window : public Container
{
public:
  Window(Container *p, Frameview view_relative_to_parent_window)
    : Container(p),
      relview(view_relative_to_parent_window)
  {}
  Window(Container *p)
    : Container(p),
      relview(Frameview(0, 0, 0, 0))
  {}
  void draw(Frameview outer)
  {
    Container::draw(outer * relview);
  }

private:
  Frameview relview;
};

// Triangles are a basic drawing primitive.

class Triangle : public Frame
{
public:
  Triangle(Container *p);
  void coords(Vector<2> x, Vector<2> y, Vector<2> z);
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
  void coords(Vector<2> x, Vector<2> y, Vector<2> z, Vector<2> w);
  void color(Color c);

private:
  Triangle s, t;
};

// A Rectangle is a convenience wrapper around a Quad

class Rectangle : public Container
{
public:
  Rectangle(Container *p);
  void coords(Vector<2> llcorner, Vector<2> urcorner);
  void color(Color c);

private:
  Quad q;
};

// A Box is a hollow rectangle

class Box : public Container
{
public:
  Box(Container *p);
  void coords(Vector<2> llcorner, Vector<2> urcorner, double width = 1.0);
  void color(Color c);

private:
  Rectangle left, right, bottom, top;
};

// A Border is always drawn just inside the viewport.
// NOTE: This is flawed, because the viewport as received in the draw()
// call is subject to clipping based on any enclosing viewport.

class Border : public Container
{
public:
  Border(Container *p);
  void color(Color c);
  void draw(Frameview view);

private:
  Box box;
};

// An antialiased 7-bit ASCII character

class Character : public Frame
{
public:
  Character(Container *p, Font *f);
  void point(Vector<2> p)
  {
    pp = p;
  }
  void character(char c)
  {
    ch = c;
  }
  void color(Color c)
  {
    cc = c;
  }
  void draw(Frameview view);

private:
  static Program *characterProg;
  static VertexArrayObject *characterVAO;
  Font *font;
  Vector<2> pp;
  Color cc;
  char ch;
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

inline void Triangle::coords(Vector<2> x, Vector<2> y, Vector<2> z)
{
  xx = x;
  yy = y;
  zz = z;
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

inline void Quad::coords(Vector<2> x, Vector<2> y, Vector<2> z, Vector<2> w)
{
  s.coords(x, y, z);
  t.coords(x, z, w);
}

inline void Quad::color(Color c)
{
  s.color(c);
  t.color(c);
}

inline Rectangle::Rectangle(Container *p)
  : Container(p),
    q(this)
{}

inline void Rectangle::coords(Vector<2> llcorner, Vector<2> urcorner)
{
  q.coords(Vector2(llcorner[0], llcorner[1]),
           Vector2(urcorner[0], llcorner[1]),
           Vector2(urcorner[0], urcorner[1]),
           Vector2(llcorner[0], urcorner[1]));
}

inline void Rectangle::color(Color c)
{
  q.color(c);
}

inline Box::Box(Container *p)
  : Container(p),
    left(this), right(this), bottom(this), top(this)
{}

inline void Box::coords(Vector<2> llcorner, Vector<2> urcorner, double width)
{
  left.coords(llcorner, Vector2(llcorner[0]+width, urcorner[1]));
  right.coords(Vector2(urcorner[0]-width, llcorner[1]), urcorner);
  bottom.coords(llcorner, Vector2(urcorner[0], llcorner[1]+width));
  top.coords(Vector2(llcorner[0], urcorner[1]-width), urcorner);
}

inline void Box::color(Color c)
{
  left.color(c);
  right.color(c);
  bottom.color(c);
  top.color(c);
}

inline Border::Border(Container *p)
  : Container(p),
    box(this)
{}

inline void Border::color(Color c)
{
  box.color(c);
}

inline void Border::draw(Frameview view)
{
  box.coords(Vector2(0, 0), Vector2(view.width, view.height));
  Container::draw(view);
}

#endif
