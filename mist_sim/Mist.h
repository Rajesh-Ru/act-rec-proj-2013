/**
 * Author: Rajesh
 * Description: a class that simulate mist in a 3D scene.
 */

#include <vector>

#include "Particle.h"

#ifndef MIST_H
#define MIST_H

namespace rps
{
typedef Point3D Volume3D;

class Mist
{
public:
  Mist(float x_span = 1.0f, float y_span = 1.0f, float z_span = 1.0f,
       float x = 0.0f, float y = 0.0f, float z = 0.0f,
       float density = 1.0f);

  Mist(Volume3D size, Point3D position, float density = 1.0f);

  void draw();

private:
  void initializeParticle(Particle &p, Volume3D vol);

  const float stepWidth = 0.05f; // step width used for sampling particles

  Volume3D size;
  Point3D position;
  float density;
  std::vector<Particle> particles;
};
}

#endif // MIST_H
