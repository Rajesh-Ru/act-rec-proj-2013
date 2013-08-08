/**
 * Author: Rajesh
 * Description: general definitions and stuff.
 */

#ifndef COMMON_H
#define COMMON_H

#define MAP_DIM 51

int mp(int x);

#define WORLD_TO_MAP(x) mp((int)floor((float)(x) / 3.0f)) % MAP_DIM

typedef enum ObjectType
{
  NOTHING = 0,
  TOKEN = 1,
  DEMON_SHOCK = 2,
  TREE = 3,
  FIRE_SERPENT = 4,
  PITFALL = 5
} ObjectType;

typedef struct Object
{
  ObjectType type;
  float x, y;
} Object;

typedef enum RoadType
{
  EMPTY = 0,
  STRAIGHT = 1,
  LEFT = 2,
  RIGHT = 4,
  START = 8,
  LEFT_RIGHT = LEFT | RIGHT
} RoadType;

typedef struct Road
{
  RoadType type;
  float orientation;
  Object object;
} Road;

typedef struct Index2D
{
  int x, y;
} Index2D;

#endif // COMMON_H
