/**
 * Author: Rajesh
 * Description: publish sensory data into shared memory using OpenNI
 */

#include "common.hpp"

#define SPIN_ONCE

#define CHECK_RC(nRetVal, what)					 \
  if ((nRetVal) != XN_STATUS_OK){				 \
    printf("%s failed: %s\n", what, xnGetStatusString(nRetVal)); \
    exit(nRetVal);						 \
  }

// variables shared by threads                                                 
SensorMsg g_msg;
pthread_mutex_t g_msgMutex;

xn::Context g_context;
xn::ScriptNode g_scriptNode;
xn::DepthGenerator g_depthGen;
xn::UserGenerator g_userGen;
XnBool g_bNeedPose = FALSE;
XnChar g_strPose[20] = "";
std::vector<SubscriberCB> subs;

void nipub_destroy()
{
  g_scriptNode.Release();
  g_depthGen.Release();
  g_userGen.Release();
  g_context.Release();
}

void nipub_subscribe(SubscriberCB sub)
{
  subs.push_back(sub);
}

void getSkeleton(SkeletonInfo &skel)
{
  const xn::SkeletonCapability &sc = g_userGen.GetSkeletonCap();

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

void* nipub_spin(void* tid)
{
  static bool onceThrough = false;

  if (!onceThrough){
    XnStatus nRetVal = XN_STATUS_OK;

    nRetVal = g_context.StartGeneratingAll();
    CHECK_RC(nRetVal, "StartGenerating");

    printf("Starting to run\n");
    if (g_bNeedPose){
      printf("Assume calibration pose\n");
    }

    onceThrough = true;
  }

  XnUserID aUsers[MAX_NUM_USERS];
  XnUInt16 nUsers;
  pthread_t threads[MAX_NUM_SUBS];
  pthread_attr_t attr;

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  while(!xnOSWasKeyboardHit()){
    g_context.WaitOneUpdateAll(g_userGen);

    /**
     * nUsers is used both as input and output parameters.
     * When it's used as input parameter, it indicates the size of the user ID
     * array, while, when it's used as output parameter, it means the number
     * of users recognized. Therefore, it needs to be re-initialized in a loop.
     */
    nUsers = MAX_NUM_USERS;
    g_userGen.GetUsers(aUsers, nUsers);

    pthread_mutex_lock(&g_msgMutex);

    g_msg.nUsers = nUsers; // num of users detected, either tracked or not
    g_msg.skels.resize(nUsers);

    for (XnUInt16 i = 0; i < nUsers; ++i){
      if (g_userGen.GetSkeletonCap().IsTracking(aUsers[i])){
	// get the user's skeleton
	g_msg.skels[i].user = aUsers[i];
	getSkeleton(g_msg.skels[i]);
      }
      else
	g_msg.skels[i].user = 0; // user is not tracked
    }

    g_depthGen.GetMetaData(g_msg.depthMD); // depth map
    g_userGen.GetUserPixels(0, g_msg.sceneMD); // labels

    pthread_mutex_unlock(&g_msgMutex);

    int rc;

    for (int i = 0; i < subs.size(); ++i){
      rc = pthread_create(&threads[i], &attr, subs[i], (void*)i);

      if (rc != 0){
	printf("ERROR: thread creation failed with rc = %d\n", rc);
	exit(EXIT_FAILURE);
      }
    }

    for (int i = 0; i < subs.size(); ++i){
      rc = pthread_join(threads[i], NULL);

      if (rc != 0){
	printf("ERROR: failed to join thread with rc = %d\n", rc);
	exit(EXIT_FAILURE);
      }
    }

#ifdef SPIN_ONCE
    pthread_attr_destroy(&attr);
    return NULL;
#endif
  } // while

  pthread_attr_destroy(&attr);
}

XnBool fileExists(const char* fn)
{
  XnBool exists;
  xnOSDoesFileExist(fn, &exists);
  return exists;
}

void XN_CALLBACK_TYPE newUserCB(xn::UserGenerator &, XnUserID nId,
				void* /*pCookie*/)
{
  XnUInt32 epochTime = 0;
  xnOSGetEpochTime(&epochTime);
  printf("%d New User %d\n", epochTime, nId);
  // New User found
  if (g_bNeedPose){
    g_userGen.GetPoseDetectionCap().StartPoseDetection(g_strPose, nId);
  }
  else{
    g_userGen.GetSkeletonCap().RequestCalibration(nId, TRUE);
  }
}

void XN_CALLBACK_TYPE lostUserCB(xn::UserGenerator &, XnUserID nId,
				 void* /*pCookie*/)
{
  XnUInt32 epochTime = 0;
  xnOSGetEpochTime(&epochTime);
  printf("%d Lost user %d\n", epochTime, nId);
}

void XN_CALLBACK_TYPE poseDetectedCB(xn::PoseDetectionCapability &,
				     const XnChar* strPose,
				     XnUserID nId, void* /*pCookie*/)
{
  XnUInt32 epochTime = 0;
  xnOSGetEpochTime(&epochTime);
  printf("%d Pose %s detected for user %d\n", epochTime, strPose, nId);
  g_userGen.GetPoseDetectionCap().StopPoseDetection(nId);
  g_userGen.GetSkeletonCap().RequestCalibration(nId, TRUE);
}

void XN_CALLBACK_TYPE calibrationStartCB(xn::SkeletonCapability &,
					 XnUserID nId,
					 void* /*pCookie*/)
{
  XnUInt32 epochTime = 0;
  xnOSGetEpochTime(&epochTime);
  printf("%d Calibration started for user %d\n", epochTime, nId);
}

void XN_CALLBACK_TYPE calibrationCompleteCB(xn::SkeletonCapability &,
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
    g_userGen.GetSkeletonCap().StartTracking(nId);
  }
  else{
    // calibration failed
    printf("%d Calibration failed for user %d\n", epochTime, nId);

    if (eStatus == XN_CALIBRATION_STATUS_MANUAL_ABORT){
      printf("Manual abort occured, stop attempting to calibrate!\n");
      return;
    }

    if (g_bNeedPose)
      g_userGen.GetPoseDetectionCap().StartPoseDetection(g_strPose, nId);
    else
      g_userGen.GetSkeletonCap().RequestCalibration(nId, TRUE);
  }
}

