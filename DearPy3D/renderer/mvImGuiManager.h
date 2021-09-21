#pragma once

#include <imgui.h>

// forward declarations
struct GLFWwindow;

namespace DearPy3D {

	void mvSetupImGui(GLFWwindow* window);
	void mvCleanupImGui();
	void mvBeginImGuiFrame();
	void mvEndImGuiFrame();
	void mvResizeImGui();

}