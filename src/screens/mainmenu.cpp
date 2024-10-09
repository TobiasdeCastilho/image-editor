#pragma once

#include <iostream>

#include "imgui.h"

inline void _menuoption_file_new() {
	std::cout << "New" << std::endl;
}

inline void append_menu(const int display_w) {
	ImGui::BeginMainMenuBar();		
		ImGui::SetWindowSize(ImVec2(display_w, 20));
		if (ImGui::BeginMenu("File")) {
			if(ImGui::MenuItem("New"))
				std::cout << "New" << std::endl;
			if(ImGui::MenuItem("Open"))
				std::cout << "Open" << std::endl;
			if(ImGui::MenuItem("Save"))
				std::cout << "Save" << std::endl;
			if(ImGui::MenuItem("Save As"))
				std::cout << "Save As" << std::endl;			
			ImGui::EndMenu();
		}			
	ImGui::EndMainMenuBar();
}