void nipub_init(const char* fn)
{
  XnStatus nRetVal = XN_STATUS_OK;
  xn::EnumerationErrors errors;

  if (!fileExists(fn)){
    printf("Could not find '%s'. Aborting.\n", fn);
    exit(XN_STATUS_ERROR);
  }

  nRetVal = g_context.InitFromXmlFile(fn, g_scriptNode, &errors);
  if (nRetVal == XN_STATUS_NO_NODE_PRESENT){
    XnChar strError[1024];
    errors.ToString(strError, 1024);
    printf("%s\n", strError);
    exit(nRetVal);
  }
  else if (nRetVal != XN_STATUS_OK){
    printf("Open failed: %s\n", xnGetStatusString(nRetVal));
    exit(nRetVal);
  }

  printf("Reading config from '%s'\n", fn);

  nRetVal = g_context.FindExistingNode(XN_NODE_TYPE_DEPTH, g_depthGen);
  CHECK_RC(nRetVal, "Find depth generator");

  nRetVal = g_context.FindExistingNode(XN_NODE_TYPE_USER, g_userGen);
  if (nRetVal != XN_STATUS_OK){
    nRetVal = g_userGen.Create(g_context);
    CHECK_RC(nRetVal, "Find user generator");
  }

  XnCallbackHandle hUserCallbacks, hCalibrationStart, hCalibrationComplete,
    hPoseDetected;

  if (!g_userGen.IsCapabilitySupported(XN_CAPABILITY_SKELETON)){
    printf("Supplied user generator doesn't support skeleton\n");
    exit(EXIT_FAILURE);
  }

  nRetVal = g_userGen.RegisterUserCallbacks(newUserCB, lostUserCB, NULL,
					  hUserCallbacks);
  CHECK_RC(nRetVal, "Register to user callbacks");
  nRetVal = g_userGen.GetSkeletonCap().
    RegisterToCalibrationStart(calibrationStartCB, NULL, hCalibrationStart);
  CHECK_RC(nRetVal, "Register to calibration start");
  nRetVal = g_userGen.GetSkeletonCap().
    RegisterToCalibrationComplete(calibrationCompleteCB, NULL,
				  hCalibrationComplete);
  CHECK_RC(nRetVal, "Register to calibration complete");

  if (g_userGen.GetSkeletonCap().NeedPoseForCalibration()){
    g_bNeedPose = TRUE;
    if (!g_userGen.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION)){
      printf("Pose required, but not supported\n");
      exit(EXIT_FAILURE);
    }
    nRetVal = g_userGen.GetPoseDetectionCap().
      RegisterToPoseDetected(poseDetectedCB, NULL, hPoseDetected);
    CHECK_RC(nRetVal, "Register to Pose Detected");
    g_userGen.GetSkeletonCap().GetCalibrationPose(g_strPose);
  }

  g_userGen.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);
  g_userGen.GetSkeletonCap().SetSmoothing(0.1f);
}
