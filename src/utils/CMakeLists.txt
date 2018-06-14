SET(utils_src
  memory_buffer.h memory_buffer.cxx
)

ADD_LIBRARY(utils OBJECT ${utils_src})

TARGET_INCLUDE_DIRECTORIES(utils PRIVATE
  ${BOOST_INCLUDE_DIRS}
)
