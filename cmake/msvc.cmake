message(STATUS "WIN32 build.")

add_definitions(
    -D_WIN32_WINNT=0x0502
    -DNOMINMAX
)

# Issue with VC and disabling of C4200: https://connect.microsoft.com/VisualStudio/feedback/details/1114440
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4200")

# Env variable hack as per http://cmake.3232098.n2.nabble.com/CMP0053-Unable-to-refer-to-ENV-PROGRAMFILES-X86-td7588994.html
set(PROGRAMFILESX86 "PROGRAMFILES(X86)")

set(PLATFORM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/win $ENV{${PROGRAMFILESX86}}/Nordic\ Semiconductor/nrf5x/bin/headers $ENV{ProgramW6432}/Nordic\ Semiconductor/nrf5x/bin/headers)

file (GLOB PLATFORM_SOURCE_FILES
    "src/platform/win/*.cpp"
)
