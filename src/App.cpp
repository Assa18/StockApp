#include "App.h"

#include <iostream>

#include <stb_image/stb_image.h>

#include "Serializer.h"

App::App()
	:m_Window(nullptr), m_UIManager(nullptr), m_DataManager(nullptr)
{
	//ImGui and glfw setup
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	m_Window = glfwCreateWindow(1280, 720, "StockScanner", nullptr, nullptr);
	if (m_Window == nullptr)
	{
		std::cout << "Failed to create window!\n";
		exit(0);
	}

	glfwMakeContextCurrent(m_Window);
	glfwSwapInterval(1);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		exit(0);
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	int width, height, channels;
	unsigned char* pixels = stbi_load("icon.png", &width, &height, &channels, 4);

	GLFWimage icon;
	icon.width = width;
	icon.height = height;
	icon.pixels = pixels;

	glfwSetWindowIcon(m_Window, 1, &icon);
	stbi_image_free(pixels);

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	//StockApp setup
	m_DataManager = new DataManager;
	Serializer::DeserializeProducts(m_DataManager, "Products.txt");
	Serializer::DeserializeStockChanges(m_DataManager, "StockChanges.txt");

	m_UIManager = new UIManager;
	m_UIManager->SetDataManager(m_DataManager);
	Serializer::DeserializeSettings(m_DataManager, m_UIManager, "Settings.txt");
}

App::~App()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(m_Window);
	glfwTerminate();

	Serializer::SerializeProducts(m_DataManager, "Products.txt");
	Serializer::SerializeStockChanges(m_DataManager, "StockChanges.txt");
	Serializer::SerializeSettings(m_DataManager, m_UIManager, "Settings.txt");

	delete m_UIManager;
	delete m_DataManager;
}

void App::Run()
{
	while (!glfwWindowShouldClose(m_Window))
	{
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		m_UIManager->Update();

		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(m_Window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(m_Window);
	}
}