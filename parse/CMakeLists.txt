INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} .
     ../geom ../kdtree ../png ../scene )

set(HDRS  parser.h glm.h )

set(LIBSRCS parser.cpp glm.cpp )

add_library(parse_lib ${HDRS} ${LIBSRCS})

