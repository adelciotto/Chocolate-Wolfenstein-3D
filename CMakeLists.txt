cmake_minimum_required(VERSION 3.5.1)
project(chocolate-wolfenstein-3D)

# Add the SDL2 CMake modules.
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Add all the cpp sources files.
file(GLOB SOURCE_FILES "src/*.cpp")
file(GLOB HEADER_FILES "src/*.h")
add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})

# Set the C++ standard and compiler flags.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:DEBUG>:/DEBUG> $<$<CONFIG:RELEASE>:/O2>)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wno-logical-op-parentheses -Wno-pragma-pack
        -Wno-unknown-pragmas -Wno-switch $<$<CONFIG:RELEASE>:-O3>)

    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        target_compile_options(${PROJECT_NAME} "-stdlib=libc++")
    endif()
endif()

# Find SDL2 and SDL2_mixer.
if(WIN32)
	set(SDL2_PATH "${CMAKE_SOURCE_DIR}/extern/SDL2-2.0.14" CACHE BOOL "" FORCE)
	set(SDL2_MIXER_PATH "${CMAKE_SOURCE_DIR}/extern/SDL2_mixer-2.0.4" CACHE BOOL "" FORCE)
endif()
find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)

# Link the SDL2 libraries.
target_link_libraries(${PROJECT_NAME} SDL2::Main SDL2::Mixer)

# Copy SDL2 DLLs to output folder on Windows
if(WIN32)
    foreach(DLL ${SDL2_DLLS} ${SDL2_MIXER_DLLS})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND
            ${CMAKE_COMMAND} -E copy_if_different ${DLL} $<TARGET_FILE_DIR:${PROJECT_NAME}>)
    endforeach()
endif()

# Copy the Wolf3D data files to the build directory.
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND 
    ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/data/ $<TARGET_FILE_DIR:${PROJECT_NAME}>)
