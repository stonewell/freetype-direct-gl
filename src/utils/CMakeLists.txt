SET(utils_src
  memory_buffer.h memory_buffer.cxx
  err_msg.h err_msg.cxx
  shader.h shader.cxx
)

ADD_LIBRARY(utils OBJECT ${utils_src})

TARGET_INCLUDE_DIRECTORIES(utils PRIVATE
  ${BOOST_INCLUDE_DIRS}
  ${FREETYPE_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIRS}
)
