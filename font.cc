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

Font::Font(int points)
{
  if (!initialized) {
    int error = FT_Init_FreeType(&library);
    if (error)
      throw "Could not init Freetype 2";
    initialized = true;
  }

  int error = FT_New_Memory_Face(library, font_data, font_data_size,
                                 0, &face);
  if (error == FT_Err_Unknown_File_Format)
    throw "Unknown font file format";
  else if (error)
    throw "Can\'t read font file";

  error = FT_Set_Char_Size(face, 0, points * 64, 72, 72);
  if (error)
    throw "Could not set font size";

  // Determine some size data
  minLeft = 0;
  maxRight = 0;
  maxWidth = 0;
  minBottom = 0;
  maxTop = 0;
  maxHeight = 0;
  maxRightMinusAdvance = 0;
  for (int ch = 0; ch < 128; ++ch) {
    setGlyph(ch);
    if (getGlyphLeft() < minLeft)
      minLeft = getGlyphLeft();
    if (getGlyphRight() > maxRight)
      maxRight = getGlyphRight();
    if (getGlyphWidth() > maxWidth)
      maxWidth = getGlyphWidth();
    if (getGlyphBottom() < minBottom)
      minBottom = getGlyphBottom();
    if (getGlyphTop() > maxTop)
      maxTop = getGlyphTop();
    if (getGlyphHeight() > maxHeight)
      maxHeight = getGlyphHeight();
    int rma = getGlyphRight() - getGlyphAdvance();
    if (rma > maxRightMinusAdvance)
      maxRightMinusAdvance = rma;
  }
  cellWidth_ = maxRight - minLeft;
  cellHeight_ = maxTop - minBottom;

  pixelData.resize(cellWidth() * cellHeight() * 128, 0);
  advanceData.resize(128);

  for (int ch = 0; ch < 128; ++ch) {
    setGlyph(ch);
    const FT_Bitmap &bitmap = face->glyph->bitmap;
    for (int bitmap_row = 0; bitmap_row < bitmap.rows; ++bitmap_row) {
      for (int bitmap_col = 0; bitmap_col < bitmap.width; ++bitmap_col) {
        int p = bitmap.buffer[bitmap_row * bitmap.width + bitmap_col];
        pixelRW(ch, maxTop - getGlyphTop() + bitmap_row,
                getGlyphLeft() - minLeft + bitmap_col) = p;
      }
    }
    advanceData.at(ch) = getGlyphAdvance();
  }

  glGenTextures(1, &texture_id);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture_id);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, cellWidth(), 128 * cellHeight(),
               0, GL_RED, GL_UNSIGNED_BYTE, &pixelData[0]);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

Font::~Font()
{
  glDeleteTextures(1, &texture_id);
}

const unsigned char &Font::pixel(int ch, int row, int col) const
{
  if (ch < 0 || ch >= 128)
    throw "ch out of range";
  if (row < 0 || row >= cellHeight())
    throw "row out of range";
  if (col < 0 || col >= cellWidth())
    throw "col out of range";
  return pixelData.at(ch * cellWidth() * cellHeight() +
                      row * cellWidth() + col);
}

unsigned char &Font::pixelRW(int ch, int row, int col)
{
  if (ch < 0 || ch >= 128)
    throw "ch out of range";
  if (row < 0 || row >= cellHeight())
    throw "row out of range";
  if (col < 0 || col >= cellWidth())
    throw "col out of range";
  return pixelData.at(ch * cellWidth() * cellHeight() +
                      row * cellWidth() + col);
}

void Font::setGlyph(int c)
{
  int glyph_index = FT_Get_Char_Index(face, c);

  int error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
  if (error)
    throw "Could not load glyph";

  error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
  if (error)
    throw "Could not render glyph";
}

bool Font::initialized = false;
FT_Library Font::library;
