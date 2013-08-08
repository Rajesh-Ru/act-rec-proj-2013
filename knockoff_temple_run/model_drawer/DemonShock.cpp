/**
 * Author: Rajesh
 * Description: an implementation of DemonShock.hpp.
 */

#include "DemonShock.hpp"

using namespace ao;

DemonShock::DemonShock(const char* filename)
{
  pDemonArm = glmReadOBJ(filename);

  ColorRGBA color;
  std::vector<ColorRGBA> colors;

  color.r = color.g = color.b = 1.0f;
  color.a = 0.8;
  colors.push_back(color);

  color.g = 0.0f;
  color.r = color.b = 0.8f;
  color.a = 0.8;
  colors.push_back(color);

  color.r = color.g = color.b = 0.2f;
  color.a = 0.8f;
  colors.push_back(color);

  color.g = 0.0f;
  color.r = color.b = 1.0f;
  color.a = 0.8f;
  colors.push_back(color);

  color.r = 0.8f;
  color.g = 0.2f;
  color.b = 0.4f;
  color.a = 0.8f;
  colors.push_back(color);

  pBolt = new Bolt(2.0f, 0.1f, 20, 5, colors);
}

DemonShock::~DemonShock()
{
  glmDelete(pDemonArm);
  delete pBolt;
}

void DemonShock::update()
{
  pBolt->update();
}

void DemonShock::draw()
{
  glPushAttrib(GL_COLOR_BUFFER_BIT | GL_LIGHTING_BIT);
  glDisable(GL_ALPHA_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_LIGHTING);

  glPushMatrix();
  glTranslatef(-1.0f, 0.0f, 0.0f);
  glScalef(0.5f, 0.5f, 0.5f);
  glmDraw(pDemonArm, GLM_SMOOTH | GLM_TEXTURE);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(1.0f, 0.0f, 0.0f);
  glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
  glScalef(0.5f, 0.5f, 0.5f);
  glmDraw(pDemonArm, GLM_SMOOTH | GLM_TEXTURE);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(0.0f, 0.0f, 1.5f);
  pBolt->draw();
  glPopMatrix();

  glPopAttrib();
}
