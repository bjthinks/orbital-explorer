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

Triangle::Triangle(Container *p)
  : Frame(p),
    xx(Vector2(0, 0)),
    yy(Vector2(0, 0)),
    zz(Vector2(0, 0)),
    cc(clear)
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

static Vector<2> deviceToWindow(Frameview view, Vector<2> v)
{
  return Vector2(2.0 * v[0] / double(view.width) - 1.0,
                 2.0 * v[1] / double(view.height) - 1.0);
}

void Triangle::draw(Frameview view)
{
  glViewport(view.left, view.bottom, view.width, view.height);
  triangleProg->use();
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);
  triangleVAO->bind();
  triangleProg->uniform<Vector<2> >("x") = deviceToWindow(view, xx);
  triangleProg->uniform<Vector<2> >("y") = deviceToWindow(view, yy);
  triangleProg->uniform<Vector<2> >("z") = deviceToWindow(view, zz);
  triangleProg->uniform<Vector<4> >("color") = cc;
  glDrawArrays(GL_TRIANGLES, 0, 3);

  GetGLError();
}
