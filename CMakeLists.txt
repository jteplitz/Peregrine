project(Peregrine CXX)
cmake_minimum_required(VERSION 3.5)
set(TENSYR_ROOT_DIR /opt/halo)
find_package( OpenCV REQUIRED )

# The install directory contains our files
# In the HALO SDK recommended layout
if(${CMAKE_INSTALL_PREFIX} STREQUAL "/usr/local") # Allow the user to overwrite
  set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/install)
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wextra -Wpedantic")

include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${TENSYR_ROOT_DIR}/runtime/include)

# Compile device code
add_library(frame_io SHARED
  ${PROJECT_SOURCE_DIR}/src/frame_io.cpp)
# Install resulting .so in our lib folder
install(TARGETS frame_io DESTINATION lib/)
# Install the header file so that HALO can call into our .so
install(FILES ${PROJECT_SOURCE_DIR}/src/include/frame_io.hpp DESTINATION include/)

# Compile peregrine library
add_library(peregrine SHARED
  ${PROJECT_SOURCE_DIR}/src/peregrine.cpp)
target_link_libraries(peregrine ${OpenCV_LIBS})
# Install resulting .so in our lib folder
install (TARGETS peregrine DESTINATION lib/)
# Install the header file so that HALO can call into our .so
install(FILES ${PROJECT_SOURCE_DIR}/src/include/peregrine.hpp DESTINATION include/)
# Install our config
install(FILES ${PROJECT_SOURCE_DIR}/src/config.conf DESTINATION .)
# Install our graph
install(FILES ${PROJECT_SOURCE_DIR}/src/peregrine.graph DESTINATION .)
# Install our run script
install(PROGRAMS ${PROJECT_SOURCE_DIR}/src/run DESTINATION .)

# Install test data
install (FILES ${PROJECT_SOURCE_DIR}/Motion_Detection/motiontest_feeder.mp4 DESTINATION videos/)
