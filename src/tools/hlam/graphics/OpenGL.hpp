#pragma once

#include <GL/glew.h>

/**
*	As specified by the OpenGL standard, 0 is not a valid texture id.
*/
const GLuint GL_INVALID_TEXTURE_ID = 0;

void glDeleteTexture(GLuint& textureId);

const char* glErrorToString(const GLenum error);

const char* glFrameBufferStatusToString(const GLenum status);
