/**
 * Author: Rajesh
 * Description: some common utilities for action recognition.
 */

#include <math.h>
#include <map>
#include <list>
#include <pthread.h>
#include <GLFW/glfw3.h>

#include "sensor_data_types.hpp"

#ifndef COMMON_HPP
#define COMMON_HPP

#define CONFIG_FILE_NAME "SamplesConfig.xml"

#define MAX_NUM_USERS 5
#define MAX_NUM_SUBS 10
#define MAX_NUM_ACT_CBS 10

#define vecDist3f(x1, y1, z1, x2, y2, z2) \
  sqrt(pow((x1)-(x2), 2.0f) + pow((y1)-(y2), 2.0f) + pow((z1)-(z2), 2.0f))

typedef enum RADenum
{
  RAD_SKEL_INFO = 1,
  RAD_SKEL_INFO_PTR = 2,
  RAD_JOINT_INFO = 3,
  RAD_JOINT_INFO_PTR = 4,
  RAD_SENSOR_MSG = 5,
  RAD_PUB = (1 << 10), // publisher: [2^10, 2^11)
  RAD_PUB_TYPE_KINECT_DATA = RAD_PUB + 1,
  RAD_SUB = (1 << 11), // subscriber: [2^11, 2^12-2^10)
  RAD_SUB_TYPE_FIREBALL_ACT = RAD_SUB + 1,
  RAD_SUB_TYPE_SCENE_DRAWER = RAD_SUB + 2
}RADenum;

typedef enum ActionState
{
  UNRECOGNIZED = 0,
  START = 1,
  IN_PROGRESS = 2,
  RECOGNIZED = 3
} ActionState;
typedef void* (*ActionCB)(void*);
typedef int ActComCBID;
typedef int ActMotCBID;

typedef void* (*SubscriberCB)(void*);

#endif // COMMON_HPP
