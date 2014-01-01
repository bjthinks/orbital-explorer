#include <ft2build.h>
#include FT_FREETYPE_H

int main(int argc, char *argv[])
try {
  if (argc <= 1)
    throw "Usage: demo <character>";

  int error;

  FT_Library library;
  error = FT_Init_FreeType(&library);
  if (error)
    throw "Could not init Freetype 2";

  FT_Face face;
  error = FT_New_Face(library, "SourceSansPro-Regular.ttf", 0, &face);
  if (error == FT_Err_Unknown_File_Format)
    throw "Unknown font file format";
  else if (error)
    throw "Can\'t read font file";

  printf("The font file contains %d face(s)\n", int(face->num_faces));
  printf("Face 0 has %d glyphs\n", int(face->num_glyphs));
  printf("Units per em is %f\n", double(face->units_per_EM));
  printf("There are %d fixed (i.e. pre-rendered) sizes\n",
         int(face->num_fixed_sizes));

  error = FT_Set_Char_Size(face,   // handle to face object
                           0,      // char_width in 1/64th of points
                           24*64,  // char_height in 1/64th of points
                           72,     // horizontal dpi
                           72);    // vertical dpi
  if (error)
    throw "Could not set font size";
  printf("Font size set to 16 point @ 72 dpi\n");

  printf("Kerning is %s\n", FT_HAS_KERNING(face) ? "present" : "absent");

  int glyph_index_A = FT_Get_Char_Index(face, argv[1][0]);
  printf("Glyph index of A is %d\n", glyph_index_A);

  error = FT_Load_Glyph(face,              // handle to face object
                        glyph_index_A,     // glyph index
                        FT_LOAD_DEFAULT);  // load flags
  if (error)
    throw "Could not load glyph of A";

  error = FT_Render_Glyph(face->glyph,             // glyph slot
                          FT_RENDER_MODE_NORMAL);  // render mode
  if (error)
    throw "Could not render A";
  printf("A has been rendered\n");

  printf("left = %d top = %d width = %d height = %d\n",
         face->glyph->bitmap_left, face->glyph->bitmap_top,
         face->glyph->bitmap.width, face->glyph->bitmap.rows);
  printf("advance = %f\n", double(face->glyph->advance.x) / 64.0);
  printf("metrics:\n");
  printf("left = %f top = %f width = %f height = %f\n",
         double(face->glyph->metrics.horiBearingX) / 64.0,
         double(face->glyph->metrics.horiBearingY) / 64.0,
         double(face->glyph->metrics.width) / 64.0,
         double(face->glyph->metrics.height) / 64.0);
  printf("advance = %f\n", double(face->glyph->metrics.horiAdvance) / 64.0);

  FT_Bitmap &bitmap = face->glyph->bitmap;
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
