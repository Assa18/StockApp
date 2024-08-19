#pragma once

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>

#include "UIManager.h"
#include "DataManager.h"

class App
{
public:
	App();
	~App();

	void Run();
private:
	GLFWwindow* m_Window;

	UIManager* m_UIManager;
	DataManager* m_DataManager;
};