/**
 * Author: Rajesh
 * Description: abstract class for .obj models with animation.
 */

#ifndef ANIMATED_OBJECT_HPP
#define ANIMATED_OBJECT_HPP

namespace ao
{
typedef struct Point3D
{
  float x, y, z;
} Point3D;

typedef Point3D RPY;

typedef Point3D Scale3D;

typedef struct ColorRGBA
{
  float r, g, b, a;
} ColorRGBA;

class AnimatedObject
{
public:
  virtual ~AnimatedObject() {}
  // Make methods pure virtual by appending "= 0" to them
  // if they should be implemented (abstract).
  virtual void update() = 0;
  virtual void draw() = 0;
};
}

#endif // ANIMATED_OBJECT_HPP
