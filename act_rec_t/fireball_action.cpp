/**
 * Author: Rajesh
 * Description: recognize the fireball action. Don't use more than
 *   one thread to execute it.
 */

#include "common.hpp"

const int numOfFramesToAbort = 45;
std::map<XnUserID, int> abortTimers;
std::map<XnUserID, ActionState> curStates;
std::list<ActComCBID> comIDRep;
std::map<ActComCBID, ActionCB> completeCBs;
std::map<XnUserID, XnPoint3D> startLeftHandPos, startRightHandPos;
std::map<XnUserID, double> startDepthVals;

extern SensorMsg g_msg;
extern pthread_mutex_t g_msgMutex;

void updateState(const SkeletonInfo* pSkel, const xn::DepthMetaData* pDepthMD,
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

    if ((lhOffset > 300.0f && rhOffset > 400.0f) ||
	(lhOffset > 400.0f && rhOffset > 300.0f)){
      ++(votes[id]);
    }
    else{
      // haven't reached the goal
      votes[id] = 0;
      ++(abortTimers[id]);
    }

    if (votes[id] >= 2){
      // goal reached, invoke each complete callback
      static pthread_t threads[MAX_NUM_ACT_CBS];
      pthread_attr_t attr;
      int rc, count = 0;

      pthread_attr_init(&attr);
      pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

      for (std::map<ActComCBID, ActionCB>::iterator it =
	     completeCBs.begin(); it != completeCBs.end(); ++it){
	rc = pthread_create(&threads[count++], &attr, it->second,
			    (void*)id);

	if (rc != 0){
	  printf("ERROR: thread creation failed with rc = %d\n", rc);
	  exit(EXIT_FAILURE);
	}
      }
     
      pthread_attr_destroy(&attr);

      curStates[id] = UNRECOGNIZED;
    }
    else if (abortTimers[id] >= numOfFramesToAbort){
      // abort the action since it lasts too long without reaching the goal
      curStates[id] = UNRECOGNIZED;
    }
  }
}

void* fba_notify(void* tid)
{
  if (!completeCBs.empty()){
    XnUInt16 nUsers = g_msg.nUsers;
    const std::vector<SkeletonInfo> &skels = g_msg.skels;
    const xn::DepthMetaData* pDepthMD = &g_msg.depthMD;
    const xn::SceneMetaData* pSceneMD = &g_msg.sceneMD;

    for (int i = 0; i < nUsers; ++i)
      if (skels[i].user != 0)
	updateState(&skels[i], pDepthMD, pSceneMD);
  }

  pthread_exit(NULL);
}

ActComCBID fba_registerCompleteCB(ActionCB hCompleteCB)
{
  static ActComCBID nextID = 0;

  ActComCBID id;

  if (!comIDRep.empty()){
    id = comIDRep.front();
    comIDRep.pop_front();
    completeCBs.insert(std::pair<ActComCBID, ActionCB>
		     (id, hCompleteCB));
  }
  else{
    id = nextID++;
    completeCBs.insert(std::pair<ActComCBID, ActionCB>
		     (id, hCompleteCB));
  }

  return id;
}

void fba_unregisterCompleteCB(ActComCBID id)
{
  completeCBs.erase(id);
  comIDRep.push_back(id);
}
