/**
 * Author: Rajesh
 * Description: an implementation of FireBallAction.hpp.
 */

#include <algorithm>

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "FireBallAction.hpp"
#include "RADFactory.hpp"

using namespace rad;

BiClassifier* pBC;

FireBallAction::FireBallAction()
{
  pBC = RADFactory::getPtrToInst()->
    getPtrToBiClassifier(RAD_BC_TYPE_FIREBALL_START);
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

void FireBallAction::getUserDiscriptor(XnUserID id,
				       const xn::DepthMetaData* pDepthMD,
				       const xn::SceneMetaData* pSceneMD,
				       cv::Mat &result)
{
  const xn::DepthMetaData &depthMD = *pDepthMD;
  const xn::SceneMetaData &sceneMD = *pSceneMD;

  const XnDepthPixel* pDPixels = depthMD.Data();
  const XnLabel* pLabels = sceneMD.Data();
  int xRes = depthMD.XRes(), yRes = depthMD.YRes(), zRes = depthMD.ZRes();
  int minx = xRes, miny = yRes, maxx = 0, maxy = 0, minz = zRes;

  for (int y = 0; y < yRes; ++y){
    for (int x = 0; x < xRes; ++x, ++pDPixels, ++pLabels){
      if (*pLabels != id)
	continue;

      if (x < minx)
	minx = x;
      if (y < miny)
	miny = y;
      if (x > maxx)
	maxx = x;
      if (y > maxy)
	maxy = y;
      if (*pDPixels != 0 && *pDPixels < minz)
	minz = *pDPixels;
    }
  }

  pDPixels = depthMD.Data() + miny*xRes + minx;
  pLabels = sceneMD.Data() + miny*xRes + minx;
  maxx++;
  maxy++;

  cv::Mat userPixels(maxy-miny, maxx-minx, CV_32FC1);

  for(int y = miny; y < maxy; ++y){
    for (int x = minx; x < maxx; ++x, ++pDPixels, ++pLabels){
      if (*pLabels != id || *pDPixels > minz+50)
	userPixels.at<float>(y-miny, x-minx) = 0.0f;
      else
	userPixels.at<float>(y-miny, x-minx) = zRes-(float)(*pDPixels-minz);
    }
    pDPixels += xRes - maxx + minx;
    pLabels += xRes - maxx + minx;
  }

  result.create(DESCRIPTOR_SIZE, DESCRIPTOR_SIZE, CV_32FC1);

  cv::resize(userPixels, result, result.size());

  //  cv::imshow("test", result);
  //  cv::waitKey(500);

  result = result.reshape(0, 1); // reshape to a row vector

  //  double maxNum;

  //  cv::minMaxIdx(result, NULL, &maxNum);
  //  result = result * (1.0f / maxNum);
}

 /*void FireBallAction::getUserDiscriptor(const std::vector<JointInfo> &joints,
				       cv::Mat &relJoints)
{
  // normalize with respect to the neck joint (assume exist and is confident)
  XnPoint3D neckPos;

  for (int i = 0; i < joints.size(); ++i){
    if (joints[i].type == XN_SKEL_NECK){
      neckPos = joints[i].pose.position.position;
      break;
    }
  }

  // a 1 x 42 matrix. Each 3 elements in a row represents the x, y, z offsets
  // of a joint relative to the neck joint. Since neck to neck is trivial,
  // this offset vector is not stored.
  relJoints.create(1, 42, CV_32FC1);

  for (int i = 0; i < joints.size(); ++i){
    const XnPoint3D &pt = joints[i].pose.position.position;

    switch (joints[i].type){
    case XN_SKEL_HEAD:
      relJoints.at<XnFloat>(0, 0) = pt.X-neckPos.X;
      relJoints.at<XnFloat>(0, 1) = pt.Y-neckPos.Y;
      relJoints.at<XnFloat>(0, 2) = pt.Z-neckPos.Z;
      break;
    case XN_SKEL_TORSO:
      relJoints.at<XnFloat>(0, 3) = pt.X-neckPos.X;
      relJoints.at<XnFloat>(0, 4) = pt.Y-neckPos.Y;
      relJoints.at<XnFloat>(0, 5) = pt.Z-neckPos.Z;
      break;
    case XN_SKEL_LEFT_SHOULDER:
      relJoints.at<XnFloat>(0, 6) = pt.X-neckPos.X;
      relJoints.at<XnFloat>(0, 7) = pt.Y-neckPos.Y;
      relJoints.at<XnFloat>(0, 8) = pt.Z-neckPos.Z;
      break;
    case XN_SKEL_LEFT_ELBOW:
      relJoints.at<XnFloat>(0, 9) = pt.X-neckPos.X;
      relJoints.at<XnFloat>(0, 10) = pt.Y-neckPos.Y;
      relJoints.at<XnFloat>(0, 11) = pt.Z-neckPos.Z;
      break;
    case XN_SKEL_LEFT_HAND:
      relJoints.at<XnFloat>(0, 12) = pt.X-neckPos.X;
      relJoints.at<XnFloat>(0, 13) = pt.Y-neckPos.Y;
      relJoints.at<XnFloat>(0, 14) = pt.Z-neckPos.Z;
      break;
    case XN_SKEL_RIGHT_SHOULDER:
      relJoints.at<XnFloat>(0, 15) = pt.X-neckPos.X;
      relJoints.at<XnFloat>(0, 16) = pt.Y-neckPos.Y;
      relJoints.at<XnFloat>(0, 17) = pt.Z-neckPos.Z;
      break;
    case XN_SKEL_RIGHT_ELBOW:
      relJoints.at<XnFloat>(0, 18) = pt.X-neckPos.X;
      relJoints.at<XnFloat>(0, 19) = pt.Y-neckPos.Y;
      relJoints.at<XnFloat>(0, 20) = pt.Z-neckPos.Z;
      break;
    case XN_SKEL_RIGHT_HAND:
      relJoints.at<XnFloat>(0, 21) = pt.X-neckPos.X;
      relJoints.at<XnFloat>(0, 22) = pt.Y-neckPos.Y;
      relJoints.at<XnFloat>(0, 23) = pt.Z-neckPos.Z;
      break;
    case XN_SKEL_LEFT_HIP:
      relJoints.at<XnFloat>(0, 24) = pt.X-neckPos.X;
      relJoints.at<XnFloat>(0, 25) = pt.Y-neckPos.Y;
      relJoints.at<XnFloat>(0, 26) = pt.Z-neckPos.Z;
      break;
    case XN_SKEL_LEFT_KNEE:
      relJoints.at<XnFloat>(0, 27) = pt.X-neckPos.X;
      relJoints.at<XnFloat>(0, 28) = pt.Y-neckPos.Y;
      relJoints.at<XnFloat>(0, 29) = pt.Z-neckPos.Z;
      break;
    case XN_SKEL_LEFT_FOOT:
      relJoints.at<XnFloat>(0, 30) = pt.X-neckPos.X;
      relJoints.at<XnFloat>(0, 31) = pt.Y-neckPos.Y;
      relJoints.at<XnFloat>(0, 32) = pt.Z-neckPos.Z;
      break;
    case XN_SKEL_RIGHT_HIP:
      relJoints.at<XnFloat>(0, 33) = pt.X-neckPos.X;
      relJoints.at<XnFloat>(0, 34) = pt.Y-neckPos.Y;
      relJoints.at<XnFloat>(0, 35) = pt.Z-neckPos.Z;
      break;
    case XN_SKEL_RIGHT_KNEE:
      relJoints.at<XnFloat>(0, 36) = pt.X-neckPos.X;
      relJoints.at<XnFloat>(0, 37) = pt.Y-neckPos.Y;
      relJoints.at<XnFloat>(0, 38) = pt.Z-neckPos.Z;
      break;
    case XN_SKEL_RIGHT_FOOT:
      relJoints.at<XnFloat>(0, 39) = pt.X-neckPos.X;
      relJoints.at<XnFloat>(0, 40) = pt.Y-neckPos.Y;
      relJoints.at<XnFloat>(0, 41) = pt.Z-neckPos.Z;
      break;
    default:
      // do nothing
      break;
    }
  }
}*/

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
  static bool isClassifierConfirmed = false;

  if (curStates[id] == UNRECOGNIZED){
    float handDist, halfTorsoLen, htDist;
    bool isSensorDataConfirmed = false;

    if (pLeftHand->position.fConfidence >= 0.5 &&
	pRightHand->position.fConfidence >= 0.5 &&
	pTorso->position.fConfidence >= 0.5 &&
	pNeck->position.fConfidence >= 0.5){
      // do the following things if sensory data is confident
      handDist = vecDist3f(lhPos.X, lhPos.Y, lhPos.Z,
			   rhPos.X, rhPos.Y, rhPos.Z);
      halfTorsoLen = vecDist3f(tPos.X, tPos.Y, tPos.Z,
			       nPos.X, nPos.Y, nPos.Z);
      htDist = vecDist3f((lhPos.X+rhPos.X)/2.0f, (lhPos.Y+rhPos.Y)/2.0f,
			 (lhPos.Z+rhPos.Z)/2.0f, tPos.X, tPos.Y, tPos.Z);
      
      if (((lhPos.X < tPos.X && rhPos.X < tPos.X) ||
	   (lhPos.X > tPos.X && rhPos.X > tPos.X)) &&
	  htDist < 1.5f * halfTorsoLen && handDist < 1.5f * halfTorsoLen){
	if (onceThroughs[id] && (lhPos.Z < startLeftHandPos[id].Z ||
				 rhPos.Z < startRightHandPos[id].Z)){
	  // regard as action start since both hands begin to move forward
	  curStates[id] = IN_PROGRESS;
	  abortTimers[id] = 0;
	  votes[id] = 0;
	  onceThroughs[id] = false;
	  isSensorDataConfirmed = true;
	}
	else{
	  if (!onceThroughs[id])
	    onceThroughs[id] = true;

	  startLeftHandPos[id] = lhPos;
	  startRightHandPos[id] = rhPos;
	  startNeckPos[id] = nPos;
	}
      }
    }

    if (!isSensorDataConfirmed){
      cv::Mat descriptor;

      getUserDiscriptor(id, pDepthMD, pSceneMD, descriptor);

      int result = pBC->predict(descriptor);

      if (result != 0){
	if (onceThroughs[id] && (lhPos.Z < startLeftHandPos[id].Z ||
				 rhPos.Z < startRightHandPos[id].Z)){
	  curStates[id] = IN_PROGRESS;
	  abortTimers[id] = 0;
	  votes[id] = 0;
	  onceThroughs[id] = false;
	  isClassifierConfirmed = true;
	  preLeftHandPos[id] = lhPos;
	  preRightHandPos[id] = rhPos;
	}
	else{
	  if (!onceThroughs[id])
	    onceThroughs[id] = true;

	  startLeftHandPos[id] = nPos;
	  startLeftHandPos[id].Z -= 200.0f;
	  startRightHandPos[id] = nPos;
	  startRightHandPos[id].Z -= 200.0f;
	  startNeckPos[id] = nPos;
	}
	//	startLeftHandPos[id] = startRightHandPos[id] =
	//	  {tPos.X, tPos.Y, tPos.Z - 200.0f};
      }
    }
  }
  else if (curStates[id] == IN_PROGRESS){
    if (pLeftHand->position.fConfidence < 0.5f ||
	pRightHand->position.fConfidence < 0.5f ||
	pNeck->position.fConfidence < 0.5f)
      return;

    float lhOffset = startLeftHandPos[id].Z - lhPos.Z;
    float rhOffset = startRightHandPos[id].Z - rhPos.Z;
    float nOffset = startNeckPos[id].Z - nPos.Z;

    if (!isClassifierConfirmed){
      if (lhOffset-nOffset > 250.0f || rhOffset-nOffset > 250.0f){
	// it seems that the goal is reached
	++(votes[id]);
      }
      else{
	// haven't reached the goal
	votes[id] = 0;
	++(abortTimers[id]);
      }
    }
    else{
      if ((preLeftHandPos[id].Z-lhPos.Z > 10 && lhOffset-nOffset > 250.0f) ||
	  (preRightHandPos[id].Z-rhPos.Z > 10 && rhOffset-nOffset > 250.0f)){
	// it seems that the goal is reached
	votes[id]++;
      }
      else{
	// haven't reached the goal
	votes[id] = 0;
	++(abortTimers[id]);
      }

      preLeftHandPos[id] = lhPos;
      preRightHandPos[id] = rhPos;
    }

    if (votes[id] >= 2){
      // goal reached, invoke each complete callback
      for (std::map<ActComCBID, ActionCallbackHandle>::iterator it =
	     completeCBs.begin(); it != completeCBs.end(); ++it)
	it->second(RAD_SKEL_INFO, 1, pSkel);

	isClassifierConfirmed = false;
	curStates[id] = UNRECOGNIZED;
    }
    else if (abortTimers[id] >= numOfFramesToAbort){
      // abort the action since it lasts too long without reaching the goal
      isClassifierConfirmed = false;
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
