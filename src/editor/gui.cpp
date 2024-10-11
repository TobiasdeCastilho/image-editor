#pragma once

#include <iostream>
#include <cstdlib>

#include "imgui.h"

#include "layers.cpp"

namespace Editor {
	inline void _menuoption_file_new() {
		std::cout << "New" << std::endl;
	}

	inline void append_new_layer_menu(GroupLayer &manager) {		
		ImGui::Text("New layer");		
		if(ImGui::Button("Filter"))
			manager.add(FILTER);
		ImGui::SameLine();
		if(ImGui::Button("Group"))
			manager.add(GROUP);
		ImGui::SameLine();
		if(ImGui::Button("Image"))
			manager.add(IMAGE);
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

	inline void append_filtter_control(FilterLayer layer, std::string id){						
		ImGui::Text("Filter...:");
		ImGui::SameLine();
		ImGui::Combo(("##" + id + "filter").c_str(), (int*) layer.filter, "None\0Blur\0Contour\0Edge\0Emboss\0Sharpen\0Threshold\0");
		
		ImGui::Text("Intensity:");
		ImGui::SameLine();
		ImGui::SliderInt(("##" + id + "intensity").c_str(), layer.intensity, 0, 100, "%d", ImGuiSliderFlags_AlwaysClamp);		
	}

	inline void append_image_control(ImageLayer layer, std::string id) {
		ImGui::Text("Image...: %s", layer.get_file().c_str());				
		ImGui::Text("Position:");		
		ImGui::SliderInt(("X##" + id + "pos_x").c_str(), layer.pos_x, 0, work_width, "%d", ImGuiSliderFlags_AlwaysClamp);		
		ImGui::SliderInt(("y##" + id + "pos_y").c_str(), layer.pos_y, 0, work_height, "%d", ImGuiSliderFlags_AlwaysClamp);
		ImGui::Text("Size:");
		ImGui::SliderInt(("Width##" + id + "width").c_str(), layer.width, 0, 1280, "%d", ImGuiSliderFlags_AlwaysClamp);
		ImGui::SliderInt(("Height##" + id + "height").c_str(), layer.height, 0, 1280, "%d", ImGuiSliderFlags_AlwaysClamp);
	}

	inline void append_group_control(GroupLayer group, std::string id, GroupLayer *parent = nullptr) {
		ImGui::Separator();
		
		group.list([](int pos, Editor::Layer* layer, void* data) {																		
			const std::string id = std::to_string(pos);
			
			switch (layer->get_type()) {
				case Editor::FILTER:
					Editor::append_filtter_control(dynamic_cast<Editor::FilterLayer&>(*layer), id);
					break;
				case Editor::IMAGE:
					Editor::append_image_control(dynamic_cast<Editor::ImageLayer&>(*layer), id);
					break;
				case Editor::GROUP:
					Editor::append_group_control(dynamic_cast<Editor::GroupLayer&>(*layer), id, (GroupLayer *) data);					
					break;				
			}

			if(ImGui::Button(std::string(std::string((layer->get_active() ? "Deactivate" : "Activate")) + "##" + id).c_str()))
				layer->set_active(!layer->get_active());											

			ImGui::Separator();
		}, &group);				

		if(parent != nullptr) {
			if(ImGui::Button(("Remove##" + id).c_str()))
				parent->remove(std::stoi(id));
			ImGui::SameLine();			
			if(ImGui::Button(("Filter##" + id).c_str()))
				group.add(FILTER);		
			ImGui::SameLine();
			if(ImGui::Button(("Image##" + id).c_str()))
				group.add(IMAGE);
		}

		ImGui::Separator();
	}
}
