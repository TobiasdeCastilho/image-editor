#pragma once

#include <iostream>
#include <cstdlib>

#include "imgui.h"

#include "layers.cpp"

Editor::GroupLayer manager;

namespace Editor {
	inline void _menuoption_file_new() {
		std::cout << "New" << std::endl;
	}

	inline void append_new_layer_menu() {		
		ImGui::Text("New layer");		
		if(ImGui::Button("Filter"))
			manager.add(FILTER);		
		ImGui::SameLine();
		if(ImGui::Button("Points"))
			manager.add(POINT);
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
		ImGui::Combo(("##" + id + "filter").c_str(), (int*) layer.filter, "None\0Blur\0Contour\0Edge\0Emboss\0Sharpen\0Threshold\0Gray\0");
		
		ImGui::Text("Intensity:");
		ImGui::SameLine();
		ImGui::SliderInt(("##" + id + "intensity").c_str(), layer.intensity, 0, 100, "%d", ImGuiSliderFlags_AlwaysClamp);		
	}

	inline void append_image_control(ImageLayer layer, std::string id) {
		ImGui::Text("Image...: %s", layer.get_file().c_str());				
		ImGui::Text("Position:");		
		ImGui::SliderInt(("X##" + id + "pos_x").c_str(), layer.pos_x, -*layer.width, *layer.width, "%d", ImGuiSliderFlags_AlwaysClamp);		
		ImGui::SliderInt(("y##" + id + "pos_y").c_str(), layer.pos_y, -*layer.height, *layer.height, "%d", ImGuiSliderFlags_AlwaysClamp);
		//ImGui::Text("Size:");
		//ImGui::SliderInt(("Width##" + id + "width").c_str(), layer.width, 0, 1280, "%d", ImGuiSliderFlags_AlwaysClamp);
		//ImGui::SliderInt(("Height##" + id + "height").c_str(), layer.height, 0, 1280, "%d", ImGuiSliderFlags_AlwaysClamp);
		ImGui::Text("Rotation:");
		ImGui::SliderInt(("##" + id + "rotation").c_str(), layer.rotation, 0, 360, "%d", ImGuiSliderFlags_AlwaysClamp);
		ImGui::Text("Opacity:");
		ImGui::SliderInt(("##" + id + "opacity").c_str(), layer.opacity, 0, 100, "%d", ImGuiSliderFlags_AlwaysClamp);
		ImGui::Text("Scale:");
		ImGui::SliderFloat(("##" + id + "scale").c_str(), layer.scale, 0.1, 1, "%.1f", ImGuiSliderFlags_AlwaysClamp);		
		if(ImGui::Button(("Resetart##" + id).c_str()))
			layer.reset();
	}

	inline void append_point_control(Editor::PointLayer& layer, const std::string& id) {
    // Ponto 1
    ImGui::Text("Ponto 1");
    ImGui::SliderInt(("X##1" + id + "pos_x").c_str(), &layer.points[0].x, -1, work_width, "%d", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SliderInt(("Y##1" + id + "pos_y").c_str(), &layer.points[0].y, -1, work_height, "%d", ImGuiSliderFlags_AlwaysClamp);
    
    // Ponto 2
    ImGui::Text("Ponto 2");
    ImGui::SliderInt(("X##2" + id + "pos_x").c_str(), &layer.points[1].x, -1, work_width, "%d", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SliderInt(("Y##2" + id + "pos_y").c_str(), &layer.points[1].y, -1, work_height, "%d", ImGuiSliderFlags_AlwaysClamp);
    
    // Ponto 3
    ImGui::Text("Ponto 3");
    ImGui::SliderInt(("X##3" + id + "pos_x").c_str(), &layer.points[2].x, -1, work_width, "%d", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SliderInt(("Y##3" + id + "pos_y").c_str(), &layer.points[2].y, -1, work_height, "%d", ImGuiSliderFlags_AlwaysClamp);
    
    // Ponto 4
    ImGui::Text("Ponto 4");
    ImGui::SliderInt(("X##4" + id + "pos_x").c_str(), &layer.points[3].x, -1, work_width, "%d", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SliderInt(("Y##4" + id + "pos_y").c_str(), &layer.points[3].y, -1, work_height, "%d", ImGuiSliderFlags_AlwaysClamp);

		ImGui::Text("Rotation:");
		ImGui::SliderInt(("##" + id + "rotation").c_str(), layer.rotation, 0, 360, "%d", ImGuiSliderFlags_AlwaysClamp);

		ImGui::Text("Scale:");
		ImGui::SliderFloat(("##" + id + "scale").c_str(), layer.scale, 0.1, 1, "%.1f", ImGuiSliderFlags_AlwaysClamp);

		if(ImGui::Button(("Resetart##" + id).c_str()))
			layer.reset();
}

	inline void append_group_control(GroupLayer group, std::string id, GroupLayer *parent = nullptr) {
		ImGui::Separator();
		
		group.list([](int pos, Editor::Layer* layer, void* data) {																		
			const std::string id = std::to_string(pos);
			
			switch (layer->get_type()) {
				case FILTER:
					append_filtter_control(dynamic_cast<FilterLayer&>(*layer), id);
					break;
				case IMAGE:
					append_image_control(dynamic_cast<ImageLayer&>(*layer), id);
					break;
				case GROUP:
					append_group_control(dynamic_cast<GroupLayer&>(*layer), id, (GroupLayer *) data);					
					break;				
				case POINT:
					append_point_control(dynamic_cast<PointLayer&>(*layer), id);
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
