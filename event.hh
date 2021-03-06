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

#ifndef EVENT_HH
#define EVENT_HH

#include <iostream>
using namespace std;

class Handler;

class Event
{
public:
  virtual ~Event() {}
  virtual bool dispatchTo(Handler &h) const = 0;
};

class Click;
class Unclick;
class Drag;
class Wheel;

class Handler
{
public:
  virtual ~Handler() {}
  virtual bool handleClick(const Click &c)
  {
    return false;
  }
  virtual bool handleUnclick(const Unclick &c)
  {
    return false;
  }
  virtual bool handleDrag(const Drag &c)
  {
    return false;
  }
  virtual bool handleWheel(const Wheel &c)
  {
    return false;
  }
};

enum Button
{
  NoButton = 0,
  LeftButton = 1,
  MiddleButton = 2,
  RightButton = 4
};

class PositionedEvent : public Event
{
public:
  PositionedEvent(int x_, int y_)
    : xpos(x_),
      ypos(y_)
  {}
  int x() const
  {
    return xpos;
  }
  int y() const
  {
    return ypos;
  }

private:
  int xpos, ypos;
};

class Click : public PositionedEvent
{
public:
  Click(int x_, int y_)
    : PositionedEvent(x_, y_)
  {}
  bool dispatchTo(Handler &h) const
  {
    return h.handleClick(*this);
  }
};

class Unclick : public PositionedEvent
{
public:
  Unclick(int x_, int y_)
    : PositionedEvent(x_, y_)
  {}
  bool dispatchTo(Handler &h) const
  {
    return h.handleUnclick(*this);
  }
};

class Drag : public PositionedEvent
{
public:
  Drag(int x_, int y_, int buttons_, int xrel_, int yrel_)
    : PositionedEvent(x_, y_),
      buts(buttons_),
      xr(xrel_),
      yr(yrel_)
  {}
  int buttons() const
  {
    return buts;
  }
  int xrel() const
  {
    return xr;
  }
  int yrel() const
  {
    return yr;
  }
  bool dispatchTo(Handler &h) const
  {
    return h.handleDrag(*this);
  }

private:
  int buts;
  int xr, yr;
};

class Wheel : public Event
{
public:
  Wheel(int direction_)
    : dir(direction_)
  {}
  int direction() const
  {
    return dir;
  }
  bool dispatchTo(Handler &h) const
  {
    return h.handleWheel(*this);
  }

private:
  int dir;
};

#endif
