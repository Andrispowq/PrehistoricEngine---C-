# Included by the Premake-generated CPU-only configuration.
if (POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif ()
include(FetchContent)
FetchContent_Declare(Catch2
        URL https://codeload.github.com/catchorg/Catch2/tar.gz/refs/tags/v3.8.1
        URL_HASH SHA256=18b3f70ac80fccc340d8c6ff0f339b2ae64944782f8d2fca2bd705cf47cadb79)
FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
include(Catch)

function(add_math_tests target)
    add_executable(${target}
            unit/math/MathTests.cpp
            unit/rendering/TemporalTests.cpp
            "${PROJECT_SOURCE_DIR}/PrehistoricEngine/src/engine/prehistoric/core/util/math/Math.cpp")
    target_include_directories(${target} PRIVATE "${PROJECT_SOURCE_DIR}/PrehistoricEngine/src/engine")
    target_link_libraries(${target} PRIVATE Catch2::Catch2WithMain)
    target_compile_definitions(${target} PRIVATE
            "$<$<CONFIG:Debug>:PR_DEBUG>" "$<$<CONFIG:Release>:PR_RELEASE>")
    if (MSVC)
        target_compile_definitions(${target} PRIVATE NOMINMAX _CRT_SECURE_NO_WARNINGS)
    endif ()
    catch_discover_tests(${target}
            TEST_SPEC "~[known-defect]"
            TEST_PREFIX "${target}/"
            PROPERTIES LABELS "contract" TIMEOUT 15)
    if (target STREQUAL "MathTestsSIMD")
        catch_discover_tests(${target}
                TEST_SPEC "[known-defect]"
                TEST_PREFIX "${target}/"
                PROPERTIES LABELS "known-defect" TIMEOUT 15)
    endif ()
endfunction()

add_math_tests(MathTestsScalar)
if (WIN32 AND MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_math_tests(MathTestsSIMD)
    target_compile_definitions(MathTestsSIMD PRIVATE PR_FAST_MATH)
endif ()
