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

#ifndef WIDGET_HH
#define WIDGET_HH

#include <list>
#include <limits>

#include "util.hh"
#include "oopengl.hh"
#include "vector.hh"
#include "color.hh"
#include "font.hh"
#include "shaders.hh"
#include "event.hh"

// Small aggregate to encapsulate rectangular regions.

struct Region
{
  Region()
    : left(0), bottom(0),
      width(std::numeric_limits<int>::max()),
      height(std::numeric_limits<int>::max())
  {}
  Region(int new_left, int new_bottom, int new_width, int new_height)
    : left(new_left), bottom(new_bottom), width(new_width), height(new_height)
  {}
  // Compose two Regions: inner = outer * inner_relative_to_outer
  Region operator*(const Region &rhs)
  {
    return Region(left + rhs.left, bottom + rhs.bottom,
                  std::max(0, std::min(rhs.width, width - rhs.left)),
                  std::max(0, std::min(rhs.height, height - rhs.bottom)));
  }
  int left, bottom, width, height;
};

inline bool isEventInsideRegion(const PositionedEvent &e, const Region &r)
{
  if (e.x() >= r.left   && e.x() < r.left   + r.width &&
      e.y() >= r.bottom && e.y() < r.bottom + r.height)
    return true;
  else
    return false;
}

// Time to make a fun class hierarchy!
// The base class is Widget, which can be drawn, can handle events,
// and has a position and size.

class Widget : public Uncopyable, public Handler
{
public:
  virtual ~Widget() {}
  virtual void draw(Region r) = 0;
  virtual bool handle(Region r, const PositionedEvent &e) = 0;
  void move(int x, int y)
  {
    geometry.left = x;
    geometry.bottom = y;
  }
  void resize(int width, int height)
  {
    geometry.width = width;
    geometry.height = height;
  }

protected:
  Region geometry;
};

// A Container is a Widget that can contain other Widgets.
// An Element is a Widget that goes in a Container.

class Contained;

class Container : virtual public Widget
{
public:
  ~Container();
  void draw(Region r);
  bool handle(Region r, const PositionedEvent &e);

  // Called only by constructors and destructors of Contained widgets
  void addContents(Contained *c);
  void deleteContents(Contained *c);

private:
  std::list<Contained *> contents;
};

// All but top-level Widgets will go in a Container. They are Contained.

class Contained : virtual public Widget
{
public:
  Contained(Container &e);
  ~Contained();

private:
  Container &enclosure;
};

// A Widget that's both a Container and an Contained is a Composite.

class Composite : public Container, public Contained
{
public:
  Composite(Container &e);
};

// Might also define a top-level Container here...

// A Widget that's only a Contained is an Element.

class Element : public Contained
{
public:
  Element(Container &e);
  bool handle(Region r, const PositionedEvent &e);
};

// Triangles are a basic drawing primitive.

class Triangle : public Element
{
public:
  Triangle(Container &e);
  Triangle &x(Vector<2> x_);
  Triangle &y(Vector<2> y_);
  Triangle &z(Vector<2> z_);
  Triangle &color(Color c);
  void draw(Region r);

private:
  static Program *triangleProg;
  static VertexArrayObject *triangleVAO;
  Vector<2> xx, yy, zz;
  Color cc;
};

// Quads are implemented as a pair of triangles.

class Quad : public Composite
{
public:
  Quad(Container &e);
  Quad &x(Vector<2> x_);
  Quad &y(Vector<2> z_);
  Quad &z(Vector<2> y_);
  Quad &w(Vector<2> w_);
  Quad &color(Color c);

private:
  Triangle s, t;
};

// A Rectangle is a convenience wrapper around a Quad.

class Rectangle : public Composite
{
public:
  Rectangle(Container &e);
  Rectangle &ll(Vector<2> llcorner);
  Rectangle &ur(Vector<2> urcorner);
  Rectangle &color(Color c);

private:
  Vector<2> llc, urc;
  Quad q;
};

// A Box is a hollow rectangle

class Box : public Composite
{
public:
  Box(Container &e);
  Box &ll(Vector<2> llcorner);
  Box &ur(Vector<2> urcorner);
  Box &width(double width_);
  Box &color(Color c);

private:
  Vector<2> llc, urc;
  double wid;
  Rectangle left, right, bottom, top;
  void setCoords();
};

// An antialiased 7-bit ASCII character

class Character : public Element
{
public:
  Character(Container &e, Font &f);
  Character &point(Vector<2> p);
  Character &set(char c);
  Character &color(Color c);
  void draw(Region r);
  int advance();

private:
  static Program *characterProg;
  static VertexArrayObject *characterVAO;
  Font &font;
  Vector<2> pp;
  Color cc;
  char ch;
};

// A character string - note absence of line wrapping

class String : public Composite
{
public:
  String(Container &e, Font &f);
  ~String();
  void point(Vector<2> p);
  void set(const std::string &s);
  void color(Color c);

private:
  Font &font;
  std::vector<Character *> str;
  Vector<2> pp;
  Color cc;

  void setCharacterPoints();
  void setCharacterColors();
};

//
// Implementations of "simple" functions follow.
//

inline Container::~Container()
{
  if (contents.size() != 0)
    throw std::logic_error("Container destructed but has contents");
}

inline void Container::draw(Region r)
{
  for (std::list<Contained *>::iterator i = contents.begin();
       i != contents.end(); ++i)
    (*i)->draw(r * geometry);
}

