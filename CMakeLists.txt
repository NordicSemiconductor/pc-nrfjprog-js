cmake_minimum_required(VERSION 2.8)

# Name of the project (will be the name of the plugin)
project (pc-nrfjprog-js)

if(APPLE)
    set(CMAKE_SKIP_BUILD_RPATH TRUE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif()

set(NRFJPROG_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/nrfjprog/include)

# Platform specific
if(MSVC)
    include(cmake/msvc.cmake)
elseif(APPLE)
    include(cmake/apple.cmake)
else()
    include(cmake/gcc.cmake)
endif()

# Essential include files to build a node addon,
# you should add this line in every CMake.js based project.
include_directories(${CMAKE_JS_INC} ${NRFJPROG_INCLUDE_DIR})

# Specify source files
file (GLOB SOURCE_FILES
    "src/*.cpp"
    "src/*.h"
)

# This line will tell CMake that we're building a shared library
# named after the project's name
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${PLATFORM_SOURCE_FILES})

# This line will give our library file a .node extension without any "lib" prefix
set_target_properties(${PROJECT_NAME}
    PROPERTIES
    COMPILE_FLAGS "${CMAKE_CXX_FLAGS} -DBUILDING_NODE_EXTENSION"
    PREFIX ""
    SUFFIX ".node"
)

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS "_CRT_SECURE_NO_WARNINGS")
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework CoreFoundation")
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    set_property(TARGET ${PROJECT_NAME} PROPERTY MACOSX_RPATH ON)
else()
    # Assume Linux
#    target_link_libraries(${PROJECT_NAME} "udev")
endif()

message(STATUS ${NRFJPROG_LIB})

# Essential library files to link to a node addon,
# you should add this line in every CMake.js based project.
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB})
