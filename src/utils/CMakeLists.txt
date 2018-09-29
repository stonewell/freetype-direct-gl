SET(utils_src
  memory_buffer.h memory_buffer.cxx
  err_msg.h err_msg.cxx
  shader.h shader.cxx
  program.h program_text_buffer.cxx program_render.cxx program.cxx
  program_render_background.cxx
  char_width.h char_width.cxx
)

ADD_LIBRARY(utils OBJECT ${utils_src})

TARGET_INCLUDE_DIRECTORIES(utils PRIVATE
  ${Boost_INCLUDE_DIRS}
  ${FREETYPE_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIRS}
)
