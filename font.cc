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

#include <ft2build.h>
#include FT_FREETYPE_H

#include "font_data.hh"

// Note: kerning is not done, because our chosen
// font (Source Sans Pro) has no kerning data

class Font
{
public:
  Font(int pixels)
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

    error = FT_Set_Char_Size(face, 0, pixels * 64, 72, 72);
    if (error)
      throw "Could not set font size";
  }

  FT_Face face;

  void setGlyph(int c)
  {
    int glyph_index = FT_Get_Char_Index(face, c);

    int error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
    if (error)
      throw "Could not load glyph";

    error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
    if (error)
      throw "Could not render glyph";
  }

  int getGlyphLeft()
  {
    return face->glyph->bitmap_left;
  }

  int getGlyphTop()
  {
    return face->glyph->bitmap_top;
  }

  int getGlyphWidth()
  {
    return face->glyph->bitmap.width;
  }

  int getGlyphRows()
  {
    return face->glyph->bitmap.rows;
  }

  int getGlyphAdvance()
  {
    return face->glyph->advance.x / 64;
  }

private:
  static bool initialized;
  static FT_Library library;
};
bool Font::initialized = false;
FT_Library Font::library;

int main(int argc, char *argv[])
try {
  if (argc <= 1)
    throw "Usage: font <character>";

  Font font(24);

  font.setGlyph(argv[1][0]);

  printf("left = %d top = %d width = %d height = %d\n",
         font.getGlyphLeft(), font.getGlyphTop(),
         font.getGlyphWidth(), font.getGlyphRows());
  printf("advance = %d\n", font.getGlyphAdvance());

  FT_Bitmap &bitmap = font.face->glyph->bitmap;
  for (int i = 0; i < bitmap.rows; ++i) {
    for (int j = 0; j < bitmap.width; ++j) {
      printf("%3d ", bitmap.buffer[i * bitmap.width + j]);
    }
    printf("\n");
  }

  return 0;
} catch (const char *msg) {
  fprintf(stderr, "%s\n", msg);
  return 1;
}
