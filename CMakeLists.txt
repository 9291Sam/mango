cmake_minimum_required(VERSION 3.15)

include(FetchContent)

project(mango VERSION 0.0.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) 
# https://bugs.llvm.org/show_bug.cgi?id=47950
set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded)

# Enable Link Time Optimization on non debug builds
if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
else()
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# Source Files

add_executable(mango 
    
  src/gfx/vulkan/allocator.cpp
  src/gfx/vulkan/buffer.cpp
  src/gfx/vulkan/descriptors.cpp
  src/gfx/vulkan/device.cpp
  src/gfx/vulkan/image.cpp
  src/gfx/vulkan/includes.cpp
  src/gfx/vulkan/instance.cpp
  src/gfx/vulkan/pipelines.cpp
  src/gfx/vulkan/render_pass.cpp
  src/gfx/vulkan/swapchain.cpp

  src/gfx/camera.cpp
  src/gfx/frame.cpp
  src/gfx/object.cpp
  src/gfx/renderer.cpp
  src/gfx/transform.cpp
  src/gfx/window.cpp

  src/util/log.cpp
  src/util/uuid.cpp

  src/game/entity/cube.cpp
  src/game/entity/disk_entity.cpp
  src/game/entity/entity.cpp

  src/game/world/voxel_octree.cpp
  src/game/world/world.cpp
  
  src/game/game.cpp
  src/game/player.cpp
  
  src/main.cpp

)

target_include_directories(mango PUBLIC ${CMAKE_SOURCE_DIR}/src)

target_compile_definitions(mango PUBLIC VERSION_MAJOR=${PROJECT_VERSION_MAJOR})
target_compile_definitions(mango PUBLIC VERSION_MINOR=${PROJECT_VERSION_MINOR})
target_compile_definitions(mango PUBLIC VERSION_PATCH=${PROJECT_VERSION_PATCH})
target_compile_definitions(mango PUBLIC VERSION_TWEAK=${PROJECT_VERSION_TWEAK})

#
# Compiler specific flags 
#

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
    # Clang
    target_compile_options(mango PUBLIC -march=native)
    target_compile_options(mango PUBLIC -fvisibility=default)
    
    if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
      # Add debug symbols that lldb can actually read
      target_compile_options(mango PUBLIC -glldb)
      target_compile_options(mango PUBLIC -gdwarf-5)
      target_compile_options(mango PUBLIC -g3)
    endif()

    target_compile_options(mango PUBLIC -Wno-c++98-compat)
    target_compile_options(mango PUBLIC -Wno-c++98-compat-pedantic)
    target_compile_options(mango PUBLIC -Wno-missing-prototypes)
    target_compile_options(mango PUBLIC -Wno-reserved-macro-identifier)
    target_compile_options(mango PUBLIC -Wno-pre-c++20-compat)
    target_compile_options(mango PUBLIC -Wno-braced-scalar-init)
    target_compile_options(mango PUBLIC -Wno-old-style-cast)
    target_compile_options(mango PUBLIC -Wno-c++20-compat)
    target_compile_options(mango PUBLIC -Wno-padded)
    target_compile_options(mango PUBLIC -Wno-documentation-unknown-command)
    target_compile_options(mango PUBLIC -Wno-exit-time-destructors)

    # Windows edge case with the address sanitizer
    # https://github.com/llvm/llvm-project/issues/56300
    if (WIN32)    
      target_compile_definitions(mango PUBLIC _ITERATOR_DEBUG_LEVEL=0)
      target_compile_definitions(mango PUBLIC _DISABLE_STRING_ANNOTATION)
      target_compile_definitions(mango PUBLIC _DISABLE_VECTOR_ANNOTATION)
    endif()
  
    # https://clang.llvm.org/docs/UsersManual.html#controlling-code-generation
    # target_compile_options(mango PUBLIC -fsanitize=address)
    # target_link_options(mango PUBLIC -fsanitize=address)

    # target_compile_options(mango PUBLIC -fsanitize=undefined)
    # target_link_options(mango PUBLIC -fsanitize=undefined)

  else()
    # Clang-cl
    message(FATAL_ERROR "Clang-cl support has not been added")
  endif()

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(mango PUBLIC -march=native)
  target_compile_options(mango PUBLIC -fvisibility=default)

  target_compile_options(mango PUBLIC -Wno-stringop-overflow)

  if (NOT WIN32)
    target_compile_options(mango PUBLIC -fsanitize=address)
    target_link_options(mango  PUBLIC -fsanitize=address)
  endif()

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  message(FATAL_ERROR "MSVC support has not been added")
else()
   message(FATAL_ERROR "Unknown and Unsupported compiler")
