add_compile_options(-pthread -std=c++11)

set(PLATFORM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/osx)

file (GLOB PLATFORM_SOURCE_FILES
    "src/platform/osx/*.cpp"
)
