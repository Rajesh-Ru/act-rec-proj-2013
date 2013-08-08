/**
 * Author: Rajesh
 * Description: a factory that creates sensor data publishers and subscribers.
 *   This class should not be used in multi-threaded program!
 */

#include "KinectDataPub.hpp"
#include "FireBallAction.hpp"
#include "SceneDrawer.hpp"

#ifndef RADFACTORY_HPP
#define RADFACTORY_HPP

namespace rad
{
class RADFactory
{
public:
  static RADFactory* getPtrToInst();
  static void releaseInst();

  ~RADFactory();
  SensorDataPub* getPtrToPub(RADenum type);
  SensorDataSub* getPtrToSub(RADenum type);
  void release(RADenum type, const void* pData);

private:
  static RADFactory* pInst;
};
}

#endif // RADFACTORY_HPP
