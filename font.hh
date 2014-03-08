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

#ifndef FONT_HH
#define FONT_HH

#include <vector>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "glprocs.hh"
#include "vector.hh"

// Note: kerning is not done, because our chosen
// font (Source Sans Pro) has no kerning data

class Font
{
public:
  Font(int points_);
  ~Font();
  int pointSize() const { return points; }
  int left(int ch)    const { return leftData.at(ch); }
  int width(int ch)   const { return widthData.at(ch); }
  int bottom(int ch)  const { return bottomData.at(ch); }
  int height(int ch)  const { return heightData.at(ch); }
  int advance(int ch) const { return advanceData.at(ch); }
  int descender() const { return descender_; }
  Vector<2> texCoordLL(int ch) const
  {
    return Vector2((ch % chars_per_row) / double(chars_per_row),
                   (ch / chars_per_row) / double(chars_per_col));
  }
  Vector<2> texCoordLR(int ch) const
  {
    return texCoordLL(ch) + Vector2
      (width(ch) / double(textureWidth), 0);
  }
  Vector<2> texCoordUL(int ch) const
  {
    return texCoordLL(ch) + Vector2
      (0, height(ch) / double(textureHeight));
  }
  Vector<2> texCoordUR(int ch) const
  {
    return texCoordLL(ch) + Vector2
      (width(ch) / double(textureWidth), height(ch) / double(textureHeight));
  }

  GLuint getTexture() const { return texture_id; }

private:
  static bool initialized;
  static FT_Library library;
  int points;
  FT_Face face;
  void setGlyph(int c);
  int getGlyphLeft()    { return face->glyph->bitmap_left; }
  int getGlyphWidth()   { return face->glyph->bitmap.width; }
  int getGlyphAdvance() { return face->glyph->advance.x / 64; }
  int getGlyphTop()     { return face->glyph->bitmap_top; }
  int getGlyphHeight()  { return face->glyph->bitmap.rows; }
  int getGlyphBottom()  { return getGlyphTop() - getGlyphHeight(); }

  std::vector<unsigned char> pixelData;
  std::vector<int> leftData, widthData, bottomData, heightData, advanceData;
  int blockWidth, textureWidth, blockHeight, textureHeight;
  int descender_;

  unsigned char &glyphPixel(int ch, int x, int y);
  unsigned char &texturePixel(int ch, int x, int y);

  GLuint texture_id;

  static const int chars_per_row = 8;
  static const int chars_per_col = 16;
};

#endif
