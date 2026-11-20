# Optional GPU check; built explicitly rather than as part of either application.
find_program(PREHISTORIC_GLSLANG_VALIDATOR glslangValidator HINTS "$ENV{VULKAN_SDK}/bin")
if(PREHISTORIC_GLSLANG_VALIDATOR)
    set(compute_shader "${CMAKE_CURRENT_BINARY_DIR}/vulkan_compute.spv")
    add_custom_command(OUTPUT "${compute_shader}"
        COMMAND "${PREHISTORIC_GLSLANG_VALIDATOR}" -V --target-env vulkan1.1
            "${CMAKE_CURRENT_LIST_DIR}/../tests/shaders/vulkan_compute.comp" -o "${compute_shader}"
        DEPENDS "${CMAKE_CURRENT_LIST_DIR}/../tests/shaders/vulkan_compute.comp"
        VERBATIM)
    add_executable(VulkanComputeSmoke EXCLUDE_FROM_ALL
        "${CMAKE_CURRENT_LIST_DIR}/../tests/vulkan_compute_smoke.cpp" "${compute_shader}")
    target_include_directories(VulkanComputeSmoke PRIVATE $<TARGET_PROPERTY:PrehistoricEngine,INCLUDE_DIRECTORIES>)
    target_compile_definitions(VulkanComputeSmoke PRIVATE $<TARGET_PROPERTY:PrehistoricEngine,COMPILE_DEFINITIONS>)
    target_compile_options(VulkanComputeSmoke PRIVATE $<TARGET_PROPERTY:PrehistoricEngine,COMPILE_OPTIONS>)
    target_link_libraries(VulkanComputeSmoke PRIVATE PrehistoricEngine)
endif()
