/**
 * Author: Rajesh
 * Description: an implementation of FireSerpent.hpp.
 */

#include <GLFW/glfw3.h>

#include "FireSerpent.hpp"

using namespace ao;

FireSerpent::FireSerpent(const char* filename)
{
  pSerpent = glmReadOBJ(filename);
  pFire = new Fire(2.5f, 0.15f);
}

FireSerpent::~FireSerpent()
{
  glmDelete(pSerpent);
  delete pFire;
}

void FireSerpent::update()
{
  pFire->update();
}

void FireSerpent::draw()
{
  glPushMatrix();
  glScalef(0.5f, 0.5f, 0.5f);
  glmDraw(pSerpent, GLM_SMOOTH | GLM_TEXTURE);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(0.1f, 0.0f, 0.85f);
  pFire->draw();
  glPopMatrix();
}
