#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "shader.h"
#include <vector>


using std::vector;
using glm::vec3;
using glm::vec4;

struct Material
{
    vec3 color;
    float emission_strength;
    vec3 emmision_color;
    float reflection_strength;
};

struct Sphere
{
    vec3 center;
    float radius;
    Material material;
};

struct Camera
{
    vec3 camera_center;
    float focal_length;
    float viewport_height;
};


// Callback function for handling scroll events
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    // Update camera focal length based on scroll direction
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
	camera->focal_length += yoffset * 0.1f;
}

// Callback function for handling cursor position events
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    // Update camera position based on cursor movement
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        static double lastX = xpos, lastY = ypos;

        double deltaX = - ( xpos - lastX );
        double deltaY = ypos - lastY;

        // Adjust camera_center based on cursor movement
        camera->camera_center += vec3(deltaX * 0.01f, -deltaY * 0.01f, 0.0f);

        lastX = xpos;
        lastY = ypos;
    }

    // Update camera position based on cursor movement
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        static double lastX = xpos, lastY = ypos;
        Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));

        double deltaX = - ( xpos - lastX );
        double deltaY = ypos - lastY;

        // Rotate camera_center based on cursor movement
        float rotationSpeed = 0.01f;
        float angleX = deltaX * rotationSpeed;
        float angleY = deltaY * rotationSpeed;

        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angleX, glm::vec3(0.0f, 1.0f, 0.0f));
        rotationMatrix = glm::rotate(rotationMatrix, angleY, glm::vec3(1.0f, 0.0f, 0.0f));

        glm::vec4 newCameraCenter = rotationMatrix * glm::vec4(camera->camera_center, 1.0f);
        camera->camera_center = glm::vec3(newCameraCenter);

        lastX = xpos;
        lastY = ypos;
    }
}

// Function to render the scene using the shader program
void renderScene(GLuint shaderProgram, int width, int height, GLuint sphereBuffer, int numSpheres, Camera camera)
{
    glUseProgram(shaderProgram);

    // Set the uniform variables
    // pass the screen vars
    glUniform1i(glGetUniformLocation(shaderProgram, "width"), width);
    glUniform1i(glGetUniformLocation(shaderProgram, "height"), height);

    // pass the objects
    glUniform1i(glGetUniformLocation(shaderProgram, "numSpheres"), numSpheres);

    // pass the camera vars
    glUniform1f(glGetUniformLocation(shaderProgram, "focal_length_in"), camera.focal_length);
    glUniform1f(glGetUniformLocation(shaderProgram, "viewport_height_in"), camera.viewport_height);
    glUniform3fv(glGetUniformLocation(shaderProgram, "camera_center_in"), 1, &camera.camera_center[0]);

    // Bind the sphere buffer
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, sphereBuffer);

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

vector<Sphere> spheresSetup()
{
    Sphere sun = { vec3(50.0f, -101.0f, -105.0f), 100.0f, vec3(0.0f, 0.0f, 1.0f), 5.0f, vec3(1.0f, 1.0f, 1.0f),  0.5f };

    // Create a vector for the spheres
    vector<Sphere> spheres = {
        {vec3(0.0f, 0.0f, -3.0f), 1.0f, vec3(1.0f, 0.0f, 0.0f), 0.0f, vec3(0, 0, 0),  0.0f},
        {vec3(-1.5f, 0.0f, -2.0f), 0.5f, vec3(1.0f, 1.0f, 0.0f), 0.0f, vec3(0, 0, 0),  0.0f},
        {vec3(-2.0f, 10.5f, -4.0f), 10.0f, vec3(0.5f, 0.0f, 0.5f), 0.0f, vec3(0, 0, 0),  0.0f},
    };

    spheres.push_back(sun);
    return spheres;
}


Camera cameraSetup()
{
    Camera camera = { vec3(0.0f, 0.0f, 0.0f), 1.0f, 2.0f };
    return camera;
}



int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Create a GLFW window
    GLFWwindow* window = glfwCreateWindow(1200, 900, "Ray Tracing - FPS: ", NULL, NULL);
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


    vector<Sphere> spheres = spheresSetup();
    Camera camera = cameraSetup();

    glfwSetWindowUserPointer(window, &camera);

    // Set up callback functions
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    // Create and fill the sphere buffer
    GLuint sphereBuffer;
    glGenBuffers(1, &sphereBuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, sphereBuffer);
    glBufferData(GL_UNIFORM_BUFFER, spheres.size() * sizeof(Sphere), spheres.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

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
        renderScene(shaderProgram, width, height, sphereBuffer, spheres.size(), camera);

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
