cmake_minimum_required(VERSION 3.18)

project(Muzombie)
set(MUZZLE_ROOT "../muzzle")
set(MUZZLE_LIB_FOLDER ${MUZZLE_ROOT}/lib)
set(MUZZLE_DEPS_FOLDER ${MUZZLE_ROOT}/deps)
set(FREETYPE ${MUZZLE_DEPS_FOLDER}/freetype)
set(target_name muzombie)

set(CMAKE_BUILD_TYPE Debug)
set(DEBUG_LEVEL Normal)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    set(MUZZLE_LIB muzzled)
    add_compile_definitions(MUZZLE_DEBUG_BUILD)
    add_compile_definitions(MUZOMBIE_DEBUG_BUILD)

    if (DEBUG_LEVEL MATCHES Sanitize)
        add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
        set_debug_libs(debug_libs asan ubsan)
    else()
        set(debug_libs "")
    endif()
else()
    set(MUZZLE_LIB muzzle)
    set(debug_libs "")
endif()

include_directories(${MUZZLE_ROOT}/include ${FREETYPE}/include include)
link_directories(${MUZZLE_ROOT}/lib ${MUZZLE_DEPS_FOLDER}/glfw/lib ${FREETYPE}/lib)

set(source
src/main.c
src/arena.c
src/helpers.c
src/list.c
src/strict_alloc.c
src/dataset.c
)

add_executable(${target_name} ${source})

message(STATUS "Checking OS")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    message(STATUS "Checking OS - found (Darwin/MacOS)")
    message(WARNING "Checking OS - warning (Darwin/MacOS support in this CMakeLists.txt is very limited)")

    target_link_libraries(${target_name} ${MUZZLE_LIB} glfw3 freetype CoreVideo OpenGL IOKit Cocoa Carbon)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    message(STATUS "Checking OS - found (WIN32)")
    target_link_libraries(${target_name} ${MUZZLE_LIB} glfw3 freetype opengl32 gdi32 DSound)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    message(STATUS "Checking OS - found (Linux)")
    target_link_libraries(${target_name} ${MUZZLE_LIB} ${debug_libs} glfw3 freetype GL m z)

else ()
# Default to Unix
    message(STATUS "Checking OS - not found (Unix)")
    target_link_libraries(${target_name} ${MUZZLE_LIB} glfw3 freetype GL m z)
endif ()
