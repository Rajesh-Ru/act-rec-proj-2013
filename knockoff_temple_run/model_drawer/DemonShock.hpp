/**
 * Author: Rajesh
 * Description: game contraption - a pair of demon arms casting shocking bolt.
 */

#include "../glm/glm.h"
#include "Bolt.hpp"

#ifndef DEMON_SHOCK_HPP
#define DEMON_SHOCK_HPP

namespace ao
{
class DemonShock final : public AnimatedObject
{
public:
  DemonShock(const char* filename);
  ~DemonShock();
  void update();
  void draw();

private:
  GLMmodel* pDemonArm;
  Bolt* pBolt;
};
}

#endif // DEMON_SHOCK_HPP
