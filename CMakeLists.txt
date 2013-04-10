cmake_minimum_required(VERSION 2.8.0)
set(CMAKE_CXX_FLAGS "-g -Wall")
set (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_modules) #for FindGLEW.cmake

FIND_PACKAGE(OpenGL REQUIRED) #for graphics
FIND_PACKAGE(Qt4 REQUIRED) #for window widgets
FIND_PACKAGE(GLEW REQUIRED)

include_directories(PNG_INCLUDE_DIR)
find_package(PNG REQUIRED)

#required CMake options for QT Apps
INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})

SET(EXT_LIBS ${QT_LIBRARIES} ${OPENGL_LIBRARY} ${GLEW_LIBRARY} )

#get symlink_data macro
include(${CMAKE_SOURCE_DIR}/cmake_modules/helpers.cmake)


set(HDRS common.h png_reader.h png_writer.h
  rgbColor.h rgbImage.h ray.h shape.h 
  material.h view.h light.h
  parser.h raytracer.h
  sphere.h rectangle.h triangle.h scene.h
  photon.h glm.h
  feature_vector.h indirect_heap.h k-heap.h k-vector.h kd-tree.h
  kd-tree_sse_24d.h
)

set(LIBSRCS common.cpp png_reader.cpp png_writer.cpp rgbImage.cpp
  ray.cpp parser.cpp raytracer.cpp sphere.cpp rectangle.cpp triangle.cpp
  photon.cpp glm.cpp
  #feature_vector.cpp indirect_heap.cpp k-heap.cpp k-vector.cpp kd-tree.cpp
  #kd-tree_io.cpp speed_kdtree.cpp test_kdtree.cpp
)
   
add_library(rtrace ${HDRS} ${LIBSRCS})

#  add these to library above when ready
#  sphere.h sphere.cpp
#  rectangle.h rectangle.cpp
#  triangle.h triangle.cpp


add_executable(makescene ${HDRS} makescene.cpp)

set(ALLLIBS rtrace ${EXT_LIBS} ${PNG_LIBRARIES})
target_link_libraries(makescene ${ALLLIBS})

symlink_data(raytracer *.txt *.obj)

