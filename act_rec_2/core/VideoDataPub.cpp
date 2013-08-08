/**
 * Author: Rajesh
 * Description: an implementation of VideoDataPub.hpp.
 */

#include "VideoDataPub.hpp"

using namespace rad;

#define CHECK_RC(nRetVal, what)					 \
  if ((nRetVal) != XN_STATUS_OK){				 \
    printf("%s failed: %s\n", what, xnGetStatusString(nRetVal)); \
    exit(nRetVal);						 \
  }

VideoDataPub* VideoDataPub::pInst = NULL;
static xn::Context context;
static xn::Player player;
static xn::ScriptNode scriptNode;
static xn::DepthGenerator depthGen;
static xn::UserGenerator userGen;
static XnBool needPose = FALSE;
static XnChar strPose[20] = "";

static void XN_CALLBACK_TYPE newUserCB(xn::UserGenerator &, XnUserID nId,
				       void* /*pCookie*/)
{
  XnUInt32 epochTime = 0;
  xnOSGetEpochTime(&epochTime);
  printf("%d New User %d\n", epochTime, nId);
  // New User found
  if (needPose){
    userGen.GetPoseDetectionCap().StartPoseDetection(strPose, nId);
  }
  else{
    userGen.GetSkeletonCap().RequestCalibration(nId, TRUE);
  }
}

static void XN_CALLBACK_TYPE lostUserCB(xn::UserGenerator &, XnUserID nId,
				      void* /*pCookie*/)
{
  XnUInt32 epochTime = 0;
  xnOSGetEpochTime(&epochTime);
  printf("%d Lost user %d\n", epochTime, nId);
}

static void XN_CALLBACK_TYPE poseDetectedCB(xn::PoseDetectionCapability &,
					    const XnChar* strPose,
					    XnUserID nId, void* /*pCookie*/)
{
  XnUInt32 epochTime = 0;
  xnOSGetEpochTime(&epochTime);
  printf("%d Pose %s detected for user %d\n", epochTime, strPose, nId);
  userGen.GetPoseDetectionCap().StopPoseDetection(nId);
  userGen.GetSkeletonCap().RequestCalibration(nId, TRUE);
}

static void XN_CALLBACK_TYPE calibrationStartCB(xn::SkeletonCapability &,
						XnUserID nId,
						void* /*pCookie*/)
{
  XnUInt32 epochTime = 0;
  xnOSGetEpochTime(&epochTime);
  printf("%d Calibration started for user %d\n", epochTime, nId);
}

static void XN_CALLBACK_TYPE calibrationCompleteCB(xn::SkeletonCapability &,
						   XnUserID nId,
						   XnCalibrationStatus eStatus,
						   void* /*pCookie*/)
{
  XnUInt32 epochTime = 0;
  xnOSGetEpochTime(&epochTime);

  if (eStatus == XN_CALIBRATION_STATUS_OK){
    // calibration succeeded
    printf("%d Calibration complete, start tracking user %d\n",
	   epochTime, nId);
    userGen.GetSkeletonCap().StartTracking(nId);
  }
  else{
    // calibration failed
    printf("%d Calibration failed for user %d\n", epochTime, nId);

    if (eStatus == XN_CALIBRATION_STATUS_MANUAL_ABORT){
      printf("Manual abort occured, stop attempting to calibrate!\n");
      return;
    }

    if (needPose)
      userGen.GetPoseDetectionCap().StartPoseDetection(strPose, nId);
    else
      userGen.GetSkeletonCap().RequestCalibration(nId, TRUE);
  }
}

VideoDataPub* VideoDataPub::getPtrToInst(const char* fn, bool setupOpenNI)
{
  if (pInst == NULL)
    pInst = new VideoDataPub(fn, setupOpenNI);

  return pInst;
}

void VideoDataPub::releaseInst()
{
  if (pInst != NULL){
    delete pInst;
    pInst = NULL;
  }
}

VideoDataPub::VideoDataPub(const char *fn, bool setupOpenNI)
{
  isEOF = false;

  if (setupOpenNI)
    initOpenNI(fn);
}

VideoDataPub::~VideoDataPub()
{
  scriptNode.Release();
  depthGen.Release();
  userGen.Release();
  context.Release();
}

xn::ProductionNode*
VideoDataPub::getPrdNode(XnPredefinedProductionNodeType type)
{
  switch (type){
  case XN_NODE_TYPE_DEPTH:
    return &depthGen;
  case XN_NODE_TYPE_USER:
    return &userGen;
  default:
    return NULL;
  }
}

bool VideoDataPub::isEndOfVideo()
{
  return isEOF;
}

inline void VideoDataPub::subscribe(SensorDataSub* pSub)
{
  subscribers.push_back(pSub);
}

inline void VideoDataPub::spinOnce()
{
  spin(true);
}

