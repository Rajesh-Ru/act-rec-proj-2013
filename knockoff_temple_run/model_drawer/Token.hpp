/**
 * Author: Rajesh
 * Description: an animated token object.
 */

#include "../glm/glm.h"
#include "AnimatedObject.hpp"

#ifndef TOKEN_HPP
#define TOKEN_HPP

namespace ao
{
class Token final : public AnimatedObject
{
public:
  Token(const char* filename);
  ~Token();
  void update();
  void draw();

private:
  Point3D position;
  RPY rpy;
  Scale3D scale;
  GLMmodel* pModel;
};
}

#endif // TOKEN_HPP