inline bool Container::handle(Region r, const PositionedEvent &e)
{
  r = r * geometry;

  if (!isEventInsideRegion(e, r))
    return false;

  if (e.dispatchTo(*this))
    return true;

  for (std::list<Contained *>::iterator i = contents.begin();
       i != contents.end(); ++i) {
    if ((*i)->handle(r, e))
      return true;
  }

  return false;
}

inline void Container::addContents(Contained *c)
{
  contents.push_front(c);
}

inline void Container::deleteContents(Contained *c)
{
  contents.remove(c);
}

inline Contained::Contained(Container &e)
  : enclosure(e)
{
  enclosure.addContents(this);
}

inline Contained::~Contained()
{
  enclosure.deleteContents(this);
}

inline Composite::Composite(Container &e)
  : Contained(e)
{}

inline Element::Element(Container &e)
  : Contained(e)
{}

inline bool Element::handle(Region r, const PositionedEvent &e)
{
  r = r * geometry;

  if (!isEventInsideRegion(e, r))
    return false;

  return e.dispatchTo(*this);
}

inline Triangle &Triangle::x(Vector<2> x_)
{
  xx = x_;
  return *this;
}

inline Triangle &Triangle::y(Vector<2> y_)
{
  yy = y_;
  return *this;
}

inline Triangle &Triangle::z(Vector<2> z_)
{
  zz = z_;
  return *this;
}

inline Triangle &Triangle::color(Color c)
{
  cc = c;
  return *this;
}

inline Quad::Quad(Container &e)
  : Composite(e),
    s(*this),
    t(*this)
{}

inline Quad &Quad::x(Vector<2> x_)
{
  s.x(x_);
  t.x(x_);
  return *this;
}

inline Quad &Quad::y(Vector<2> y_)
{
  s.y(y_);
  return *this;
}

inline Quad &Quad::z(Vector<2> z_)
{
  s.z(z_);
  t.y(z_);
  return *this;
}

inline Quad &Quad::w(Vector<2> w_)
{
  t.z(w_);
  return *this;
}

inline Quad &Quad::color(Color c)
{
  s.color(c);
  t.color(c);
  return *this;
}

inline Rectangle::Rectangle(Container &e)
  : Composite(e),
    llc(Vector2(0, 0)),
    urc(Vector2(0, 0)),
    q(*this)
{}

inline Rectangle &Rectangle::ll(Vector<2> llcorner)
{
  llc = llcorner;
  q .x(Vector2(llc[0], llc[1]))
    .y(Vector2(urc[0], llc[1]))
    .w(Vector2(llc[0], urc[1]));
  return *this;
}

inline Rectangle &Rectangle::ur(Vector<2> urcorner)
{
  urc = urcorner;
  q .y(Vector2(urc[0], llc[1]))
    .z(Vector2(urc[0], urc[1]))
    .w(Vector2(llc[0], urc[1]));
  return *this;
}

inline Rectangle &Rectangle::color(Color c)
{
  q.color(c);
  return *this;
}

inline Box::Box(Container &e)
  : Composite(e),
    llc(Vector2(0, 0)),
    urc(Vector2(0, 0)),
    wid(0),
    left(*this),
    right(*this),
    bottom(*this),
    top(*this)
{}

inline void Box::setCoords()
{
  left  .ll(llc).ur(Vector2(llc[0]+wid, urc[1]));
  bottom.ll(llc).ur(Vector2(urc[0], llc[1]+wid));
  right .ll(Vector2(urc[0]-wid, llc[1])).ur(urc);
  top   .ll(Vector2(llc[0], urc[1]-wid)).ur(urc);
}

inline Box &Box::ll(Vector<2> llcorner)
{
  llc = llcorner;
  setCoords();
  return *this;
}

inline Box &Box::ur(Vector<2> urcorner)
{
  urc = urcorner;
  setCoords();
  return *this;
}

inline Box &Box::width(double width_)
{
  wid = width_;
  setCoords();
  return *this;
}

inline Box &Box::color(Color c)
{
  left.color(c);
  right.color(c);
  bottom.color(c);
  top.color(c);
  return *this;
}

inline Character &Character::point(Vector<2> p)
{
  pp = p;
  return *this;
}

inline Character &Character::set(char c)
{
  ch = c;
  return *this;
}

inline Character &Character::color(Color c)
{
  cc = c;
  return *this;
}

inline int Character::advance()
{
  return font.advance(ch);
}

inline String::String(Container &e, Font &f)
  : Composite(e),
    font(f)
{}

inline String::~String()
{
  for (int i = 0; i < int(str.size()); ++i)
    delete str[i];
}

inline void String::point(Vector<2> p)
{
  pp = p;
  setCharacterPoints();
}

inline void String::set(const std::string &s)
{
  for (int i = 0; i < int(str.size()); ++i)
    delete str[i];
  str.resize(0);
  for (int i = 0; i < int(s.size()); ++i) {
    Character *c = new Character(*this, font);
    c->set(s[i]);
    str.push_back(c);
  }
  setCharacterPoints();
  setCharacterColors();
}

inline void String::color(Color c)
{
  cc = c;
  setCharacterColors();
}

inline void String::setCharacterPoints()
{
  Vector<2> cur = pp;
  for (int i = 0; i < int(str.size()); ++i) {
    str[i]->point(cur);
    cur += Vector2(str[i]->advance(), 0);
  }
}

inline void String::setCharacterColors()
{
  for (int i = 0; i < int(str.size()); ++i)
    str[i]->color(cc);
}

#endif
