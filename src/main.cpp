#include "../../src/engine/renderer/Window.h"
#include "glm/fwd.hpp"
#include "stb_image.h"
#include <iostream>
#include <vector>

const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1000;
unsigned int loadTexture(char const *path);
bool isPaused = false;
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
#include <filesystem>
#include <iostream>

int main()
{
  Window *win = new Window(SCR_WIDTH, SCR_HEIGHT, "ForgeGL");

  Engine e;
  glfwSetWindowUserPointer(win->window, &e);

  e.run(win->window, win->screenWidth, win->screenHeight);

  win->terminateWindow();
  return 0;
}