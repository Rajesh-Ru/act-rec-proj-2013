/**
 * Author: Rajesh
 * Description: an action recognition program.
 */

#include "rad_interfaces.hpp"

void* fireballCompleteCB(void* userID)
{
  printf("User %d unleashed a FIRE BALL!\n", (XnUserID)userID);
}

// called by GLFW when the framebuffer is resized
void resizeCB(GLFWwindow* win, int w, int h)
{
  glViewport(0, 0, w, h);
}

int main(int argc, char** argv)
{
  nipub_init(CONFIG_FILE_NAME);
  nipub_subscribe(fba_notify);
  fba_registerCompleteCB(fireballCompleteCB);
  nipub_subscribe(sd_notify);

  GLFWwindow* win;

  if (!glfwInit())
    return -1;

  win = glfwCreateWindow(640, 480, "act_rec", NULL, NULL);
  if (!win){
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(win);
  glfwSetFramebufferSizeCallback(win, resizeCB);

  while(!glfwWindowShouldClose(win)){
    glClear(GL_COLOR_BUFFER_BIT);

    nipub_spin(NULL);

    glfwSwapBuffers(win);
    glfwPollEvents();
  }

  glfwDestroyWindow(win);
  glfwTerminate();

  sd_destroy();
  nipub_destroy();
  pthread_exit(NULL);
  return 0;
}
