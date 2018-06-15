SET(test_font_manager_src
    test_font_manager.cxx
)

ADD_EXECUTABLE(test_font_manager
    ${test_font_manager_src}
)

TARGET_INCLUDE_DIRECTORIES(test_font_manager PRIVATE
     "../../src/font"
)

TARGET_LINK_LIBRARIES(test_font_manager
    freetype_direct_gl
    ${FONTCONFIG_LIBRARY}
    ${FREETYPE_LIBRARY}
)

ADD_EXECUTABLE(example5
    example5.cpp
)

TARGET_INCLUDE_DIRECTORIES(example5 PRIVATE
     ${FREETYPE_INCLUDE_DIRS}
)

TARGET_LINK_LIBRARIES(example5
    ${FREETYPE_LIBRARY}
)
