message(STATUS "WIN32 build.")

add_definitions(
    -D_WIN32_WINNT=0x0502
    -DNOMINMAX
)

# Issue with VC and disabling of C4200: https://connect.microsoft.com/VisualStudio/feedback/details/1114440
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4200")

set(PLATFORM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/win)

file (GLOB PLATFORM_SOURCE_FILES
    "src/platform/win/*.cpp"
)
