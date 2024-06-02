#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include "shader.h"

// Function to render the scene using the shader program
void renderScene(GLuint shaderProgram, int width, int height) {
    glUseProgram(shaderProgram);

    // Set the uniform variables
    glUniform1i(glGetUniformLocation(shaderProgram, "width"), width);
    glUniform1i(glGetUniformLocation(shaderProgram, "height"), height);

    // Draw a full-screen quad
    glBegin(GL_TRIANGLES);
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(1.0f, -1.0f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(-1.0f, 1.0f);
    glEnd();

    glUseProgram(0);
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Create a GLFW window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Ray Tracing - FPS: ", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the OpenGL context current
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Set the viewport
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // Compile and link shaders
    GLuint shaderProgram = createShaderProgram("vertex_shader.glsl", "fragment_shader.glsl");

    // Variables for FPS calculation
    double lastTime = glfwGetTime();
    int nbFrames = 0;

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Measure the time
        double currentTime = glfwGetTime();
        nbFrames++;

        // If one second has passed, update the window title with the FPS
        if (currentTime - lastTime >= 1.0) {
            int fps = double(nbFrames) / (currentTime - lastTime);
            std::string title = "Ray Tracing - FPS: " + std::to_string(fps);
            glfwSetWindowTitle(window, title.c_str());
            nbFrames = 0;
            lastTime = currentTime;
        }

        // Render the scene
        renderScene(shaderProgram, width, height);

        // Swap buffers
        glfwSwapBuffers(window);

        // Poll for events
        glfwPollEvents();
    }

    // Cleanup
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
