/**
 * Author: Rajesh
 * Description: an implementation of FireBallAction.hpp.
 */

#include <algorithm>

#include "FireBallAction.hpp"
#include "RADFactory.hpp"

using namespace rad;

FireBallAction::FireBallAction()
{
}

void FireBallAction::notify(RADenum msgType, const void* msg)
{
  if (msgType != RAD_SENSOR_MSG)
    return;

  if (!motionCBs.empty() || !completeCBs.empty()){
    XnUInt16 nUsers = ((const SensorMsg*)msg)->nUsers;

    const SensorMsg* pMsg = (const SensorMsg*)msg;
    const SkeletonInfo* pSkels = pMsg->pSkels;
    const xn::DepthMetaData* pDepthMD = pMsg->pDepthMD;
    const xn::SceneMetaData* pSceneMD = pMsg->pSceneMD;

    for (int i = 0; i < nUsers; ++i)
      if (pSkels[i].user != 0)
	updateState(&pSkels[i], pDepthMD, pSceneMD);
  }
}

void FireBallAction::updateState(const SkeletonInfo* pSkel,
				 const xn::DepthMetaData* pDepthMD,
				 const xn::SceneMetaData* pSceneMD)
{
  XnUserID id = pSkel->user;

  if (id == 0)
    return;

  static std::map<XnUserID, bool> onceThroughs;

  if (curStates.find(id) == curStates.end()){
    curStates[id] = UNRECOGNIZED;
    onceThroughs[id] = false;
  }

  const std::vector<JointInfo> &joints = pSkel->joints;
  const JointInfo* pJoints[4];

  for (int i = 0; i < joints.size(); ++i){
    switch(joints[i].type){
    case XN_SKEL_LEFT_HAND:
      pJoints[0] = &joints[i];
      break;
    case XN_SKEL_RIGHT_HAND:
      pJoints[1] = &joints[i];
      break;
    case XN_SKEL_TORSO:
      pJoints[2] = &joints[i];
      break;
    case XN_SKEL_NECK:
      pJoints[3] = &joints[i];
      break;
    default:
      // do nothing
      break;
    }
  }

  const XnSkeletonJointTransformation *pLeftHand = &pJoints[0]->pose,
    *pRightHand = &pJoints[1]->pose, *pTorso = &pJoints[2]->pose,
    *pNeck = &pJoints[3]->pose;
  const XnVector3D &lhPos = pLeftHand->position.position,
    &rhPos = pRightHand->position.position,
    &tPos = pTorso->position.position,
    &nPos = pNeck->position.position;

  static std::map<XnUserID, int> votes; // use vote to add some robustness

  if (curStates[id] == UNRECOGNIZED){
    if (pLeftHand->position.fConfidence < 0.5 ||
	pRightHand->position.fConfidence < 0.5 ||
	pTorso->position.fConfidence < 0.5 ||
	pNeck->position.fConfidence < 0.5)
      return; // not confident enough about the data obtained

    float handDist = vecDist3f(lhPos.X, lhPos.Y, lhPos.Z,
				   rhPos.X, rhPos.Y, rhPos.Z);
    float halfTorsoLen = vecDist3f(tPos.X, tPos.Y, tPos.Z,
				      nPos.X, nPos.Y, nPos.Z);
    float htDist = vecDist3f((lhPos.X+rhPos.X)/2.0f, (lhPos.Y+rhPos.Y)/2.0f,
			     (lhPos.Z+rhPos.Z)/2.0f, tPos.X, tPos.Y, tPos.Z);

    if (((lhPos.X < tPos.X && rhPos.X < tPos.X) ||
	 (lhPos.X > tPos.X && rhPos.X > tPos.X)) &&
	htDist < 1.4f * halfTorsoLen && handDist < 1.4f * halfTorsoLen){
      if (onceThroughs[id] && (lhPos.Z < startLeftHandPos[id].Z ||
			  rhPos.Z < startRightHandPos[id].Z)){
	// regard as action start since both hands begin to move forward
	curStates[id] = IN_PROGRESS;
	abortTimers[id] = 0;
	votes[id] = 0;

	/*	XnPoint3D pts[2];
	pts[0] = lhPos;
	pts[1] = rhPos;
	((xn::DepthGenerator*)((KinectDataPub*)RADFactory::getPtrToInst()->
			       getPtrToPub(RAD_PUB_TYPE_KINECT_DATA))->
	 getPrdNode(XN_NODE_TYPE_DEPTH))->
	  ConvertRealWorldToProjective(2, pts, pts);
	startDepthVals[id] = localMeanDepth(pts[0].X, pts[0].Y, 10, pDepthMD) +
	  localMeanDepth(pts[1].X, pts[1].Y, 10, pDepthMD);
	*/
	//	startDepthVals[id] = userMeanDepth(id, pDepthMD, pSceneMD);
	onceThroughs[id] = false;
      }
      else{
	if (!onceThroughs[id])
	  onceThroughs[id] = true;

	startLeftHandPos[id] = lhPos;
	startRightHandPos[id] = rhPos;
      }
    }
  }
  else if (curStates[id] == IN_PROGRESS){
    if (pLeftHand->position.fConfidence < 0.5 ||
	pRightHand->position.fConfidence < 0.5)
      return;

    float lhOffset = startLeftHandPos[id].Z - lhPos.Z;
    float rhOffset = startRightHandPos[id].Z - rhPos.Z;
    /*
    XnPoint3D pts[2];
    pts[0] = lhPos;
    pts[1] = rhPos;
    ((xn::DepthGenerator*)((KinectDataPub*)RADFactory::getPtrToInst()->
			   getPtrToPub(RAD_PUB_TYPE_KINECT_DATA))->
     getPrdNode(XN_NODE_TYPE_DEPTH))->
      ConvertRealWorldToProjective(2, pts, pts);
    */

    if (lhOffset > 300.0f || rhOffset > 300.0f){
      //    printf("%f\n", startDepthVals[id] - userMeanDepth(id, pDepthMD, pSceneMD));
      // it seems that the goal is reached
      ++(votes[id]);
    }
    else{
      // haven't reached the goal
      votes[id] = 0;
      ++(abortTimers[id]);
    }

    if (votes[id] >= 2){
      // goal reached, invoke each complete callback
      for (std::map<ActComCBID, ActionCallbackHandle>::iterator it =
	     completeCBs.begin(); it != completeCBs.end(); ++it)
	it->second(RAD_SKEL_INFO, 1, pSkel);
      curStates[id] = UNRECOGNIZED;
    }
    else if (abortTimers[id] >= numOfFramesToAbort){
      // abort the action since it lasts too long without reaching the goal
      curStates[id] = UNRECOGNIZED;
    }
    else{
      // action in progress
      for (std::map<ActMotCBID, ActionCallbackHandle>::iterator it =
	     motionCBs.begin(); it != motionCBs.end(); ++it)
	it->second(RAD_SKEL_INFO, 1, pSkel);
    }
  }
}

