INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} .
     ../geom ../kdtree ../png ../parse ../scene )

set(HDRS feature_vector.h indirect_heap.h k-heap.h k-vector.h kd-tree.h  )
   
add_library(kdtree_lib ${HDRS} )

