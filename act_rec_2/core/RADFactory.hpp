/**
 * Author: Rajesh
 * Description: a factory that creates sensor data publishers and subscribers.
 *   This class should not be used in multi-threaded program!
 */

#include "VideoDataPub.hpp"
#include "KinectDataPub.hpp"
#include "FireBallAction.hpp"
#include "SceneDrawer.hpp"
#include "../binary_classifier/BiClassifier.hpp"

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
  BiClassifier* getPtrToBiClassifier(RADenum type);
  void release(RADenum type, const void* pData);

private:
  static RADFactory* pInst;
};
}

#endif // RADFACTORY_HPP
