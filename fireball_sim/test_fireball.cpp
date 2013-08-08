/**
 * Author: Rajesh
 * Description: test Fireball.hpp and Fireball.cpp.
 */

#include <stdlib.h>
#include <time.h>
#include <GLFW/glfw3.h>

#include "weapon_controlling_interfaces.h"

int main(void)
{
  GLFWwindow* window;

  if (!glfwInit())
    exit(EXIT_FAILURE);

  window = glfwCreateWindow(640, 480, "fireball_sim", NULL, NULL);

  if (!window){
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);

  glEnable(GL_BLEND);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  wd_add(ao::FIREBALL, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.05f, 1.5f);

  while (!glfwWindowShouldClose(window)){
    float ratio;
    int width, height;

    glfwGetFramebufferSize(window, &width, &height);
    ratio = width / (float) height;

    glViewport(0, 0, width, height);

    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-2.0*ratio, 2.0*ratio, -2.f, 2.f, 2.f, -2.f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    wd_update();
    wd_draw();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  wd_destroy();

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
