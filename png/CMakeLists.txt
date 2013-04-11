INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} .
     ../geom ../kdtree ../png ../parse ../scene )

set(HDRS  png_reader.h png_writer.h rgbColor.h rgbImage.h)
set(LIBSRCS png_reader.cpp png_writer.cpp rgbImage.cpp)
   
add_library(png_lib ${HDRS} ${LIBSRCS})

