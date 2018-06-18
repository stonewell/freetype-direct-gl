SET(render_src
  render.h render.cxx
)

ADD_LIBRARY(render OBJECT ${render_src})
SET_TARGET_PROPERTIES(render PROPERTIES LINKER_LANGUAGE CXX)

TARGET_INCLUDE_DIRECTORIES(render PRIVATE
  "../text"
  "../font"
  "../utils"
  ${FREETYPE_INCLUDE_DIRS}
)
