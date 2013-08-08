/**
 * Author: Rajesh
 * Descriptor: simulating a fireball.
 */

#include <vector>

#include "AnimatedObject.hpp"

#ifndef FIREBALL_HPP
#define FIREBALL_HPP

namespace ao
{
typedef struct ParticleR
{
  Point3D position;
  int lifespan;
  float yaw, pitch; // in radian
} ParticleR;

class Fireball final : public AnimatedObject
{
public:
  Fireball(float r);
  void update();
  void draw();

private:
  void addNewParticles();

  const float yStepWidth = 10.0f;
  const float pStepWidth = 10.0f;
  const float rDelta = 0.05;
  int maxLifespan;
  float radius;
  std::vector<ParticleR> livePars;
  std::vector<int> deadParIdx;
};
}

#endif // FIREBALL_HPP
