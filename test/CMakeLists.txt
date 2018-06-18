ADD_SUBDIRECTORY(font)

FIND_PACKAGE(glfw3 3.2 REQUIRED)

SET(markup_src
    markup.cxx
)

SET(screenshot_src
    screenshot-util.c screenshot-util.h
    opengl.h
)

ADD_EXECUTABLE(markup
    ${markup_src}
    ${screenshot_src}
)

TARGET_INCLUDE_DIRECTORIES(markup PRIVATE
     "../src/font"
     "../src/text"
     "../src/render"
    $<TARGET_PROPERTY:glfw,INTERFACE_INCLUDE_DIRECTORIES>
    ${OPENGL_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(markup
    freetype_direct_gl
    ${FONTCONFIG_LIBRARY}
    ${FREETYPE_LIBRARY}
    glfw
    ${OPENGL_LIBRARY}
)
