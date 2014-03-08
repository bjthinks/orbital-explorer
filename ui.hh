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

#ifndef UI_HH
#define UI_HH

#include <sstream>
#include <cstdio>

#include "widget.hh"

template <typename T>
class ParameterDisplayer : public Composite
{
public:
  ParameterDisplayer(Container &e, ParameterReader<T> pr_, const Font &font)
    : Composite(e), pr(pr_), str(*this, font)
  {
    str.justify(CenterJustified);
  }
  void draw(Region r)
  {
    ostringstream ss;
    ss << pr;
    str.set(ss.str());
    str.color(green);
    Composite::draw(r);
  }
  void resize(int width, int height)
  {
    str.resize(width, height);
    Composite::resize(width, height);
  }

private:
  ParameterReader<T> pr;
  String str;
};

template <typename T>
class ParameterIncrementer : public Composite
{
public:
  ParameterIncrementer(Container &e, RangedParameterController<T> pc_)
    : Composite(e), pc(pc_), tri(*this)
  {}
  void draw(Region r)
  {
    tri.x(Vector2(0, 0));
    tri.y(Vector2(geometry.width, 0));
    tri.z(Vector2(geometry.width / 2.0, geometry.height));
    if (pc == pc.max())
      tri.color(grey);
    else
      tri.color(white);
    Composite::draw(r);
  }
  bool handleClick(const Click &c)
  {
    pc = pc + 1;
    return true;
  }

private:
  RangedParameterController<T> pc;
  Triangle tri;
};

template <typename T>
class ParameterDecrementer : public Composite
{
public:
  ParameterDecrementer(Container &e, RangedParameterController<T> pc_)
    : Composite(e), pc(pc_), tri(*this)
  {}
  void draw(Region r)
  {
    tri.x(Vector2(0, geometry.height));
    tri.y(Vector2(geometry.width, geometry.height));
    tri.z(Vector2(geometry.width / 2.0, 0));
    if (pc == pc.min())
      tri.color(grey);
    else
      tri.color(white);
    Composite::draw(r);
  }
  bool handleClick(const Click &c)
  {
    pc = pc - 1;
    return true;
  }

private:
  RangedParameterController<T> pc;
  Triangle tri;
};

template <typename T>
class ParameterWidget : public Composite
{
public:
  ParameterWidget(Container &e,
                  RangedParameterController<T> pc,
                  const Font &font)
    : Composite(e),
      disp(*this, pc, font),
      inc(*this, pc),
      dec(*this, pc),
      font_height(font.pointSize())
  {}
  void resize(int width, int height)
  {
    Composite::resize(width, height);

    int quarter_width = width / 4;
    int font_space = font_height * 9 / 8;
    int arrow_height = (height - font_space) / 2;

    dec.move(quarter_width, 0);
    dec.resize(width - 2 * quarter_width, arrow_height);
    disp.move(0, arrow_height);
    disp.resize(width, font_height);
    inc.move(quarter_width, arrow_height + font_space);
    inc.resize(width - 2 * quarter_width, arrow_height);
  }

private:
  ParameterDisplayer<T> disp;
  ParameterIncrementer<T> inc;
  ParameterDecrementer<T> dec;
  int font_height;
};

#endif
