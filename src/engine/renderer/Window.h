#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include "../core/Engine.h"

class Window {
private:
public:
  GLFWwindow *window;
  unsigned int screenWidth = 1600;
  unsigned int screenHeight = 1000;
  const char *title = "Game";

  Window(unsigned int sW, unsigned int sH, const char *sT);
  void terminateWindow();
};

Window::Window(unsigned int sW, unsigned int sH, const char *sT) {
  this->screenWidth = sW;
  this->screenHeight = sH;
  this->title = sT;

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // creating window
  window = glfwCreateWindow(screenWidth, screenHeight, title, NULL, NULL);
  glfwSetCursorPosCallback(window, Engine::mouse_callback);

  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
  }
  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
  }
}

void Window::terminateWindow() {
  glfwDestroyWindow(window);
  glfwTerminate(); 
}
