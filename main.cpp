// normal includes
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <tuple>
#include "shader.h"
#include <vector>

// imgui includes
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION

// custom includes
#include "camera.h"

// using glm
using glm::vec3;
using glm::vec4;

// using std
using std::string;
using std::vector;
using std::tuple;
using std::cout;
using std::endl;
using std::tie;

struct Sphere
{
    vec3 center;
    float radius;
    vec3 color;
    float pad; // padding to align with GLSL struct
};

static void imgui_end_loop(GLFWwindow* window)
{
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    //glViewport(0, 0, display_w, display_h);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
    glClear(GL_COLOR_BUFFER_BIT);
}

static void imgui_start_loop()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

static void imgui_setup(GLFWwindow* window)
{
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

static GLFWwindow* glfw_setup(int width, int height)
{
    // Setup window
    if (!glfwInit())
        return NULL;

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(width, height, "RayTracing", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(window);
    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return NULL;
    }

    glfwSwapInterval(1); // Enable vsync
    glfwSetErrorCallback(glfw_error_callback);
    glViewport(0, 0, width, height);

    return window;
}

static void fps_counter(GLFWwindow* window, float& lastTime, int& nbFrames)
{
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
}

static void renderScene(GLuint shaderProgram, int width, int height, Camera camera, GLuint sphereBuffer, int numSpheres)
{
    glUseProgram(shaderProgram);

    // Set the uniform variables
    glUniform1i(glGetUniformLocation(shaderProgram, "width"), width);
    glUniform1i(glGetUniformLocation(shaderProgram, "height"), height);
    glUniform1i(glGetUniformLocation(shaderProgram, "numSpheres"), numSpheres);

	// Set the camera uniform variables
	glUniform3fv(glGetUniformLocation(shaderProgram, "center"), 1, &camera.center[0]);
	glUniform3fv(glGetUniformLocation(shaderProgram, "pixel00_loc"), 1, &camera.pixel00_loc[0]);
	glUniform3fv(glGetUniformLocation(shaderProgram, "pixel_delta_u"), 1, &camera.pixel_delta_u[0]);
	glUniform3fv(glGetUniformLocation(shaderProgram, "pixel_delta_v"), 1, &camera.pixel_delta_v[0]);



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

static tuple<int, int>set_bounding_box()
{
    const float ratio = 16.0f / 9.0f;
    const int width = 1280;
    const int height = width / ratio;
	return { width, height };
}

static void cleanup(GLFWwindow* window)
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
}

static Camera createCamera(int width, int height)
{
	// Create a camera
	Camera camera;
	camera.init(width, height);
	return camera;
}

int main()
{
	int width, height;
	std::tie(width, height) = set_bounding_box();

    GLFWwindow* window = glfw_setup(width, height);

    // Setup Dear ImGui context
    imgui_setup(window);

	// Setup camera
	Camera camera = createCamera(width, height);

    // Create a vector for the spheres
    vector<Sphere> spheres = {
        {vec3(0.0f, 0.0f, -3.0f), 1.0f, vec3(1.0f, 1.0f, 0.0f), 0.0f},
        {vec3(2.0f, 0.0f, -3.0f), 2.0f, vec3(0.0f, 1.0f, 1.0f), 0.0f},
        {vec3(-2.0f, 0.0f, -4.0f), 1.0f, vec3(1.0f, 0.0f, 1.0f), 0.0f}

    };



    // Compile and link shaders
    GLuint shaderProgram = createShaderProgram("vertex_shader.glsl", "fragment_shader.glsl");

    // Variables for FPS calculation
    float lastTime = glfwGetTime();
    int nbFrames = 0;

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll for events
        glfwPollEvents();
		imgui_start_loop();
		fps_counter(window, lastTime, nbFrames);

        // demo window
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");

            ImGui::Text("Sphere[0]");

            //ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("sphere[0].x", &spheres[0].center.x, -5.0f, 5.0f);           
            ImGui::SliderFloat("sphere[0].y", &spheres[0].center.y, -5.0f, 5.0f);
            ImGui::SliderFloat("sphere[0].z", &spheres[0].center.z, -5.0f, 5.0f);

            if (ImGui::Button("reset pos"))
                camera.center = vec3(0.0f, 0.0f, 0.0f);
            if (ImGui::Button("reset lookAt"))
				camera.lookat = vec3(0.0f, 0.0f, 0.0f);
			// check if up key is pressed
            
			if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow), true))
				camera.center.z -= 0.1f;
			if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow), true))
				camera.center.z += 0.1f;
			if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow), true))
				camera.center.x -= 0.1f;
			if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow), true))
				camera.center.x += 0.1f;
			if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Space), true))
				camera.center.y -= 0.1f;
			if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftShift), true))
				camera.center.y += 0.1f;
            




            ImGui::Text("Camera Pos");

            ImGui::SliderFloat("Camera.pos.x", &camera.center.x, -5.0f, 5.0f);
			ImGui::SliderFloat("Camera.pos.y", &camera.center.y, 5.0f, -5.0f);
			ImGui::SliderFloat("Camera.pos.z", &camera.center.z, 5.0f, -5.0f);

			ImGui::Text("Camera LookAt");

			ImGui::SliderFloat("Camera.lookAt.x", &camera.lookat.x, -5.0f, 5.0f);
			ImGui::SliderFloat("Camera.lookAt.y", &camera.lookat.y, 5.0f, -5.0f);
			ImGui::SliderFloat("Camera.lookAt.z", &camera.lookat.z, -5.0f, 5.0f);

			camera.update_view();
            //ImGui::SliderFloat("CameraPos.x", )
            
            //ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            //if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            //    counter++;
            //ImGui::SameLine();
            //ImGui::Text("sphere[0].x: %f", spheres[0].center.x);

            //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        // Create and fill the sphere buffer
        GLuint sphereBuffer;
        glGenBuffers(1, &sphereBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, sphereBuffer);
        glBufferData(GL_UNIFORM_BUFFER, spheres.size() * sizeof(Sphere), spheres.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        // Render the scene
        renderScene(shaderProgram, width, height, camera, sphereBuffer, spheres.size());
		imgui_end_loop(window);
    }

    // Cleanup
	cleanup(window);
    return 0;
}