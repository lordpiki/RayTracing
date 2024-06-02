#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <string>

// Function to read shader source code from a file
std::string readShaderSource(const char* filepath);

// Function to compile a shader
GLuint compileShader(GLenum type, const char* source);

// Function to create a shader program
GLuint createShaderProgram(const char* vertexFilePath, const char* fragmentFilePath);

#endif // SHADER_H
