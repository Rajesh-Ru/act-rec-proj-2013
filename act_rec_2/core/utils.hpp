/**
 * Author: Rajesh
 * Description: some common utilities for action recognition.
 */

#include <math.h>

#ifndef UTILS_HPP
#define UTILS_HPP

#define MAX_NUM_USERS 1 // allow only one user

// @ref #define DESCRIPTORSIZE 64 in skel_editor/main.cpp
#define DESCRIPTOR_SIZE 64

#define vecDist3f(x1, y1, z1, x2, y2, z2) \
  sqrt(pow((x1)-(x2), 2.0f) + pow((y1)-(y2), 2.0f) + pow((z1)-(z2), 2.0f))

namespace rad
{
typedef enum RADenum
{
  RAD_SKEL_INFO = 1,
  RAD_SKEL_INFO_PTR = 2,
  RAD_JOINT_INFO = 3,
  RAD_JOINT_INFO_PTR = 4,
  RAD_SENSOR_MSG = 5,
  RAD_PUB = (1 << 10), // publisher: [2^10, 2^11)
  RAD_PUB_TYPE_KINECT_DATA = RAD_PUB + 1,
  RAD_PUB_TYPE_VIDEO_DATA = RAD_PUB + 2,
  RAD_SUB = (1 << 11), // subscriber: [2^11, 2^12-2^10)
  RAD_SUB_TYPE_FIREBALL_ACT = RAD_SUB + 1,
  RAD_SUB_TYPE_SCENE_DRAWER = RAD_SUB + 2,
  RAD_BC = (1 << 12),
  RAD_BC_TYPE_FIREBALL_START = RAD_BC + 1
}RADenum;
}

#endif // UTILS_HPP
