/**
 * Author: Rajesh
 * Description: code that draws trees.
 */

#include "../glm/glm.h"
#include "AnimatedObject.hpp"

#ifndef TREE_HPP
#define TREE_HPP

namespace ao
{
class Tree final : public AnimatedObject
{
public:
  Tree(const char* filename);
  ~Tree();
  void update() {}
  void draw();

private:
  GLMmodel* pModel;
};
}

#endif // TREE_HPP
