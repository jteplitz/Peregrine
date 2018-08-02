project(Peregrine CXX)
cmake_minimum_required(VERSION 3.6)

include_directories(${PROJECT_SOURCE_DIR}/src/include)

# Compile device code
add_library(frame_io SHARED
  ${PROJECT_SOURCE_DIR}/src/frame_io.cpp)
# Install resulting .so in our lib folder
install(TARGETS frame_io DESTINATION lib/)
# Install the header file so that HALO can call into our .so
install(FILES ${PROJECT_SOURCE_DIR}/src/include/frame_io.hpp DESTINATION include/)
