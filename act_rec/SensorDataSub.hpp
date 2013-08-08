/**
 * Author: Rajesh
 * Description: The interface that the other classes who want to be notified
 *   about updated sensory information need to implement.
 */

#include <XnCppWrapper.h>
#include <vector>

#include "utils.hpp"
#include "sensor_data_types.hpp"

#ifndef SENSORDATASUB_HPP
#define SENSORDATASUB_HPP

namespace rad // Rajesh Action Detector
{
class SensorDataSub
{
public:
  virtual ~SensorDataSub() {}
  // called by a SkeletonPublisher
  virtual void notify(RADenum msgType, const void* msg) = 0;
};
}

#endif // SENSORDATASUB_HPP
