#pragma once

#include <GLES2/gl2.h>
#include <GL/glew.h>	

void cvmat_to_gluint(const cv::Mat mat, GLuint* texture_id)
{		     
	if (texture_id == nullptr)
		texture_id = new GLuint;		

	glGenTextures(1, texture_id);
	glBindTexture(GL_TEXTURE_2D, *texture_id);
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
	glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mat.cols, mat.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, mat.data);  
}