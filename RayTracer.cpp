#include "RayTracer.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include "camera.h"


RayTracer::RayTracer(int width, int height)
    : m_width(width), m_height(height), m_outputTexture(0) {}

RayTracer::~RayTracer() {
    if (m_outputTexture != 0) {
        glDeleteTextures(1, &m_outputTexture);
    }
    if (m_quadVAO != 0) {
        glDeleteVertexArrays(1, &m_quadVAO);
    }
    if (m_quadVBO != 0) {
        glDeleteBuffers(1, &m_quadVBO);
    }
}

bool RayTracer::initialize() {
    if (!loadComputeShader()) {
        return false;
    }

    if (!loadRenderShader()) {
        return false;
    }

    // Create texture for compute shader output
    glGenTextures(1, &m_outputTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_outputTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(0, m_outputTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    // Create a quad for rendering the texture
    float quadVertices[] = {
        // positions        // texture Coords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };
    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);
    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    return true;
}

void RayTracer::render() {
    // Dispatch compute shader
    m_computeShader.use();
    glDispatchCompute(m_width / 16, m_height / 16, 1);

    // Make sure writing to image has finished before read
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // Render the output texture
    glClear(GL_COLOR_BUFFER_BIT);

    m_renderShader.use();
    glBindVertexArray(m_quadVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_outputTexture);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

bool RayTracer::loadComputeShader() {
    std::ifstream shaderFile("computeShader.glsl");
    if (!shaderFile.is_open()) {
        std::cerr << "Failed to open compute shader file" << std::endl;
        return false;
    }

    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    shaderFile.close();

    std::string shaderCode = shaderStream.str();
    return m_computeShader.compileComputeShader(shaderCode);
}

bool RayTracer::loadRenderShader() {
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aTexCoords;
        out vec2 TexCoords;
        void main() {
            TexCoords = aTexCoords;
            gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        in vec2 TexCoords;
        uniform sampler2D screenTexture;
        void main() {
            FragColor = texture(screenTexture, TexCoords);
        }
    )";

    return m_renderShader.compileShaders(vertexShaderSource, fragmentShaderSource);
}


void RayTracer::updateCamera(const Camera& camera) {
    m_computeShader.use();
    // Set the camera uniform variables
    glUniform3fv(glGetUniformLocation(m_computeShader.getProgram(), "center"), 1, &camera.center[0]);
    glUniform3fv(glGetUniformLocation(m_computeShader.getProgram(), "pixel00_loc"), 1, &camera.pixel00_loc[0]);
    glUniform3fv(glGetUniformLocation(m_computeShader.getProgram(), "pixel_delta_u"), 1, &camera.pixel_delta_u[0]);
    glUniform3fv(glGetUniformLocation(m_computeShader.getProgram(), "pixel_delta_v"), 1, &camera.pixel_delta_v[0]);

}

void RayTracer::updateSpheres(const std::vector<Sphere>& spheres) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_sphereBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, spheres.size() * sizeof(Sphere), spheres.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_sphereBuffer);
}

void RayTracer::setMaxDepth(int depth) {
    m_maxDepth = depth;
    m_computeShader.use();
    glUniform1i(glGetUniformLocation(m_computeShader.getProgram(), "maxDepth"), m_maxDepth);
}

void RayTracer::setRaysPerPixel(int rays) {
    m_raysPerPixel = rays;
    m_computeShader.use();
    glUniform1i(glGetUniformLocation(m_computeShader.getProgram(), "raysPerPixel"), m_raysPerPixel);
}