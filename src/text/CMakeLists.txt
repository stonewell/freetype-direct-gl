SET(text_src
  text_buffer.h text_buffer.cxx
  color.h
  pen.h
)

ADD_LIBRARY(text OBJECT ${text_src})
SET_TARGET_PROPERTIES(text PROPERTIES LINKER_LANGUAGE CXX)

TARGET_INCLUDE_DIRECTORIES(text PRIVATE
  "../font"
  "../utils"
  ${FREETYPE_INCLUDE_DIRS}
  ${GLM_INCLUDE_DIR}
)
