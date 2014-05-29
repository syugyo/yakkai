#
PROJECT( yakkai )
CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )

#
include_directories( "${PROJECT_SOURCE_DIR}" )

#
if(APPLE)
  set( CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++ -Wall" )
else(UNIX)
  set( CMAKE_CXX_FLAGS "-std=c++11 -Wall" )
endif(APPLE)

#
file( GLOB_RECURSE files ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp )

#
add_executable( yakkai ${files} )

#
target_link_libraries(
  yakkai
  )
set_target_properties( yakkai PROPERTIES LINKER_LANGUAGE CXX )

#
install( TARGETS yakkai DESTINATION bin )
