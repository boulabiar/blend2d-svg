cmake_minimum_required(VERSION 3.20)
project(svgimage)

file(GLOB Header_Files "svg/*.h")
set(Source_Files svgimage.cpp)

add_executable(${PROJECT_NAME} 
    ${Header_Files}
    ${Source_Files}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    blend2d
)

# Link configuration
target_link_directories(${PROJECT_NAME} PRIVATE
    lib
)
target_link_libraries(${PROJECT_NAME} PRIVATE blend2d)

# Set C++ standard (modern CMake way)
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Compiler options with proper conditionals
target_compile_options(${PROJECT_NAME} PRIVATE
    # MSVC flags
    $<$<CXX_COMPILER_ID:MSVC>:
        /permissive-
        /std:c++17
        /sdl
        /W3
        /wd5105  # Optional: disable macro expansion warning
    >
    
    # GCC/Clang flags
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:
        -std=c++17
        -Wuninitialized
        -Wmissing-include-dirs
        -Werror=return-type
	#-Werror=non-virtual-dtor
    >
)
