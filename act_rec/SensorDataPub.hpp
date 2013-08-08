/**
 * Author: Rajesh
 * Description: the interface that nodes publish sensory data should implement.
 */

#include "SensorDataSub.hpp"

#ifndef SENSORDATAPUB_HPP
#define SENSORDATAPUB_HPP

namespace rad
{
class SensorDataPub
{
public:
  virtual ~SensorDataPub() {}
  virtual void subscribe(SensorDataSub *pSub) = 0;
  virtual void spin(bool async = false) = 0;
  virtual void spinOnce() = 0;
};
}

#endif // SENSORDATAPUB_HPP
