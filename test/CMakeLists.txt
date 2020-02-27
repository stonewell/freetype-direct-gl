ADD_SUBDIRECTORY(font)

FIND_PACKAGE(glfw3 3.2 REQUIRED)

SET(markup_src
    markup.cxx
)

SET(screenshot_src
    screenshot-util.c screenshot-util.h
    opengl.h
)

SET(EXTRA_LIB "")

IF(APPLE)
   EXEC_PROGRAM(uname ARGS -v  OUTPUT_VARIABLE DARWIN_VERSION)
   STRING(REGEX MATCH "[0-9]+" DARWIN_VERSION ${DARWIN_VERSION})
   MESSAGE(STATUS "DARWIN_VERSION=${DARWIN_VERSION}")
   IF (DARWIN_VERSION GREATER_EQUAL 18)
      LIST(APPEND markup_src glfw_hack.h glfw_hack.m)
      SET(EXTRA_LIB "-framework Foundation")
      MESSAGE(STATUS "add glfw hack fix for macos > 10.14")
      ADD_DEFINITIONS(-DGLFW_HACK_FIX)
   ENDIF (DARWIN_VERSION GREATER_EQUAL 18)
ELSE()
  IF(${CMAKE_VERSION} VERSION_LESS "3.15.0")
    SET(EXTRA_LIB ${GLEW_LIBRARY})
  ELSE()
    SET(EXTRA_LIB GLEW::GLEW)
  ENDIF()
ENDIF()

ADD_EXECUTABLE(markup
    ${markup_src}
    ${screenshot_src}
)

TARGET_INCLUDE_DIRECTORIES(markup PRIVATE
     "../src/font"
     "../src/text"
     "../src/render"
     "../src/viewport"
    $<TARGET_PROPERTY:glfw,INTERFACE_INCLUDE_DIRECTORIES>
    ${OPENGL_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(markup
    freetype_direct_gl
    ${FONTCONFIG_LIBRARY}
    ${FREETYPE_LIBRARY}
    glfw
    ${OPENGL_LIBRARY}
    ${EXTRA_LIB}
)
