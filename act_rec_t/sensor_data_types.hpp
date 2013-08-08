/**
 * Author: Rajesh
 * Description: the definitions of some sensor data types.
 */

#include <vector>

#include "XnCppWrapper.h"

#ifndef SENSOR_DATA_TYPES_HPP
#define SENSOR_DATA_TYPES_HPP

typedef struct JointInfo
{
  XnSkeletonJoint type;
  XnSkeletonJointTransformation pose; // position + orientation                                                                                        
} JointInfo;

typedef struct SkeletonInfo
{
  XnUserID user;
  std::vector<JointInfo> joints;
} SkeletonInfo;

typedef struct SensorMsg
{
  XnUInt16 nUsers;
  std::vector<SkeletonInfo> skels;
  xn::SceneMetaData sceneMD;
  xn::DepthMetaData depthMD;
} SensorMsg;

#endif // SENSOR_DATA_TYPES_HPP
