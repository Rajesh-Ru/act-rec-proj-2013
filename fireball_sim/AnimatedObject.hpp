/**
 * Author: Rajesh
 * Description: abstract class for .obj models with animation.
 */

#include <stdio.h>

#ifndef ANIMATED_OBJECT_HPP
#define ANIMATED_OBJECT_HPP

namespace ao
{
typedef struct Point3D
{
  float x, y, z;
} Point3D;

typedef Point3D Vector3D;

typedef Point3D RPY;

typedef Point3D Scale3D;

typedef struct ColorRGBA
{
  float r, g, b, a;
} ColorRGBA;

typedef enum WeaponType
{
  FIREBALL
} WeaponType;

class AnimatedObject
{
public:
  virtual ~AnimatedObject() {}
  // Make methods pure virtual by appending "= 0" to them
  // if they should be implemented (abstract).
  virtual void update() = 0;
  virtual void draw() = 0;
};

typedef struct Weapon
{
  AnimatedObject* pObject;
  Point3D position;
  Vector3D direction;
  float speed, dist;
} Weapon;
}

#endif // ANIMATED_OBJECT_HPP
