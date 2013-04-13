INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} .
     ../kdtree ../png ../parse ../scene )

set(HDRS
  common.h ray.h shape.h 
  material.h view.h light.h
  sphere.h rectangle.h triangle.h scene.h
  photon.h )

set(LIBSRCS common.cpp  scene.cpp ray.cpp 
    sphere.cpp rectangle.cpp triangle.cpp
    photon.cpp )
   
add_library(geom_lib ${HDRS} ${LIBSRCS})

