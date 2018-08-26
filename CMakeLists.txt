project(Peregrine CXX)
cmake_minimum_required(VERSION 3.5)
set(TENSYR_ROOT_DIR /opt/halo)

# The install directory contains our files
# In the HALO SDK recommended layout
if(${CMAKE_INSTALL_PREFIX} STREQUAL "/usr/local") # Allow the user to overwrite
  set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/install)
endif()

set(CMAKE_CXX_STANDARD 11)

include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${TENSYR_ROOT_DIR}/runtime/include)

# Compile device code
add_library(frame_io SHARED
  ${PROJECT_SOURCE_DIR}/src/frame_io.cpp)
# Install resulting .so in our lib folder
install(TARGETS frame_io DESTINATION lib/)
# Install the header file so that HALO can call into our .so
install(FILES ${PROJECT_SOURCE_DIR}/src/include/frame_io.hpp DESTINATION include/)
# Install our config
install(FILES ${PROJECT_SOURCE_DIR}/src/config.conf DESTINATION .)
# Install our graph
install(FILES ${PROJECT_SOURCE_DIR}/src/peregrine.graph DESTINATION .)
# Install our run script
install(PROGRAMS ${PROJECT_SOURCE_DIR}/src/run DESTINATION .)

# Install test data
install (FILES ${PROJECT_SOURCE_DIR}/Motion_Detection/motiontest_feeder.mp4 DESTINATION videos/)
