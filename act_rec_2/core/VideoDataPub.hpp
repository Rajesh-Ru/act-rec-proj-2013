/**
 * Author: Rajesh
 * Description: reads data from a .oni file and publishes it as if it's from
 *   a sensor.
 */

#include "SensorDataPub.hpp"

#ifndef VIDEODATAPUB_HPP
#define VIDEODATAPUB_HPP

namespace rad
{
class VideoDataPub : public SensorDataPub
{
public:
  // fn is the file name of the video record
  VideoDataPub(const char* fn, bool setupOpenNI = true);
  ~VideoDataPub();
  void subscribe(SensorDataSub* pSub);
  void spin(bool async = false);
  void spinOnce();
  xn::ProductionNode* getPrdNode(XnPredefinedProductionNodeType type);
  bool isEndOfVideo();

  static VideoDataPub* getPtrToInst(const char* fn, bool setupOpenNI = true);
  static void releaseInst();

private:
  XnBool fileExists(const char* fn);
  void initOpenNI(const char* fn);
  void getSkeleton(SkeletonInfo &skel);

  static VideoDataPub* pInst;

  bool isEOF;
  std::vector<SensorDataSub*> subscribers;
};
}

#endif // VIDEODATAPUB_HPP
