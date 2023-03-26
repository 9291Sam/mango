cmake_minimum_required(VERSION 3.15)

project(mango VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded)

add_executable(mango src/main.cpp)
target_compile_options(mango PUBLIC -std=c++2b)
target_compile_options(mango PUBLIC -O0)
target_compile_options(mango PUBLIC -g)
target_compile_options(mango PUBLIC -fsanitize=address)
target_compile_options(mango PUBLIC -march=native)
target_compile_options(mango PUBLIC -fvisibility=default)
target_compile_options(mango PUBLIC -Weverything)

target_compile_options(mango PUBLIC -Wno-c++98-compat)
target_compile_options(mango PUBLIC -Wno-missing-prototypes)

target_link_options(mango  PUBLIC -fsanitize=address)

target_compile_definitions(mango  PUBLIC _ITERATOR_DEBUG_LEVEL=0)
target_compile_definitions(mango  PUBLIC _DISABLE_STRING_ANNOTATION _DISABLE_VECTOR_ANNOTATION)

# # detect compilers, enable warnings and standard conformance
# set(msvc "$<COMPILE_LANG_AND_ID:CXX,MSVC>")
# set(gcc "$<COMPILE_LANG_AND_ID:CXX,GNU>")
# set(clang "$<COMPILE_LANG_AND_ID:CXX,Clang,AppleClang,ROCMClang,XLClang,FujitsuClang,ARMClang>")

# set(simulate_msvc "$<STREQUAL:${CMAKE_CXX_COMPILER_FRONTEND_VARIANT},MSVC>")
# set(clang_cl "$<AND:${clang},${simulate_msvc}>")

# set(msvc_like "$<OR:${msvc},${clang_cl}>")
# set(gcc_like "$<OR:${gcc},$<AND:${clang},$<NOT:${simulate_msvc}>>>")

# add_library(options INTERFACE)
# add_library(common::options ALIAS options)
# target_compile_options(
#     options INTERFACE
#     $<${msvc_like}:/permissive- /utf-8 /diagnostics:caret /W4>
#     $<${gcc_like}:-Wall -Wextra -pedantic-errors>
# )

add_library(vulkan INTERFACE)
target_include_directories(vulkan INTERFACE ${CMAKE_SOURCE_DIR}/inc/Vulkan-Headers/include)
target_link_libraries(mango vulkan)
if(APPLE) 
  target_link_libraries(mango "-framework Cocoa -framework IOKit")
endif()