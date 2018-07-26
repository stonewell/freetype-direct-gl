SET(viewport_src
    viewport.h viewport.cxx
)

ADD_LIBRARY(viewport OBJECT ${viewport_src})

TARGET_INCLUDE_DIRECTORIES(viewport PRIVATE
  ${BOOST_INCLUDE_DIRS}
  ${FREETYPE_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIRS}
  "../font"
)