double FireBallAction::localMeanDepth(int x, int y, int dim,
				      const xn::DepthMetaData* pDepthMD)
{
  int numOfPixels = 4 * dim * dim;
  int xRes = pDepthMD->XRes(), yRes = pDepthMD->YRes();
  int xStart = (x - dim >= 0)? x-dim : 0,
    xEnd = (x + dim <= xRes)? x+dim : xRes,
    yStart = (y - dim >= 0)? y-dim : 0,
    yEnd = (y + dim <= yRes)? y+dim : yRes;
  const XnDepthPixel* pDepth = pDepthMD->Data();
  double total = 0.0;

  for (int j = yStart; j < yEnd; ++j)
    for (int i = xStart; i < xEnd; ++i)
      total += *(pDepth + (j*xRes + i));

  return total / (double)numOfPixels;
}

double FireBallAction::usernMeanDepth(XnUserID user,
				      const xn::DepthMetaData* pDepthMD,
				      const xn::SceneMetaData* pSceneMD,
				      int n)
{
  int numOfPixels = pDepthMD->XRes() * pDepthMD->YRes();
  const XnDepthPixel* pDepth = pDepthMD->Data();
  const XnLabel* pLabels = pSceneMD->Data();

  std::vector<XnDepthPixel> vals;

  for (int i = 0; i < numOfPixels; ++i, ++pLabels, ++pDepth){
    if (*pLabels == user)
      vals.push_back(*pDepth);
  }

  if (n > vals.size())
    n = vals.size();

  std::sort(vals.begin(), vals.end());

  double total = 0.0;

  for (int i = 0; i < n; ++i){
    total += vals[i];
  }

  return total / (double)n;
}

double FireBallAction::userMeanDepth(XnUserID user,
				     const xn::DepthMetaData* pDepthMD,
				     const xn::SceneMetaData* pSceneMD)
{
  int numOfPixels = pDepthMD->XRes() * pDepthMD->YRes();
  const XnDepthPixel* pDepthMap = pDepthMD->Data();
  const XnLabel* pLabels = pSceneMD->Data();

  int numOfPoints = 0;
  double total = 0;

  for (int i = 0; i < numOfPixels; ++i, ++pLabels, ++pDepthMap){
    if (*pLabels == user){
      total += (double)*pDepthMap;
      ++numOfPoints;
    }
  }

  if (numOfPoints == 0)
    return -1.0;

  return total / (double)numOfPoints;
}

ActMotCBID FireBallAction::registerMotionCB(ActionCallbackHandle hMotionCB)
{
  static ActMotCBID nextID = 0;

  ActMotCBID id;

  if (!motIDRep.empty()){
    id = motIDRep.front();
    motIDRep.pop_front();
    motionCBs.insert(std::pair<ActMotCBID, ActionCallbackHandle>
		     (id, hMotionCB));
  }
  else{
    id = nextID++;
    motionCBs.insert(std::pair<ActMotCBID, ActionCallbackHandle>
		     (id, hMotionCB));
  }

  return id;
}

ActComCBID FireBallAction::registerCompleteCB(ActionCallbackHandle hCompleteCB)
{
  static ActComCBID nextID = 0;

  ActComCBID id;

  if (!comIDRep.empty()){
    id = comIDRep.front();
    comIDRep.pop_front();
    completeCBs.insert(std::pair<ActComCBID, ActionCallbackHandle>
		     (id, hCompleteCB));
  }
  else{
    id = nextID++;
    completeCBs.insert(std::pair<ActComCBID, ActionCallbackHandle>
		     (id, hCompleteCB));
  }

  return id;
}

void FireBallAction::unregisterMotionCB(ActMotCBID id)
{
  motionCBs.erase(id);
  motIDRep.push_back(id);
}

void FireBallAction::unregisterCompleteCB(ActComCBID id)
{
  completeCBs.erase(id);
  comIDRep.push_back(id);
}
