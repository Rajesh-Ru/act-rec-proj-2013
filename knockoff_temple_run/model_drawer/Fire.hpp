/**
 * Author: Rajesh
 * Description: code that simulate fire.
 */

#include <list>

#include "AnimatedObject.hpp"

namespace ao
{
typedef struct Particle
{
  Point3D position;
  int lifespan;
} Particle;

class Fire final : public AnimatedObject
{
public:
  Fire(float l, float r);
  void update();
  void draw();

private:
  void addNewParticles();

  const float lStepWidth = 0.05f;
  const float rStepWidth = 0.01f;
  const float aStepWidth = 5.0f; // a = azimuth
  int maxLifespan;
  float length;
  float radius;
  std::list<Particle> particles;
};
}