endif()



#
# Libraries 
#

FetchContent_Declare(fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG master
)
FetchContent_MakeAvailable(fmt)
target_link_libraries(mango PUBLIC fmt::fmt)



FetchContent_Declare(glfw
  GIT_REPOSITORY https://github.com/glfw/glfw
  GIT_TAG master
)
FetchContent_MakeAvailable(glfw)
target_link_libraries(mango PUBLIC glfw)



FetchContent_Declare(concurrentqueue
  GIT_REPOSITORY https://github.com/cameron314/concurrentqueue
  GIT_TAG master
)
FetchContent_MakeAvailable(concurrentqueue)
target_link_libraries(mango PUBLIC concurrentqueue)



FetchContent_Declare(glm
  GIT_REPOSITORY https://github.com/g-truc/glm
  GIT_TAG master
)
FetchContent_MakeAvailable(glm)  
target_link_libraries(mango PUBLIC glm)



FetchContent_Declare(tinyobjloader
  GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader
  GIT_TAG release
)
FetchContent_MakeAvailable(tinyobjloader)  
target_link_libraries(mango PUBLIC tinyobjloader)

FetchContent_Declare(gcem
  GIT_REPOSITORY https://github.com/kthohr/gcem
  GIT_TAG master
)
FetchContent_MakeAvailable(gcem)  
target_link_libraries(mango PUBLIC gcem)





find_package(Vulkan REQUIRED)
include_directories("${Vulkan_INCLUDE_DIRS}")


# VMA's cmakescript is terribly broken, doesnt work with fetchContent + 
# **requires** static linking the
# VMA_DYNAMIC_VULKAN_FUNCTIONS and VMA_STATIC_VULKAN_FUNCTIONS options do
# nothing!!!!!

# as a result the best solution is to just include it as a submodule and include
# the damned header. 
# I love C++'s tooling, it is perfect and has no flaws.
# I love C++'s tooling, it is perfect and has no flaws.
# I love C++'s tooling, it is perfect and has no flaws.
# I love C++'s tooling, it is perfect and has no flaws.
# I love C++'s tooling, it is perfect and has no flaws.

add_library(VulkanMemoryAllocator INTERFACE)
set_target_properties(VulkanMemoryAllocator PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES 
  ${CMAKE_SOURCE_DIR}/inc/VulkanMemoryAllocator/include
)
target_link_libraries(mango PUBLIC VulkanMemoryAllocator)


find_package(Vulkan COMPONENTS glslc)
find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)
if(APPLE) 
  target_link_libraries(mango "-framework Cocoa -framework IOKit")
endif()

function(compile_shader target)
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "ENV;FORMAT" "SOURCES")
    foreach(source ${arg_SOURCES})
        add_custom_command(
            OUTPUT ${source}.${arg_FORMAT}
            DEPENDS ${source}
            DEPFILE ${source}.d
            COMMAND
                ${glslc_executable}
                $<$<BOOL:${arg_ENV}>:--target-env=${arg_ENV}>
                $<$<BOOL:${arg_FORMAT}>:-mfmt=${arg_FORMAT}>
                -MD -MF ${source}.d
                -o ${source}.${arg_FORMAT}
                ${CMAKE_CURRENT_SOURCE_DIR}/${source}
        )
        target_sources(${target} PRIVATE ${source}.${arg_FORMAT})
    endforeach()
endfunction()

compile_shader(mango
  ENV vulkan1.0
  FORMAT bin
  SOURCES
    src/gfx/vulkan/shaders/flat_pipeline.vert
    src/gfx/vulkan/shaders/flat_pipeline.frag
    src/gfx/vulkan/shaders/voxel.vert
    src/gfx/vulkan/shaders/voxel.frag
)