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

#include "frame.hh"
#include "shaders.hh"
#include "vector.hh"

Program *Triangle::triangleProg = NULL;
VertexArrayObject *Triangle::triangleVAO = NULL;

Triangle::Triangle(Frame *p)
  : Frame(p)
{
  if (triangleProg == NULL) {
    triangleProg = new Program();
    triangleProg->vertexShader(triangleVertexShaderSource);
    triangleProg->fragmentShader(triangleFragmentShaderSource);
    glBindAttribLocation(*triangleProg, 0, "index");
    glBindFragDataLocation(*triangleProg, 0, "fragColor");
    triangleProg->link();

    GetGLError();

    triangleVAO = new VertexArrayObject();
    triangleVAO->bind();
    int verts[3] = { 0, 1, 2 };
    triangleVAO->buffer(GL_ARRAY_BUFFER, verts, sizeof(verts));
    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(0, 1, GL_INT, sizeof(int), NULL);

    GetGLError();
  }
}

Vector<2> Triangle::deviceToWindow(double x0, double x1)
{
  int width = 8;
  int height = 8;
  return Vector2(2.0 * x0 / double(width) - 1.0,
                 2.0 * x1 / double(height) - 1.0);
}

void Triangle::x(double x0, double x1)
{
  xx = deviceToWindow(x0, x1);
}

void Triangle::y(double y0, double y1)
{
  yy = deviceToWindow(y0, y1);
}

void Triangle::z(double z0, double z1)
{
  zz = deviceToWindow(z0, z1);
}

void Triangle::color(Color c)
{
  cc = c;
}

void Triangle::draw()
{
  glViewport(left(), bottom(), 8, 8);
  triangleProg->use();
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);
  triangleVAO->bind();
  triangleProg->uniform<Vector<2> >("x") = xx;
  triangleProg->uniform<Vector<2> >("y") = yy;
  triangleProg->uniform<Vector<2> >("z") = zz;
  triangleProg->uniform<Vector<4> >("color") = cc;
  glDrawArrays(GL_TRIANGLES, 0, 3);

  GetGLError();
}

Quad::Quad(Frame *p)
  : Frame(p), s(this), t(this)
{}

void Quad::x(double x0, double x1)
{
  s.x(x0, x1);
  t.x(x0, x1);
}

void Quad::y(double y0, double y1)
{
  s.y(y0, y1);
}

void Quad::z(double z0, double z1)
{
  s.z(z0, z1);
  t.y(z0, z1);
}

void Quad::w(double w0, double w1)
{
  t.z(w0, w1);
}
