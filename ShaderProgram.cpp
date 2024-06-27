#include "ShaderProgram.h"
#include <iostream>

ShaderProgram::ShaderProgram() : m_program(0) {}

ShaderProgram::~ShaderProgram() {
    if (m_program != 0) {
        glDeleteProgram(m_program);
    }
}

bool ShaderProgram::compileComputeShader(const std::string& source) {
    GLuint computeShader;
    if (!compileShader(source, GL_COMPUTE_SHADER, computeShader)) {
        return false;
    }

    m_program = glCreateProgram();
    glAttachShader(m_program, computeShader);
    glLinkProgram(m_program);

    GLint success;
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(m_program, 512, NULL, infoLog);
        std::cerr << "Compute program linking failed:\n" << infoLog << std::endl;
        glDeleteShader(computeShader);
        glDeleteProgram(m_program);
        m_program = 0;
        return false;
    }

    glDeleteShader(computeShader);
    return true;
}

bool ShaderProgram::compileShaders(const std::string& vertexSource, const std::string& fragmentSource) {
    GLuint vertexShader, fragmentShader;
    if (!compileShader(vertexSource, GL_VERTEX_SHADER, vertexShader)) {
        return false;
    }
    if (!compileShader(fragmentSource, GL_FRAGMENT_SHADER, fragmentShader)) {
        glDeleteShader(vertexShader);
        return false;
    }

    m_program = glCreateProgram();
    glAttachShader(m_program, vertexShader);
    glAttachShader(m_program, fragmentShader);
    glLinkProgram(m_program);

    GLint success;
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(m_program, 512, NULL, infoLog);
        std::cerr << "Shader program linking failed:\n" << infoLog << std::endl;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(m_program);
        m_program = 0;
        return false;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return true;
}

void ShaderProgram::use() {
    glUseProgram(m_program);
}

bool ShaderProgram::compileShader(const std::string& source, GLenum shaderType, GLuint& shader) {
    shader = glCreateShader(shaderType);
    const char* sourcePtr = source.c_str();
    glShaderSource(shader, 1, &sourcePtr, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "Shader compilation failed:\n" << infoLog << std::endl;
        glDeleteShader(shader);
        return false;
    }

    return true;
}