/**
 * Author: Rajesh
 * Description: draw bolt with specified length and height field.
 */

#include <vector>

#include "AnimatedObject.hpp"

#ifndef BOLT_HPP
#define BOLT_HPP

namespace ao
{
typedef struct BoltElement
{
  std::vector<Point3D> controlPoints;
  ColorRGBA color;
} BoltElement;

class Bolt final : public AnimatedObject
{
public:
  Bolt(float l, float h, int numOfControlPoints, int numOfBoltElements,
       const std::vector<ColorRGBA> &colors);
  void update();
  void draw();

private:
  std::vector<BoltElement> elements;
  float length;
  float height;
};
}

#endif // BOLT_HPP
