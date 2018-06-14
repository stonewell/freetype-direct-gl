SET(font_src
  font_manager.h font_manager.cxx
  font.h
)

ADD_LIBRARY(font STATIC ${font_src})
