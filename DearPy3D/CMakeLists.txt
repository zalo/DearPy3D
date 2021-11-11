cmake_minimum_required (VERSION 3.13)

add_library(dearpy3d STATIC)

set_property(TARGET dearpy3d PROPERTY CXX_STANDARD 17)

target_compile_definitions(dearpy3d

	PUBLIC

		$<$<CONFIG:Debug>:MV_DEBUG>
		$<$<CONFIG:Release>:MV_RELEASE>
)

target_sources(dearpy3d

	PRIVATE

		"mvContext.cpp"

		# core
		"core/mvViewport.cpp"
		"core/mvTimer.cpp"
		"core/mvMath.cpp"

		# renderer
		"renderer/mvObjLoader.cpp"
		"renderer/mvRenderer.cpp"
		"renderer/mvAssetManager.cpp"
		"renderer/mvMaterials.cpp"
		"renderer/mvLights.cpp"
		"renderer/mvGraphics.cpp"
		"renderer/mvCamera.cpp"
		"renderer/mvBuffer.cpp"
		"renderer/mvPipeline.cpp"
		"renderer/mvTextures.cpp"
		"renderer/mvMesh.cpp"
		"renderer/mvScene.cpp"

		# Vulkan Memory Allocator
		"../Dependencies/VulkanMemoryAllocator/vk_mem_alloc.cpp"

		# imgui
		"vendor/imgui/misc/cpp/imgui_stdlib.cpp"
		"vendor/imgui/imgui.cpp"
		"vendor/imgui/imgui_demo.cpp"
		"vendor/imgui/imgui_draw.cpp"
		"vendor/imgui/imgui_widgets.cpp"
		"vendor/imgui/imgui_tables.cpp"
		"vendor/imgui/backends/imgui_impl_glfw.cpp"
		"vendor/imgui/backends/imgui_impl_vulkan.cpp"
)

target_include_directories(dearpy3d

	PRIVATE

		"."
		"core/"
		"renderer/"
		"renderer/drawables/"
		"vendor/imgui/"
		"vendor/imgui/backends/"

	    "$ENV{VK_SDK_PATH}/Include/"
		"../Dependencies/glfw/include/"
		"../Dependencies/stb/"
		"../Dependencies/VulkanMemoryAllocator/"
)

target_link_directories(dearpy3d

	PRIVATE
		
		"$ENV{VK_SDK_PATH}/Lib"

)

target_link_libraries(dearpy3d

	PRIVATE

		$<$<PLATFORM_ID:Linux>:vulkan>
		$<$<PLATFORM_ID:Windows>:vulkan-1>
		glfw
)
