cmake_minimum_required(VERSION 3.10)
project(SWARM_X LANGUAGES CXX)

# Require C++14 or higher (supports modern C++ features across all compilers)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include the header files directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Gather all C++ source files inside the src directory
file(GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# Create executable with main.cpp and all src files
add_executable(SWARM_X main.cpp ${SRC_FILES})

# Platform-specific socket linking for HTTP web server
if(WIN32)
    target_link_libraries(SWARM_X PRIVATE ws2_32 wsock32)
endif()