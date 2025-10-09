#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

// Vertex shader
const char *vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;

out vec3 vertexColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vertexColor = aColor;
}
)";

// Fragment shader
const char *fragmentShaderSource = R"(
#version 330 core
in vec3 vertexColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(vertexColor, 1.0);
}
)";

// Resize callback
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

int main() {
  // --- Initialize GLFW ---
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW\n";
    return -1;
  }
  GLFWwindow *window = glfwCreateWindow(800, 600, "Rotating Cube", NULL, NULL);
  if (!window) {
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  // --- Initialize GLEW ---
  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize GLEW\n";
    return -1;
  }

  // --- Enable depth testing ---
  glEnable(GL_DEPTH_TEST);

  // --- Cube vertex data (positions + colors) ---
  float vertices[] = {
      // positions           // colors
      -0.5f, -0.5f, -0.5f, 1.0f,  0.0f, 0.0f, // back face
      0.5f,  -0.5f, -0.5f, 0.0f,  1.0f, 0.0f,  0.5f, 0.5f, -0.5f,
      0.0f,  0.0f,  1.0f,  -0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.0f,

      -0.5f, -0.5f, 0.5f,  1.0f,  0.0f, 1.0f, // front face
      0.5f,  -0.5f, 0.5f,  0.0f,  1.0f, 1.0f,  0.5f, 0.5f, 0.5f,
      1.0f,  1.0f,  1.0f,  -0.5f, 0.5f, 0.5f,  0.5f, 0.5f, 0.5f};

  unsigned int indices[] = {
      0, 1, 2, 2, 3, 0, // back
      4, 5, 6, 6, 7, 4, // front
      0, 4, 7, 7, 3, 0, // left
      1, 5, 6, 6, 2, 1, // right
      3, 2, 6, 6, 7, 3, // top
      0, 1, 5, 5, 4, 0  // bottom
  };

  unsigned int VAO, VBO, EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);

  // Position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // Color attribute
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // --- Compile shaders ---
  auto compileShader = [](GLenum type, const char *source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      char infoLog[512];
      glGetShaderInfoLog(shader, 512, NULL, infoLog);
      std::cerr << "Shader compile error: " << infoLog << std::endl;
    }
    return shader;
  };

  unsigned int vertexShader =
      compileShader(GL_VERTEX_SHADER, vertexShaderSource);
  unsigned int fragmentShader =
      compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

  unsigned int shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  // --- Get uniform locations ---
  int modelLoc = glGetUniformLocation(shaderProgram, "model");
  int viewLoc = glGetUniformLocation(shaderProgram, "view");
  int projLoc = glGetUniformLocation(shaderProgram, "projection");

  // --- Main loop ---
  while (!glfwWindowShouldClose(window)) {
    // Input
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(window, true);

    // Render
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use shader
    glUseProgram(shaderProgram);

    // Model: rotation
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f),
                        glm::vec3(1.0f, 1.0f, 0.0f));

    // View: move back a bit
    glm::mat4 view =
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));

    // Projection: perspective
    glm::mat4 projection =
        glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    // Send uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Draw cube
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // --- Cleanup ---
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
  glDeleteProgram(shaderProgram);
  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
