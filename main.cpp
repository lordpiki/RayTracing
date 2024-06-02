#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include "shader.h"

using glm::vec3;

struct Sphere
{
    vec3 color;
	vec3 center;
	float radius;
};

// Function to render the scene using the shader program
void renderScene(GLuint shaderProgram, int width, int height)
{
    glUseProgram(shaderProgram);

	Sphere sphere;
	sphere.color = vec3(1.0f, 1.0f, 0.0f);
	sphere.center = vec3(0.0f, 0.0f, -3.0f);
	sphere.radius = 1.0f;

    // Set the uniform variables
    glUniform1i(glGetUniformLocation(shaderProgram, "width"), width);
    glUniform1i(glGetUniformLocation(shaderProgram, "height"), height);
    glUniform3fv(glGetUniformLocation(shaderProgram, "sphereColor"), 1, &sphere.color[0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "sphereCenter"), 1, &sphere.center[0]);
    glUniform1f(glGetUniformLocation(shaderProgram, "sphereRadius"), sphere.radius);

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
