cmake_minimum_required (VERSION 3.13)

function(add_shader SHADER)
    find_program(GLSLC glslc "../Dependencies/vk_sdk_lite/bin/")

    set(current-shader-path "${CMAKE_CURRENT_SOURCE_DIR}/../DearPy3D/shaders/${SHADER}")
    set(current-output-path "${CMAKE_CURRENT_SOURCE_DIR}/../DearPy3D/shaders/${SHADER}.spv")

    # Add a custom command to compile GLSL to SPIR-V.
    get_filename_component(current-output-dir ${current-output-path} DIRECTORY)
    file(MAKE_DIRECTORY ${current-output-dir})

    add_custom_command(
           OUTPUT ${current-output-path}
           COMMAND ${GLSLC} -o ${current-output-path} ${current-shader-path}
           DEPENDS ${current-shader-path}
           IMPLICIT_DEPENDS CXX ${current-shader-path}
           VERBATIM)

    # Make sure our build depends on this output.
    set_source_files_properties(${current-output-path} PROPERTIES GENERATED TRUE)
    target_sources(dearpy3dsandbox PRIVATE ${current-output-path})
endfunction(add_shader)

add_executable (dearpy3dsandbox)

set_property(TARGET dearpy3dsandbox PROPERTY CXX_STANDARD 17)

if(MV_NO_SHADERS)
else()
	add_shader(phong.frag)
	add_shader(shadow.frag)
	add_shader(phong.vert)
	add_shader(shadow.vert)
	add_shader(skybox.vert)
	add_shader(skybox.frag)
	add_shader(omnishadow.frag)
	add_shader(omnishadow.vert)
endif()

target_sources(dearpy3dsandbox

	PRIVATE

		"main.cpp"
)

target_include_directories(dearpy3dsandbox

	PRIVATE

		"../DearPy3D/"
		"../DearPy3D/core/"
		"../DearPy3D/renderer/"
		"../DearPy3D/vendor/imgui/"
		"../DearPy3D/vendor/imgui/backends/"

		"../Dependencies/vk_sdk_lite/Include/"
		"../Dependencies/stb/"
		"../Dependencies/VulkanMemoryAllocator/"
)

target_link_directories(dearpy3dsandbox

	PRIVATE
		
		"../Dependencies/vk_sdk_lite/Lib"

)

target_link_libraries(dearpy3dsandbox

	PRIVATE

		$<$<PLATFORM_ID:Linux>:vulkan>
		$<$<PLATFORM_ID:Windows>:vulkan-1>
		dearpy3d
)

set_target_properties(dearpy3dsandbox
  PROPERTIES
  CXX_STANDARD 17
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}$<$<CONFIG:Release>:/cmake-build-release/>$<$<CONFIG:Debug>:/cmake-build-debug/>DearSandbox/"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}$<$<CONFIG:Release>:/cmake-build-release/>$<$<CONFIG:Debug>:/cmake-build-debug/>DearSandbox/"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}$<$<CONFIG:Release>:/cmake-build-release/>$<$<CONFIG:Debug>:/cmake-build-debug/>DearSandbox/"
  )
