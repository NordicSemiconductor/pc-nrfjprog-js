add_compile_options(
    -Wall
    -Wno-effc++
    -Wno-unknown-pragmas
    -Wno-undef
    -Wstrict-overflow=2
    -Wno-long-long
    -Wfloat-equal
    -Wshadow
    -Wpointer-arith
    -Wlogical-op
    #-H # Used for debugging header dependencies. See https://docs.freebsd.org/info/gcc/gcc.info.Preprocessor_Options.html
)

set(PLATFORM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/linux)

file (GLOB PLATFORM_SOURCE_FILES
    "src/platform/linux/*.cpp"
)
