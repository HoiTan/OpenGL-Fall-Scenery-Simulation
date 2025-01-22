#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <unordered_map>

// Include GLFW for window and input
#include <GLFW/glfw3.h>

// If using GLEW for modern OpenGL functions:
#include <GL/glew.h>

// If on Windows, you may need <Windows.h> before GL includes
// #include <Windows.h>

// Include GLM for math operations
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Your own headers
#include "LSystem.hpp"
#include "Turtle.hpp"

// Shader loading utility (you would implement these functions)
GLuint LoadShader(const char* vertexPath, const char* fragmentPath);

// Window dimensions
const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 600;

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "L-System", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW (if used)
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        std::cerr << "GLEW initialization error: " << glewGetErrorString(err) << "\n";
        return -1;
    }

    // Define the L-system
    std::string axiom = "A";
    std::unordered_map<char, std::string> rules = {
        {'A', "F[+A][-A]"}
    };
    LSystem lsystem(axiom, rules, 5);
    std::string finalString = lsystem.generate();

    // Interpret L-system string into line vertices
    Turtle turtle;
    turtle.setAngle(25.0f);
    turtle.setStep(0.5f);

    std::vector<glm::vec3> lineVertices;
    turtle.interpret(finalString, lineVertices);

    // Create VAO, VBO for line data
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, lineVertices.size() * sizeof(glm::vec3), &lineVertices[0], GL_STATIC_DRAW);

    // Assume shader uses location 0 for position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    glBindVertexArray(0);

    // Load shaders
    GLuint shaderProgram = LoadShader("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");

    // Enable depth test if desired
    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);

        // Setup camera
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH/(float)HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(glm::vec3(0, -20, 20), glm::vec3(0,0,0), glm::vec3(0,0,1));
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 MVP = projection * view * model;

        GLint mvpLoc = glGetUniformLocation(shaderProgram, "uMVP");
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &MVP[0][0]);

        glBindVertexArray(VAO);
        glDrawArrays(GL_LINES, 0, (GLsizei)lineVertices.size());
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
