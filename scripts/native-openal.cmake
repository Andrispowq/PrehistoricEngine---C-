# The old vendor snapshot contains a Windows-generated config.h and no upstream
# CMake build. Use the complete, pinned upstream release for macOS instead.
include(FetchContent)
set(LIBTYPE STATIC CACHE STRING "OpenAL library type" FORCE)
set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ALSOFT_TESTS OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL OFF CACHE BOOL "" FORCE)
set(ALSOFT_REQUIRE_COREAUDIO ON CACHE BOOL "" FORCE)
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()
FetchContent_Declare(openal_soft
    URL https://codeload.github.com/kcat/openal-soft/tar.gz/refs/tags/1.24.2
    URL_HASH SHA256=7efd383d70508587fbc146e4c508771a2235a5fc8ae05bf6fe721c20a348bd7c
)
FetchContent_MakeAvailable(openal_soft)

# Explicitly requested only; validates linkage independently of the engine port.
add_executable(OpenALSmoke EXCLUDE_FROM_ALL
    "${CMAKE_CURRENT_LIST_DIR}/../tests/openal_smoke.cpp")
target_link_libraries(OpenALSmoke PRIVATE OpenAL::OpenAL)
