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
#include <cctype>

#include "font.hh"
#include "font_data.hh"
#include "util.hh"

Font::Font(int points_)
  : points(points_)
{
  if (!initialized) {
    int error = FT_Init_FreeType(&library);
    if (error)
      FATAL("Could not init Freetype 2");
    initialized = true;
  }

  int error = FT_New_Memory_Face(library, font_data, font_data_size,
                                 0, &face);
  if (error == FT_Err_Unknown_File_Format)
    FATAL("Unknown font file format");
  else if (error)
    FATAL("Can\'t read font file");

  error = FT_Set_Char_Size(face, 0, points * 64, 72, 72);
  if (error)
    FATAL("Could not set font size");

  // Determine some size data
  int minBottom = 0;
  int maxWidth = 0;
  int maxHeight = 0;
  for (int ch = 0; ch < 128; ++ch) {
    setGlyph(ch);
    if (getGlyphBottom() < minBottom)
      minBottom = getGlyphBottom();
    if (getGlyphWidth() > maxWidth)
      maxWidth = getGlyphWidth();
    if (getGlyphHeight() > maxHeight)
      maxHeight = getGlyphHeight();
  }
  blockWidth = maxWidth;
  blockHeight = maxHeight;
  textureWidth = maxWidth;
  textureHeight = maxHeight * 128;
  descender_ = -minBottom;

  pixelData.resize(textureWidth * textureHeight, 0);
  leftData.resize(128);
  widthData.resize(128);
  bottomData.resize(128);
  heightData.resize(128);
  advanceData.resize(128);

  for (int ch = 0; ch < 128; ++ch) {
    setGlyph(ch);
    for (int x = 0; x < getGlyphWidth(); ++x) {
      for (int y = 0; y < getGlyphHeight(); ++y)
        texturePixel(ch, x, y) = glyphPixel(ch, x, y);
    }
    leftData.at(ch) = getGlyphLeft();
    widthData.at(ch) = getGlyphWidth();
    bottomData.at(ch) = getGlyphBottom();
    heightData.at(ch) = getGlyphHeight();
    advanceData.at(ch) = getGlyphAdvance();
  }

  glGenTextures(1, &texture_id);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture_id);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, textureWidth, textureHeight,
               0, GL_RED, GL_UNSIGNED_BYTE, &pixelData[0]);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

Font::~Font()
{
  glDeleteTextures(1, &texture_id);
}

unsigned char &Font::glyphPixel(int ch, int x, int y)
{
  if (ch < 0 || ch >= 128)
    FATAL("ch out of range");
  if (x < 0 || x >= getGlyphWidth())
    FATAL("x out of range");
  if (y < 0 || y >= getGlyphHeight())
    FATAL("y out of range");

  int y_flipped = getGlyphHeight() - 1 - y;
  return face->glyph->bitmap.buffer
    [y_flipped * getGlyphWidth() + x];
}

unsigned char &Font::texturePixel(int ch, int x, int y)
{
  if (ch < 0 || ch >= 128)
    FATAL("ch out of range");
  if (x < 0 || x >= blockWidth)
    FATAL("x out of range");
  if (y < 0 || y >= blockHeight)
    FATAL("y out of range");

  return pixelData.at(ch * blockWidth * blockHeight
                      + y * blockWidth + x);
}

void Font::setGlyph(int c)
{
  int glyph_index = FT_Get_Char_Index(face, c);

  int error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
  if (error)
    FATAL("Could not load glyph");

  error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
  if (error)
    FATAL("Could not render glyph");
}

bool Font::initialized = false;
FT_Library Font::library;