void VideoDataPub::spin(bool async)
{
  static bool onceThrough = false;
  XnStatus nRetVal = XN_STATUS_OK;

  if (!onceThrough){
    nRetVal = context.StartGeneratingAll();
    CHECK_RC(nRetVal, "StartGenerating");
    onceThrough = true;

    printf("Starting to run\n");
    if (needPose){
      printf("Assume calibration pose\n");
    }
  }

  XnUserID aUsers[MAX_NUM_USERS];
  XnUInt16 nUsers;
  SkeletonInfo skels[MAX_NUM_USERS];
  SensorMsg msg;
  xn::DepthMetaData depthMD;
  xn::SceneMetaData sceneMD;

  while(!xnOSWasKeyboardHit()){
    nRetVal = context.WaitAndUpdateAll();

    if (nRetVal == XN_STATUS_EOF){
      isEOF = true;
      return;
    }

    /**
     * nUsers is used both as input and output parameters.
     * When it's used as input parameter, it indicates the size of the user ID
     * array, while, when it's used as output parameter, it means the number
     * of users recognized. Therefore, it needs to be re-initialized in a loop.
     */
    nUsers = MAX_NUM_USERS;
    userGen.GetUsers(aUsers, nUsers);
    memset(&msg, 0, sizeof(SensorMsg));

    for (XnUInt16 i = 0; i < nUsers; ++i){
      if (userGen.GetSkeletonCap().IsTracking(aUsers[i])){
	// get the user's skeleton
	skels[i].user = aUsers[i];
	getSkeleton(skels[i]);
      }
      else
	skels[i].user = 0; // user is not tracked
    }

    depthGen.GetMetaData(depthMD); // depth map
    userGen.GetUserPixels(0, sceneMD); // labels

    msg.nUsers = nUsers; // num of users detected, either tracked or not
    msg.pSkels = skels;
    msg.pDepthMD = &depthMD;
    msg.pSceneMD = &sceneMD;

    for (int i = 0; i < subscribers.size(); ++i)
      subscribers[i]->notify(RAD_SENSOR_MSG, &msg);

    if (async)
      return;
  }
}

void VideoDataPub::getSkeleton(SkeletonInfo &skel)
{
  const xn::SkeletonCapability &sc = userGen.GetSkeletonCap();

  XnSkeletonJoint activeJoints[30];
  XnUInt16 nJoints = 30;

  sc.EnumerateActiveJoints(activeJoints, nJoints);

  std::vector<JointInfo> &joints = skel.joints;
  joints.resize(nJoints);

  for (int i = 0; i < joints.size(); ++i){
    joints[i].type = activeJoints[i];
    sc.GetSkeletonJoint(skel.user, joints[i].type, joints[i].pose);
  }
}

void VideoDataPub::initOpenNI(const char* fn)
{
  XnStatus nRetVal = XN_STATUS_OK;

  if (!fileExists(fn)){
    printf("Could not find '%s'. Aborting.\n", fn);
    exit(XN_STATUS_ERROR);
  }

  nRetVal = context.Init();
  CHECK_RC(nRetVal, "Init");

  nRetVal = context.OpenFileRecording(fn, player);
  CHECK_RC(nRetVal, "Open input file");
  printf("The video used is %s\n", fn);

  nRetVal = context.FindExistingNode(XN_NODE_TYPE_DEPTH, depthGen);
  CHECK_RC(nRetVal, "Find depth generator");

  nRetVal = context.FindExistingNode(XN_NODE_TYPE_USER, userGen);
  if (nRetVal != XN_STATUS_OK){
    nRetVal = userGen.Create(context);
    CHECK_RC(nRetVal, "Find user generator");
  }

  XnCallbackHandle hUserCallbacks, hCalibrationStart, hCalibrationComplete,
    hPoseDetected;

  if (!userGen.IsCapabilitySupported(XN_CAPABILITY_SKELETON)){
    printf("Supplied user generator doesn't support skeleton\n");
    exit(EXIT_FAILURE);
  }

  nRetVal = userGen.RegisterUserCallbacks(newUserCB, lostUserCB, NULL,
					  hUserCallbacks);
  CHECK_RC(nRetVal, "Register to user callbacks");
  nRetVal = userGen.GetSkeletonCap().
    RegisterToCalibrationStart(calibrationStartCB, NULL, hCalibrationStart);
  CHECK_RC(nRetVal, "Register to calibration start");
  nRetVal = userGen.GetSkeletonCap().
    RegisterToCalibrationComplete(calibrationCompleteCB, NULL,
				  hCalibrationComplete);
  CHECK_RC(nRetVal, "Register to calibration complete");

  if (userGen.GetSkeletonCap().NeedPoseForCalibration()){
    needPose = TRUE;

    if (!userGen.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION)){
      printf("Pose required, but not supported\n");
      exit(EXIT_FAILURE);
    }
    nRetVal = userGen.GetPoseDetectionCap().
      RegisterToPoseDetected(poseDetectedCB, NULL, hPoseDetected);
    CHECK_RC(nRetVal, "Register to Pose Detected");
    userGen.GetSkeletonCap().GetCalibrationPose(strPose);
  }

  userGen.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);
  userGen.GetSkeletonCap().SetSmoothing(0.1f);

  nRetVal = player.SetRepeat(FALSE);
}

XnBool VideoDataPub::fileExists(const char* fn)
{
  XnBool exists;
  xnOSDoesFileExist(fn, &exists);
  return exists;
}
