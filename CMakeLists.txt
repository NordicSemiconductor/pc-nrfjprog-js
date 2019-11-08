cmake_minimum_required(VERSION 3.12)

# Name of the project (will be the name of the plugin)
project (pc-nrfjprog-js)

if(APPLE)
    set(CMAKE_SKIP_BUILD_RPATH TRUE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    set(CMAKE_MACOSX_RPATH 1)
endif()

set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "nrfjprog")

# set(NRF_COMMAND_LINE_TOOLS_CMAKE_PACKAGE_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/nrfjprog/share)

# message(STATUS "LOCATION: ${NRF_COMMAND_LINE_TOOLS_CMAKE_PACKAGE_LOCATION}")

# find_package(nRFCommandLineTools
#     10.4.1 EXACT
#     REQUIRED
#     COMPONENTS nrf::highlevelnrfjprog nrf::nrfjprogdll
#     PATHS ${NRF_COMMAND_LINE_TOOLS_CMAKE_PACKAGE_LOCATION}
# )

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform specific
if(MSVC)
    include(cmake/msvc.cmake)
elseif(APPLE)
    include(cmake/apple.cmake)
else()
    include(cmake/gcc.cmake)
endif()

# Include the local include directories
# include_directories(${PLATFORM_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/nrfjprog/include)
include_directories(${PLATFORM_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/nrfjprog)

# Essential include files to build a node addon,
# you should add this line in every CMake.js based project.
include_directories(SYSTEM ${CMAKE_JS_INC})

# Specify source files
set(SOURCE_FILES
    src/export.cpp
    src/highlevel_helpers.cpp
    src/highlevel.cpp
    src/osfiles.cpp
    src/utility/conversion.cpp
    src/utility/errormessage.cpp
    src/utility/utility.cpp
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
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/DELAYLOAD:node.exe")
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework CoreFoundation")
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    set_property(TARGET ${PROJECT_NAME} PROPERTY MACOSX_RPATH ON)
else()
    # Assume Linux
#    target_link_libraries(${PROJECT_NAME} "udev")
endif()

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()

message(STATUS ${NRFJPROG_LIB})


find_library(HIGHLEVEL highlevelnrfjprog PATHS "${CMAKE_SOURCE_DIR}/nrfjprog")

# Essential library files to link to a node addon,
# you should add this line in every CMake.js based project.
target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_JS_LIB} ${HIGHLEVEL})

include (cmake/clang-dev-tools.cmake)
