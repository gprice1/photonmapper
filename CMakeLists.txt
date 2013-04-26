
cmake_minimum_required(VERSION 2.8.0)
set(CMAKE_CXX_FLAGS "-g -Wall -O3")
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

#add_subdirectory( data )
add_subdirectory( png )
add_subdirectory( geom )
add_subdirectory( parse )

#INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} . )
#include_directories( ./geom/ ./kdtree/ ./png/ ./parse/ )

set( DIREC png kdtree geom parse )
include_directories( ${DIREC} )
link_directories( ${DIREC} )

set(HDRS  raytracer.h photonmapper.h functions.h )

set(LIBSRCS raytracer.cpp photonmapper.cpp)
   
add_library(rtrace ${HDRS} ${LIBSRCS})


SET(MYLIBS ${EXT_LIBS} )
add_executable(profiler profiler.cpp)

add_executable(makescene ${HDRS} makescene.cpp)


#include_directories( ${DIREC} )
#link_directories( ${DIREC} )

set(ALLLIBS rtrace ${EXT_LIBS} ${PNG_LIBRARIES} geom_lib parse_lib
                                                 png_lib)
target_link_libraries(makescene ${ALLLIBS})
target_link_libraries(profiler ${ALLLIBS})

symlink_data(raytracer *.txt)

