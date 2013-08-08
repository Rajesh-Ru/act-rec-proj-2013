/**
 * Author: Rajesh
 * Description: an implementation of Tree.hpp.
 */

#include "Tree.hpp"

using namespace ao;

Tree::Tree(const char* filename)
{
  pModel = glmReadOBJ(filename);
}

Tree::~Tree()
{
  glmDelete(pModel);
}

void Tree::draw()
{
  glPushAttrib(GL_COLOR_BUFFER_BIT);
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER, 0.5);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glmDraw(pModel, GLM_SMOOTH | GLM_TEXTURE);

  glPopAttrib();
}
