cmake_minimum_required(VERSION 3.15)

project(mango VERSION 0.0.0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (WIN32)
  set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded)
endif()

add_executable(mango 
  src/main.cpp

  src/util/log.cpp

  src/gfx/renderer.cpp
  src/gfx/window.cpp
    
  src/gfx/vulkan/allocator.cpp
  src/gfx/vulkan/device.cpp
  src/gfx/vulkan/includes.cpp
  src/gfx/vulkan/instance.cpp
  src/gfx/vulkan/swapchain.cpp
)

target_include_directories(mango PUBLIC ${CMAKE_SOURCE_DIR}/src)

target_compile_definitions(mango PUBLIC VERSION_MAJOR=${PROJECT_VERSION_MAJOR})
target_compile_definitions(mango PUBLIC VERSION_MINOR=${PROJECT_VERSION_MINOR})
target_compile_definitions(mango PUBLIC VERSION_PATCH=${PROJECT_VERSION_PATCH})
target_compile_definitions(mango PUBLIC VERSION_TWEAK=${PROJECT_VERSION_TWEAK})


if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
    target_compile_options(mango PUBLIC -std=c++2b)
    target_compile_options(mango PUBLIC -O0)
    target_compile_options(mango PUBLIC -g)
    target_compile_options(mango PUBLIC -fsanitize=address)
    target_compile_options(mango PUBLIC -march=native)
    target_compile_options(mango PUBLIC -fvisibility=default)
    target_compile_options(mango PUBLIC -Weverything)
    target_compile_options(mango PUBLIC -Werror)


    target_compile_options(mango PUBLIC -Wno-c++98-compat)
    target_compile_options(mango PUBLIC -Wno-c++98-compat-pedantic)
    target_compile_options(mango PUBLIC -Wno-missing-prototypes)
    target_compile_options(mango PUBLIC -Wno-reserved-macro-identifier)
    target_compile_options(mango PUBLIC -Wno-pre-c++20-compat)
    target_compile_options(mango PUBLIC -Wno-braced-scalar-init)
    target_compile_options(mango PUBLIC -Wno-old-style-cast)
    target_compile_options(mango PUBLIC -Wno-c++20-compat)
    target_compile_options(mango PUBLIC -Wno-padded)
    
    target_link_options(mango PUBLIC -fsanitize=address)

    if (WIN32)    
      target_compile_definitions(mango  PUBLIC _ITERATOR_DEBUG_LEVEL=0)
      target_compile_definitions(mango  PUBLIC _DISABLE_STRING_ANNOTATION _DISABLE_VECTOR_ANNOTATION)
    endif()
  else()
    message(FATAL_ERROR "Clang-cl is not supported")
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(mango PUBLIC -std=c++2b)
  target_compile_options(mango PUBLIC -O0)
  target_compile_options(mango PUBLIC -g)
  
  target_compile_options(mango PUBLIC -march=native)
  target_compile_options(mango PUBLIC -fvisibility=default)

  if (NOT WIN32)
    target_compile_options(mango PUBLIC -fsanitize=address)
    target_link_options(mango  PUBLIC -fsanitize=address)
  endif()

else()
  message(FATAL_ERROR "Unsupported compiler")
endif()




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

add_library(glfw STATIC IMPORTED)
set_target_properties(glfw PROPERTIES
  IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/libglfw3.a
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/inc/glfw/include
)
target_link_libraries(mango glfw)

add_library(fmt STATIC IMPORTED)
set_target_properties(fmt PROPERTIES
  IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/libfmt.a
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/inc/fmt/include
)
target_link_libraries(mango fmt)

add_library(concurrentQueue INTERFACE)
set_target_properties(concurrentQueue PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/inc/concurrentqueue
)
target_link_libraries(mango concurrentQueue)

add_library(VulkanMemoryAllocator INTERFACE)
set_target_properties(VulkanMemoryAllocator PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/inc/VulkanMemoryAllocator/include
)
target_link_libraries(mango VulkanMemoryAllocator)

# message(CMAKE_RC_COMPILER="${CMAKE_RC_COMPILER}")

# "-DCMAKE_RC_COMPILER:FILEPATH=C:/Program Files/LLVM/bin/llvm-rc.exe"