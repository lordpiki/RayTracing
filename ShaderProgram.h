#pragma once
#include <GL/glew.h>
#include <string>

class ShaderProgram {
public:
    ShaderProgram();
    ~ShaderProgram();

    bool compileComputeShader(const std::string& source);
    bool compileShaders(const std::string& vertexSource, const std::string& fragmentSource);
    void use();
    GLuint getProgram() const { return m_program; }

private:
    GLuint m_program;
    bool compileShader(const std::string& source, GLenum shaderType, GLuint& shader);
};