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
    font
    ${FONTCONFIG_LIBRARY}
)
