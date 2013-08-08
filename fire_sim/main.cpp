/**
 * Author: Rajesh
 * Description: simple test code for Fire.*.
 */

#include <GL/freeglut.h>
#include <stdio.h>

#include "Fire.hpp"

static ao::Fire* pFire;

void init()
{
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glShadeModel(GL_SMOOTH);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  glEnable(GL_DEPTH_TEST);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  pFire = new ao::Fire(3.0f, 0.2f);
}

void timerCB(int val)
{
  pFire->update();
  glutPostRedisplay();
  glutTimerFunc(33, timerCB, 0);
}

void reshape(int w, int h)
{
  glViewport(0, 0, (GLsizei)w, (GLsizei)h);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0, (double)w/(double)h, 0.1, 10);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void display()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_COLOR_MATERIAL);

  glPushMatrix();
  gluLookAt(0.0, -2.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);

  glTranslatef(0.0f, 3.0f, -1.0f);
  glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
  pFire->draw();

  glPopMatrix();

  glutSwapBuffers();
}

int main(int argc, char** argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(500, 500);
  glutCreateWindow("Fire sim");
  init();
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutTimerFunc(33, timerCB, 0);
  glutMainLoop();

  delete pFire;

  return 0;
}
