/**
 * Author: Rajesh
 * Description: skeleton publishers obtain data from Kinect sensors. This class
 *   should not be used in multi-threaded program!
 */

#include "SensorDataPub.hpp"

#ifndef KINECTDATAPUB_HPP
#define KINECTDATAPUB_HPP

namespace rad
{
class KinectDataPub : public SensorDataPub
{
public:
  // configure from an XML file
  KinectDataPub(const char* fn, bool setupOpenNI = true);
  ~KinectDataPub();
  void subscribe(SensorDataSub* pSub);
  void spin(bool async = false);
  void spinOnce();
  xn::ProductionNode* getPrdNode(XnPredefinedProductionNodeType type);

  static KinectDataPub* getPtrToInst(const char* fn, bool setupOpenNI = true);
  static void releaseInst();

private:
  XnBool fileExists(const char* fn);
  void initOpenNI(const char* fn);
  void getSkeleton(SkeletonInfo &skel);

  static KinectDataPub* pInst;

  std::vector<SensorDataSub*> subscribers;
};
}

#endif // KINECTDATAPUB_HPP
