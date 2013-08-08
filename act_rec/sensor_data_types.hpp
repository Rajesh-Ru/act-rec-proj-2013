/**
 * Author: Rajesh
 * Description: the definitions of some sensor data types.
 */

#include "XnCppWrapper.h"

#ifndef SENSOR_DATA_TYPES_HPP
#define SENSOR_DATA_TYPES_HPP

namespace rad
{
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
  SkeletonInfo* pSkels;
  xn::SceneMetaData* pSceneMD;
  xn::DepthMetaData* pDepthMD;
} SensorMsg;
}

#endif // SENSOR_DATA_TYPES_HPP
