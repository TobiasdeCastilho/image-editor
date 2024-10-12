#pragma once

#include <iostream>
#include <string>
#include <cstdlib>
#include <GLES2/gl2.h>
#include <GL/glew.h>	

std::string _dialog_file() {
  char file_name[1024] = {0};
  FILE *f = popen("zenity --file-selection --title=\"Select a File\"", "r");

  if (f) {
    fgets(file_name, sizeof(file_name), f);
    pclose(f);
  }
  
  std::string file = file_name;
  if (!file.empty() && file[file.size() - 1] == '\n') {
    file.erase(file.size() - 1);
  }

  return file;
}

void cvmat_to_gluint(const cv::Mat mat, GLuint* texture_id) {		     
	if (texture_id == nullptr)
		texture_id = new GLuint;

	glGenTextures(1, texture_id);
	glBindTexture(GL_TEXTURE_2D, *texture_id);
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
	glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mat.cols, mat.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, mat.data);  
}

void overlay(cv::Mat& main, const cv::Mat& overlay, int pos_x, int pos_y, int opacity) {    
  int overlay_start_x = std::max(0, -pos_x);
  int overlay_start_y = std::max(0, -pos_y);

  int main_start_x = std::max(0, pos_x);
  int main_start_y = std::max(0, pos_y);

  int width = std::min(overlay.cols - overlay_start_x, main.cols - main_start_x);
  int height = std::min(overlay.rows - overlay_start_y, main.rows - main_start_y);

  if (width <= 0 || height <= 0)
    return;

  cv::Rect roi_main(main_start_x, main_start_y, width, height);
  cv::Rect roi_overlay(overlay_start_x, overlay_start_y, width, height);

  cv::Mat main_roi = main(roi_main);
  cv::Mat overlay_roi = overlay(roi_overlay);		

  cv::addWeighted(main_roi, 1.0 - (opacity / 100.0), overlay_roi, opacity / 100.0, 0.0, main_roi);

}
