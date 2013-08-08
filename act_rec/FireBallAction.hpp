/**
 * Author: Rajesh
 * Description: a state machine that recognizes the fire-ball action.
 */

#include <list>
#include <map>

#include "Action.hpp"
#include "SensorDataPub.hpp"

#ifndef FIREBALLACTION_HPP
#define FIREBALLACTION_HPP

namespace rad
{
class FireBallAction final : public Action
{
public:
  FireBallAction();
  ~FireBallAction() {}
  void notify(RADenum msgType, const void* msg);
  ActMotCBID registerMotionCB(ActionCallbackHandle hMotionCB);
  ActComCBID registerCompleteCB(ActionCallbackHandle hCompleteCB);
  void unregisterMotionCB(ActMotCBID id);
  void unregisterCompleteCB(ActComCBID id);

private:
  void updateState(const SkeletonInfo* pSkel,
		   const xn::DepthMetaData* pDepthMD,
		   const xn::SceneMetaData* pSceneMD);
  double localMeanDepth(int x, int y, int dim,
			const xn::DepthMetaData* pDepthMD);
  double userMeanDepth(XnUserID user, const xn::DepthMetaData* pDepthMD,
		     const xn::SceneMetaData* pSceneMD);
  double usernMeanDepth(XnUserID user, const xn::DepthMetaData* pDepthMD,
		      const xn::SceneMetaData* pSceneMD, int n);

  const int numOfFramesToAbort = 45;
  std::map<XnUserID, int> abortTimers;
  std::map<XnUserID, ActionState> curStates;
  std::list<ActMotCBID> motIDRep;
  std::map<ActMotCBID, ActionCallbackHandle> motionCBs;
  std::list<ActComCBID> comIDRep;
  std::map<ActComCBID, ActionCallbackHandle> completeCBs;
  std::map<XnUserID, XnVector3D> startLeftHandPos, startRightHandPos;
  std::map<XnUserID, double> startDepthVals;
};
}

#endif // FIREBALLACTION_HPP
