cmake_minimum_required (VERSION 3.13)

add_library(dearpy3d STATIC)

set_property(TARGET dearpy3d PROPERTY CXX_STANDARD 17)

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
		"renderer/mvAllocator.cpp"
		"renderer/mvGraphics.cpp"
		"renderer/mvCamera.cpp"
		"renderer/mvBuffer.cpp"
		"renderer/mvVertexLayout.cpp"
		"renderer/mvPipeline.cpp"
		"renderer/mvShader.cpp"
		"renderer/mvTextures.cpp"
		"renderer/mvMesh.cpp"
		"renderer/mvScene.cpp"

		# Vulkan Memory Allocator
		"../Dependencies/VulkanMemoryAllocator/vk_mem_alloc.cpp"

		# imgui
		"../Dependencies/imgui/misc/cpp/imgui_stdlib.cpp"
		"../Dependencies/imgui/imgui.cpp"
		"../Dependencies/imgui/imgui_demo.cpp"
		"../Dependencies/imgui/imgui_draw.cpp"
		"../Dependencies/imgui/imgui_widgets.cpp"
		"../Dependencies/imgui/imgui_tables.cpp"
		"../Dependencies/imgui/backends/imgui_impl_glfw.cpp"
		"../Dependencies/imgui/backends/imgui_impl_vulkan.cpp"
)

target_include_directories(dearpy3d

	PRIVATE

		"."
		"core/"
		"renderer/"
		"renderer/drawables/"

	    "$ENV{VK_SDK_PATH}/Include/"
		"../Dependencies/glfw/include/"
		"../Dependencies/glm/"
		"../Dependencies/stb/"
		"../Dependencies/VulkanMemoryAllocator/"
		"../Dependencies/imgui/"
		"../Dependencies/imgui/backends/"
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
