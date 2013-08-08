/**
 * Author: Rajesh
 * Description: code that draws a fire sperpent.
 */

#include "../glm/glm.h"
#include "Fire.hpp"

#ifndef FIRE_HPP
#define FIRE_HPP

namespace ao
{
class FireSerpent final : public AnimatedObject
{
public:
  FireSerpent(const char* filename);
  ~FireSerpent();
  void update();
  void draw();

private:
  GLMmodel* pSerpent;
  Fire* pFire;
};
}

#endif // FIRE_HPP
