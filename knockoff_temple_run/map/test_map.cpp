/**
 * Author: Rajesh
 * Description: an integrated test for the map module.
 */

#define __USE_BSD
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>
#include <iostream>

#include "../common/common.h"
#include "map_interfaces.h"

extern Road g_map[MAP_DIM][MAP_DIM];

float wx = 0.0f, wy = 0.0f;
float orientation = 0.0f;
bool g_bDrawBackward = true;

void init()
{
  // random number initialization
  srand(time(NULL));

  // initialize model logic and drawing modules
  ml_init(wx, wy);
  md_init();

  // OpenGL initialization
  glClearColor(0.0, 0.0, 0.0, 0.0);

  float l0Pos[4] = {-1.0f, 1.0f, 1.0f, 0.0f};
  float l1Pos[4] = {1.0f, -1.0f, 1.0f, 0.0f};
  float l2Pos[4] = {-1.0f, -1.0f, 1.0f, 0.0f};
  float l3Pos[4] = {1.0f, 1.0f, 1.0f, 0.0f};
  float lDiffuse[4] = {0.6f, 0.6f, 0.6f, 1.0f};
  float lSpecular[4] = {1.0f, 1.0f, 1.0f, 1.0f};

  glLightfv(GL_LIGHT0, GL_POSITION, l0Pos);
  glLightfv(GL_LIGHT1, GL_POSITION, l1Pos);
  glLightfv(GL_LIGHT2, GL_POSITION, l2Pos);
  glLightfv(GL_LIGHT3, GL_POSITION, l3Pos);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lDiffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, lSpecular);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, lDiffuse);
  glLightfv(GL_LIGHT1, GL_SPECULAR, lSpecular);
  glLightfv(GL_LIGHT2, GL_DIFFUSE, lDiffuse);
  glLightfv(GL_LIGHT2, GL_SPECULAR, lSpecular);
  glLightfv(GL_LIGHT3, GL_DIFFUSE, lDiffuse);
  glLightfv(GL_LIGHT3, GL_SPECULAR, lSpecular);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);
  glEnable(GL_LIGHT2);

  glEnable(GL_DEPTH_TEST);

  glEnable(GL_BLEND);
}

void keyboardCB(GLFWwindow* window, int key, int scancode, int action,
	       int mods)
{
  switch(key){
  case GLFW_KEY_W:
    if (action == GLFW_PRESS){
      wx += 0.1f * -sin(orientation/180.0f*M_PI);
      wy += 0.1f * cos(orientation/180.0f*M_PI);
      printf("(%f, %f)\n", wx, wy);
    }
    break;
  case GLFW_KEY_S:
    if (action == GLFW_PRESS){
      wx -= 0.1f * -sin(orientation/180.0f*M_PI);
      wy -= 0.1f * cos(orientation/180.0f*M_PI);
      printf("(%f, %f)\n", wx, wy);
    }
    break;
  case GLFW_KEY_D:
    if (action == GLFW_PRESS)
      orientation = (int)(orientation + 270.0f) % 360;
    break;
  case GLFW_KEY_A:
    if (action == GLFW_PRESS)
      orientation = (int)(orientation + 90.0f) % 360;
    break;
  case GLFW_KEY_B:
    if (action == GLFW_PRESS)
      g_bDrawBackward = !g_bDrawBackward;
    break;
  case GLFW_KEY_ESCAPE:
    if (action == GLFW_PRESS)
      glfwSetWindowShouldClose(window, GL_TRUE);
    break;
  default:
    // do nothing
    break;
  }
}

void auto_runner()
{
  wx += 0.1f * -sin(orientation/180.0f*M_PI);
  wy += 0.1f * cos(orientation/180.0f*M_PI);

  int mx = WORLD_TO_MAP(wx + 1.5f),
    my = WORLD_TO_MAP(wy);
  float ori = g_map[my][mx].orientation;
  float distToTurn;

  static float goalOrientation = 0.0f;
  static int sign = 1;

  if (g_map[my][mx].type == LEFT && goalOrientation == ori){
    goalOrientation = (int)(goalOrientation + 90.0f) % 360;
    sign = 1;
  }
  else if (g_map[my][mx].type == RIGHT && goalOrientation == ori){
    goalOrientation = (int)(goalOrientation + 270.0f) % 360;
    sign = -1;
  }
  else if (g_map[my][mx].type == LEFT_RIGHT && goalOrientation == ori){
    int r = rand() % 1000;

    if (r < 500){
      goalOrientation = (int)(goalOrientation + 90.0f) % 360;
      sign = 1;
    }
    else{
      goalOrientation = (int)(goalOrientation + 270.0f) % 360;
      sign = -1;
    }
  }

  static const double dt = 2.0*atan2(1.0, 29.0)/M_PI*180.0;
  float angleDiff = fabs(orientation - goalOrientation);

  angleDiff = (angleDiff > 180.0f)? (360.0f-angleDiff) : angleDiff;

  if (angleDiff > 1.0f){
    orientation += sign*dt;

    if (orientation < 0.0f)
      orientation += 360.0f;
    else if (orientation > 360.0f)
      orientation -= 360.0f;
  }
  else if (orientation != goalOrientation)
    orientation = goalOrientation;
}

int main(void)
{
  GLFWwindow* window;

  if (!glfwInit())
    exit(EXIT_FAILURE);

  window = glfwCreateWindow(640, 480, "Knockoff Temple Run", NULL, NULL);

  if (!window){
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);
  glfwSetKeyCallback(window, keyboardCB);
  init();

  int w, h;

  while (!glfwWindowShouldClose(window)){
    glfwGetFramebufferSize(window, &w, &h);

    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w/(double)h, 0.1, 50);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    auto_runner();

    ml_update(wx, wy);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();
    gluLookAt(0.0, 0.0, 1.0, -sin(orientation/180.0f*M_PI),
	      cos(orientation/180.0f*M_PI), 0.8, 0.0, 0.0, 1.0);
    md_draw(wx, wy, g_bDrawBackward);
    glPopMatrix();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  md_destroy();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
