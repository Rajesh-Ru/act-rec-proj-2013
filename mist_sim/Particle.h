/**
 * Author: Rajesh
 * Description: definition of a particle along with some basic types.
 */

#ifndef PARTICLE_H
#define PARTICLE_H

namespace rps
{
typedef struct Point3D
{
  float x, y, z;
} Point3D;

typedef struct ColorRGBA
{
  float r, g, b, a;
} ColorRGBA;

typedef struct Particle
{
  Point3D position;
  ColorRGBA color;
  float size;
} Particle;
}

#endif // PARTICLE_H
