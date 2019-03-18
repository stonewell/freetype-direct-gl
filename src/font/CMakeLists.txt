SET(font_src
  font_manager.h font_manager.cxx
  font.h
  font_impl.h font_impl.cxx
  glyph.h
  glyph_impl.h glyph_impl.cxx
  glyph_compiler.h glyph_compiler.cxx
  cu2qu.cxx cu2qu.h
)

ADD_LIBRARY(font OBJECT
  ${font_src}
)

TARGET_INCLUDE_DIRECTORIES(font PRIVATE
  ${FREETYPE_INCLUDE_DIRS}
  ${FONTCONFIG_INCLUDE_DIR}
  "../utils"
)
