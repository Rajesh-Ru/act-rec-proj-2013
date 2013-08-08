/**
 * Author: Rajesh
 * Description: an implementation of Bolt.hpp.
 */

#include <stdlib.h>
#include <time.h>
#include <GLFW/glfw3.h>

#include "Bolt.hpp"

using namespace ao;

Bolt::Bolt(float l, float h, int numOfControlPoints, int numOfBoltElements,
	   const std::vector<ColorRGBA> &colors)
{
  length = l;
  height = h;

  elements.resize(numOfBoltElements);

  float stepWidth = l / (float)(numOfControlPoints - 1);

  for (int i = 0; i < numOfBoltElements; ++i){
    elements[i].color = colors[i];
    elements[i].controlPoints.resize(numOfControlPoints);

    std::vector<Point3D> &cp = elements[i].controlPoints;

    for (int j = 0; j < numOfControlPoints; ++j){
      cp[j].x = -l / 2.0f + stepWidth * (float)j;
      cp[j].y = cp[j].z = 0.0f;
    }
  }

  srand(time(NULL));
}

void Bolt::update()
{
  float stepWidth = length /
    (float)(elements[0].controlPoints.size() - 1);

  for (int i = 0; i < elements.size(); ++i){
    std::vector<Point3D> &cp = elements[i].controlPoints;

    for (int j = 1; j < cp.size() - 1; ++j){
      cp[j].x = -length / 2.0f +
	(rand() % 1000 / 1000.0f + (float)j - 0.5f) * stepWidth;
      cp[j].y = (rand() % 1000 / 1000.0f - 0.5f) * height;
      cp[j].z = (rand() % 1000 / 1000.0f - 0.5f) * height;
    }
  }
}

void Bolt::draw()
{
  glPushAttrib(GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT | GL_LIGHTING_BIT);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_COLOR_MATERIAL);

  for (int i = 0; i < elements.size(); ++i){
    glColor4f(elements[i].color.r, elements[i].color.g, elements[i].color.b,
	      elements[i].color.a);

    std::vector<Point3D> &cp = elements[i].controlPoints;

    glBegin(GL_LINE_STRIP);
    for (int j = 0; j < cp.size(); ++j)
      glVertex3f(cp[j].x, cp[j].y, cp[j].z);
    glEnd();
  }

  glPopAttrib();
}
