SET(font_src
  font_manager.h font_manager.cxx
  font.h
  font_impl.h font_impl.cxx
)

ADD_LIBRARY(font STATIC ${font_src})

TARGET_INCLUDE_DIRECTORIES(font PRIVATE
  ${FREETYPE_INCLUDE_DIRS}
)
