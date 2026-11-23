# Also usable standalone by Premake/Visual Studio, without configuring the engine.
cmake_minimum_required(VERSION 3.21)
project(PrehistoricResources LANGUAGES NONE)

get_filename_component(resource_source "${CMAKE_CURRENT_LIST_DIR}/../../PrehistoricEngine/res" ABSOLUTE)
set(PREHISTORIC_RESOURCE_OUTPUT "${CMAKE_BINARY_DIR}/bin/engine-resources" CACHE PATH
        "Runtime engine resources directory")
find_program(PREHISTORIC_GLSLC NAMES glslc
        HINTS "$ENV{VULKAN_SDK}/bin" "$ENV{VULKAN_SDK}/Bin")
if (NOT PREHISTORIC_GLSLC)
    message(FATAL_ERROR "glslc is required to build engine shaders. Install the Vulkan SDK, set VULKAN_SDK or PATH, or pass -DPREHISTORIC_GLSLC=/path/to/glslc.")
endif ()
option(PREHISTORIC_EXPERIMENTAL_SHADERS "Compile the unfinished Vulkan environment-map shader" OFF)

# Runtime inputs are copied incrementally; shader source additions trigger reconfigure.
file(GLOB_RECURSE runtime_inputs CONFIGURE_DEPENDS RELATIVE "${resource_source}"
        "${resource_source}/fonts/*" "${resource_source}/shaders/opengl/*" "${resource_source}/shaders/shared/*"
        "${resource_source}/models/*" "${resource_source}/textures/*")
set(runtime_outputs)
foreach (relative IN LISTS runtime_inputs)
    set(output "${PREHISTORIC_RESOURCE_OUTPUT}/${relative}")
    get_filename_component(directory "${output}" DIRECTORY)
    add_custom_command(OUTPUT "${output}"
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${directory}"
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${resource_source}/${relative}" "${output}"
            DEPENDS "${resource_source}/${relative}" VERBATIM)
    list(APPEND runtime_outputs "${output}")
endforeach ()

# glslc depfiles do not reliably escape spaces across SDK versions. Conservatively
# depend on all non-entrypoint Vulkan files instead; entrypoint edits stay local,
# while include additions/edits rebuild the shader set. No Python/shell required.
file(GLOB_RECURSE shader_includes CONFIGURE_DEPENDS "${resource_source}/shaders/vulkan/*"
        "${resource_source}/shaders/shared/*.glsl")
foreach (entry atmosphere/capture.comp atmosphere/scattering.frag postProcessing/hdr.vert postProcessing/hdr.frag basic/basic.vert basic/basic.frag pbr/pbr.vert pbr/pbr.frag background/background.vert background/background.frag ibl/ibl.comp ibl/convert.comp environmentMap/environmentMap.comp)
    list(REMOVE_ITEM shader_includes "${resource_source}/shaders/vulkan/${entry}")
endforeach ()
# A removed include must invalidate outputs too, rather than disappearing silently
# from the dependency list after a glob-triggered reconfigure.
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/shader-includes.txt" CONTENT "${shader_includes}")

function(prehistoric_shader source destination)
    set(input "${resource_source}/shaders/vulkan/${source}")
    set(output "${PREHISTORIC_RESOURCE_OUTPUT}/shaders/vulkan/${destination}")
    get_filename_component(directory "${output}" DIRECTORY)
    add_custom_command(OUTPUT "${output}"
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${directory}"
            COMMAND "${PREHISTORIC_GLSLC}" --target-env=vulkan1.1
            -I "${resource_source}/shaders/vulkan"
            "${input}" -o "${output}"
            DEPENDS "${input}" "${PREHISTORIC_GLSLC}" ${shader_includes}
            "${CMAKE_CURRENT_BINARY_DIR}/shader-includes.txt"
            VERBATIM
            COMMENT "Compiling Vulkan shader ${source}")
    set(shader_outputs ${shader_outputs} "${output}" PARENT_SCOPE)
endfunction()

prehistoric_shader(atmosphere/capture.comp atmosphere/capture_CS.spv)
prehistoric_shader(atmosphere/scattering.frag atmosphere/scattering_FS.spv)
prehistoric_shader(postProcessing/hdr.vert postProcessing/hdr_VS.spv)
prehistoric_shader(postProcessing/hdr.frag postProcessing/hdr_FS.spv)
prehistoric_shader(basic/basic.vert basic/basic_VS.spv)
prehistoric_shader(basic/basic.frag basic/basic_FS.spv)
prehistoric_shader(pbr/pbr.vert pbr/pbr_VS.spv)
prehistoric_shader(pbr/pbr.frag pbr/pbr_FS.spv)
prehistoric_shader(background/background.vert background/background_VS.spv)
prehistoric_shader(background/background.frag background/background_FS.spv)
prehistoric_shader(ibl/convert.comp ibl/convert_CS.spv)
prehistoric_shader(ibl/ibl.comp ibl/ibl_CS.spv)
if (PREHISTORIC_EXPERIMENTAL_SHADERS)
    prehistoric_shader(environmentMap/environmentMap.comp environmentMap/environmentMap_CS.spv)
endif ()
add_custom_target(Shaders DEPENDS ${shader_outputs})
add_custom_target(EngineResources ALL DEPENDS ${runtime_outputs})
add_dependencies(EngineResources Shaders)
# Stage/install only runtime files, not sources or compiler scripts.
# Exclude depfiles left by older builds.
install(DIRECTORY "${PREHISTORIC_RESOURCE_OUTPUT}/" DESTINATION engine-resources
        PATTERN "*.d" EXCLUDE)

# Shared demonstration assets are a separate package, not engine defaults.
get_filename_component(sample_source "${CMAKE_CURRENT_LIST_DIR}/../../samples/res" ABSOLUTE)
get_filename_component(package_parent "${PREHISTORIC_RESOURCE_OUTPUT}" DIRECTORY)
set(PREHISTORIC_SAMPLE_OUTPUT "${package_parent}/sample-resources" CACHE PATH
        "Runtime shared sample resources directory")
file(GLOB_RECURSE sample_inputs CONFIGURE_DEPENDS RELATIVE "${sample_source}"
        "${sample_source}/models/*" "${sample_source}/textures/*" "${sample_source}/sounds/*")
set(sample_outputs)
foreach (relative IN LISTS sample_inputs)
    set(output "${PREHISTORIC_SAMPLE_OUTPUT}/${relative}")
    get_filename_component(directory "${output}" DIRECTORY)
    add_custom_command(OUTPUT "${output}"
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${directory}"
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${sample_source}/${relative}" "${output}"
            DEPENDS "${sample_source}/${relative}" VERBATIM)
    list(APPEND sample_outputs "${output}")
endforeach ()
add_custom_target(SampleResources DEPENDS ${sample_outputs})
add_dependencies(EngineResources SampleResources)
install(DIRECTORY "${PREHISTORIC_SAMPLE_OUTPUT}/" DESTINATION sample-resources)
