/**
 * Author: Rajesh
 * Description: an action recognition program.
 */

#include <GLFW/glfw3.h>

#include "RADFactory.hpp"

static void fireBallCompleteCB(rad::RADenum type, int size, const void* pParam)
{
  if (type != rad::RAD_SKEL_INFO)
    return;

  const rad::SkeletonInfo* pSkel = (const rad::SkeletonInfo*)pParam;

  printf("User %d unleashed a FIRE BALL!\n", pSkel->user);
}

// called by GLFW when the framebuffer is resized
void resizeCB(GLFWwindow* win, int w, int h)
{
  glViewport(0, 0, w, h);
}

int main(int argc, char** argv)
{
  rad::SensorDataPub* pPub = rad::RADFactory::getPtrToInst()->
    getPtrToPub(rad::RAD_PUB_TYPE_KINECT_DATA);
  rad::Action* pAction = (rad::Action*)rad::RADFactory::getPtrToInst()->
    getPtrToSub(rad::RAD_SUB_TYPE_FIREBALL_ACT);
  rad::SceneDrawer* pDrawer =
    (rad::SceneDrawer*)rad::RADFactory::getPtrToInst()->
    getPtrToSub(rad::RAD_SUB_TYPE_SCENE_DRAWER);

  pAction->registerCompleteCB(fireBallCompleteCB);
  pPub->subscribe(pAction);
  pPub->subscribe(pDrawer);

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

    pPub->spinOnce();

    glfwSwapBuffers(win);
    glfwPollEvents();
  }

  glfwDestroyWindow(win);
  glfwTerminate();

  rad::RADFactory::getPtrToInst()->release(rad::RAD_PUB_TYPE_KINECT_DATA,
					   NULL);
  rad::RADFactory::getPtrToInst()->release(rad::RAD_SUB_TYPE_FIREBALL_ACT,
					   pAction);
  rad::RADFactory::releaseInst();

  return 0;
}
