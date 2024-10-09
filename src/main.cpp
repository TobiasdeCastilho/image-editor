#include <iostream>

#include <opencv4/opencv2/opencv.hpp>

#define GL_SILENCE_DEPRECATION
#include <GLES2/gl2.h>
#include <GLFW/glfw3.h>

//submodules
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

//project
#include "utils/conversion.cpp"
#include "screens/mainmenu.cpp"
#include "classes/screens/image.cpp"

void prepare(GLFWwindow *window)
{
	const char *glsl_version = "#version 130";

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();	
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
}

void close(GLFWwindow *window)
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
}

bool config_opened = false;

IMG::ImageManager manager;

int main(int, char **)
{
	glfwSetErrorCallback(
		[](int error, const char *description) {
			fprintf(stderr, "GLFW Error %d: %s\n", error, description);
		}
	);	
	if (!glfwInit()) exit(1);

	int display_w = 1280, display_h = 720;
	GLFWwindow *window = glfwCreateWindow(display_w, display_h, "Client", nullptr, nullptr);	
	if (window == nullptr) exit(1);
	prepare(window);		

	std::string image_path = "/home/tobias/Imagens/Capturas de tela/abapuru.png";		
	manager.layers_add(IMG::FILTER, "BLUR");
	manager.layers_add(IMG::FILTER, "SHARPEN");
	manager.load_image(image_path);
		
	manager.list_layers([](std::string name, IMG::Layer* layer, int *count) {		
		IMG::FilterLayer* filter = dynamic_cast<IMG::FilterLayer*>(layer);	
			std::cout << name << std::endl;
		if(*count == 0)
			filter->set_filter(IMG::BLUR);					
		else
			filter->set_filter(IMG::SHARPEN);
	});
	manager.process_layers();
	
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
			ImGui_ImplGlfw_Sleep(10);
			continue;
		}

		glfwGetWindowSize(window, &display_w, &display_h);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();							    		

		append_menu(display_w);

		ImGui::Begin("left", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
		ImGui::SetWindowSize(ImVec2(40, display_h - 20));
		ImGui::SetWindowPos(ImVec2(0,20));		
		ImGui::End();	

		ImGui::Begin("right", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
		ImGui::SetWindowSize(ImVec2(300, display_h - 20));							
		ImGui::SetWindowPos(ImVec2(display_w - 300, 20));
		
		manager.list_layers([](std::string name, IMG::Layer* layer, int *count) {		
			ImGui::Text("%s", name.c_str());							
			ImGui::SameLine();

			std::string button_name = std::string((layer->get_active() ? "Deactivate" : "Activate")) + "##" + name;

			if(ImGui::Button(button_name.c_str())) {			
				layer->set_active(!layer->get_active());
				manager.process_layers();				
			}
		});		
		ImGui::End();
		
		ImGui::Begin("main", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);						
		ImGui::SetWindowSize(ImVec2(display_w - 340, display_h - 20));
		ImGui::SetWindowPos(ImVec2(40,20));								
		ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() / 2 - 300, ImGui::GetWindowHeight() / 2 - 300));		
		ImGui::Image((void*)(intptr_t)manager.get_texture(), ImVec2(600, 600));
		ImGui::End();

    ImGui::Render();    
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	close(window);

	return 0;
}