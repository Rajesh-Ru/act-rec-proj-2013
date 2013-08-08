/**
 * Author: Rajesh
 * Description: test the simulation of mist.
 */

#include <GL/freeglut.h>
#define __USE_BSD
#include <math.h>

#include "Mist.h"

static rps::Mist* g_pMist = NULL;
static bool g_bLeftButtonDown = false;
static int g_iOldX, g_iOldY;
static int g_idx = 0, g_idy = 0;

void init()
{
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glShadeModel(GL_FLAT);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  g_pMist = new rps::Mist(1.0f, 1.0f);
}

void reshape(int w, int h)
{
  glViewport(0, 0, (GLsizei)w, (GLsizei)h);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  gluPerspective(45.0, (double)w/(double)h, 0.1, 10.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void display()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  int viewport[4];

  glGetIntegerv(GL_VIEWPORT, viewport);

  int winWidth = viewport[2];
  int winHeight = viewport[3];
  
  static float rx = 0.0f;
  static float ry = 0.0f;
  static float rz = 0.0f;

  ry += atan2((float)g_idx, (float)winWidth) / M_PI * 180.0f;
  rx += atan2((float)g_idy, (float)winHeight) / M_PI * 180.0f;

  glPushMatrix();
  gluLookAt(0.0, -2.0, 2.0, 0.0, 0.0, 0.5, 0.0, 0.0, 1.0);
  glRotatef(ry, 0.0f, 1.0f, 0.0f);
  glRotatef(rx, 1.0f, 0.0f, 0.0f);
    g_pMist->draw();
  glPopMatrix();

  glutSwapBuffers();
}

void mouse(int button, int state, int x, int y)
{
  switch (button){
  case GLUT_LEFT_BUTTON:
    switch (state){
    case GLUT_DOWN:
      g_iOldX = x;
      g_iOldY = y;
      g_bLeftButtonDown = true;
      break;
    case GLUT_UP:
      g_bLeftButtonDown = false;
      break;
    }
    break;
  }
}

void mouseMotion(int x, int y)
{
  if (g_bLeftButtonDown){
    g_idx = x - g_iOldX;
    g_idy = y - g_iOldY;
  }

  glutPostRedisplay();
}

int main(int argc, char** argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(500, 500);
  glutCreateWindow("mist_sim");
  init();
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutMouseFunc(mouse);
  glutMotionFunc(mouseMotion);
  glutMainLoop();

  delete g_pMist;

  return 0;
}
