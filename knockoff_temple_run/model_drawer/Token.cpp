/**
 * Author: Rajesh
 * Description: implementation of Token.hpp.
 */

#include <math.h>

#include "Token.hpp"

using namespace ao;

Token::Token(const char* filename)
{
  pModel = glmReadOBJ(filename);
  position.x = position.y = position.z = 0.0f;
  rpy.x = rpy.y = rpy.z = 0.0f;
  scale.x = scale.y = scale.z = 0.5f;
}

Token::~Token()
{
  glmDelete(pModel);
}

void Token::update()
{
  static bool upGoing = true;

  if (upGoing){
    position.z += 0.01;
    if (position.z > 0.5)
      upGoing = false;
  }
  else{
    position.z -= 0.01;
    if (position.z < -0.5)
      upGoing = true;
  }

  rpy.z += 1.0f;
  if (rpy.z > 360.0f)
    rpy.z -= 360.0f;
}

void Token::draw()
{
  glPushMatrix();
  glTranslatef(position.x, position.y, position.z);
  glRotatef(rpy.z, 0.0f, 0.0f, 1.0f);
  glScalef(scale.x, scale.y, scale.z);
  glmDraw(pModel, GLM_SMOOTH | GLM_TEXTURE);
  glPopMatrix();
}
