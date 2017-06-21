add_compile_options(-pthread -std=c++11)

file (GLOB PLATFORM_SOURCE_FILES
    "src/platform/*_osx.cpp"
)
