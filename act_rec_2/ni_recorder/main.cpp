/****************************************************************************
*                                                                           *
*  OpenNI 1.x Alpha                                                         *
*  Copyright (C) 2011 PrimeSense Ltd.                                       *
*                                                                           *
*  This file is part of OpenNI.                                             *
*                                                                           *
*  OpenNI is free software: you can redistribute it and/or modify           *
*  it under the terms of the GNU Lesser General Public License as published *
*  by the Free Software Foundation, either version 3 of the License, or     *
*  (at your option) any later version.                                      *
*                                                                           *
*  OpenNI is distributed in the hope that it will be useful,                *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the             *
*  GNU Lesser General Public License for more details.                      *
*                                                                           *
*  You should have received a copy of the GNU Lesser General Public License *
*  along with OpenNI. If not, see <http://www.gnu.org/licenses/>.           *
*                                                                           *
*  This file is modified by Rajesh to allow the program to record human     *
*  skeleton. The skeleton data is stored in a .skel file that has the same  *
*  name as the corresponding .oni file except the extension. A .skel file   *
*  start with a number indicating the number of frames, after which, are a  *
*  number of skeleton (i.e. UserSkel = an array of 24                       *
*  XnSkeletonJointTransformation structures). Note that the number of       *
*  UserSkel objects equals to the number of frames, not all joints are      *
*  active (find active joints using                                         *
*  xn::SkeletonCapability::EnumerateActiveJoints() based on the data from   *
*  the corresponding .oni file, and each skeleton frame corresponding to    *
*  the depth frame in the .oni file with the same index.                    *
****************************************************************************/
//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnOpenNI.h>
#include <XnLog.h>
#include <XnCodecIDs.h>
#include <XnCppWrapper.h>
#include <GLFW/glfw3.h>

#include "scene_drawer/SceneDrawer.hpp"

//---------------------------------------------------------------------------
// Macros
//---------------------------------------------------------------------------
#define CHECK_RC(rc, what)						\
  if (rc != XN_STATUS_OK)						\
    {									\
      printf("%s failed: %s\n", what, xnGetStatusString(rc));		\
      return rc;							\
    }

#define CHECK_RC_ERR(rc, what, errors)					\
{									\
  if (rc == XN_STATUS_NO_NODE_PRESENT)					\
    {									\
      XnChar strError[1024];						\
      errors.ToString(strError, 1024);					\
      printf("%s\n", strError);						\
    }									\
  CHECK_RC(rc, what)							\
}

#define MAX_NUM_USERS 1 // allow only one user
#define MAX_NUM_JOINTS 24

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------

void printUsage(XnChar* strName)
{
	printf("%s "
	       "time <seconds> [depth [qvga|vga]] [image [qvga|vga]] [verbose] "
	       "[mirror <on|off>] [registration] [framesync] [outdir <directory>]\n", strName);
}

XnMapOutputMode QVGAMode = { 320, 240, 30 };
XnMapOutputMode VGAMode = { 640, 480, 30 };
XnBool g_bNeedPose = FALSE;
XnChar g_strPose[20] = "";
xn::UserGenerator g_userGen;
xn::DepthGenerator depthGenerator;

rad::SceneDrawer g_sceneDrawer;

// Configuration
struct RecConfiguration
{
  RecConfiguration()
  {
    pDepthMode = &QVGAMode;
    pImageMode = &QVGAMode;
    bRecordDepth = FALSE;
    bRecordImage = FALSE;
		
    bMirrorIndicated = FALSE;
    bMirror = TRUE;

    bRegister = FALSE;
    bFrameSync = FALSE;
    bVerbose = FALSE;

    nDumpTime = 0;
    sprintf(strDirName, ".");
  }
  XnMapOutputMode* pDepthMode;
  XnMapOutputMode* pImageMode;
  XnBool bRecordDepth;
  XnBool bRecordImage;

  XnBool bMirrorIndicated;
  XnBool bMirror;
  XnBool bRegister;
  XnBool bFrameSync;
  XnBool bVerbose;

  XnUInt32 nDumpTime;
  XnChar strDirName[XN_FILE_MAX_PATH];
};

// Parse the command line arguments
XnBool ParseArgs(int argc, char** argv, RecConfiguration& config)
{
  XnBool bError = FALSE;

  for (int i = 1; i < argc; ++i){
    if (xnOSStrCaseCmp(argv[i], "time") == 0){
      // Set the time of each recording
      if (argc > i+1){
	config.nDumpTime = atoi(argv[i+1]);
	if (config.nDumpTime == 0)
	  bError = TRUE;
	i++;
      }
      else{
	bError = TRUE;
      }
    }
    else if (xnOSStrCaseCmp(argv[i], "depth") == 0){
      // Set depth resolution (default is QVGA)
      if (argc > i+1){
	if (xnOSStrCaseCmp(argv[i+1], "vga") == 0){
	  config.pDepthMode = &VGAMode;
	  ++i;
	}
	else if (xnOSStrCaseCmp(argv[i+1], "qvga") ==0){
	  config.pDepthMode = &QVGAMode;
	  ++i;
	}
      }

      config.bRecordDepth = TRUE;
    }
    else if (xnOSStrCaseCmp(argv[i], "image") == 0){
      // Set image resolution (default is QVGA)
      if (argc > i+1){
	if (xnOSStrCaseCmp(argv[i+1], "vga") == 0){
	  config.pImageMode = &VGAMode;
	  ++i;
	}
	else if (xnOSStrCaseCmp(argv[i+1], "qvga") ==0){
	  config.pImageMode = &QVGAMode;
	  ++i;
	}
      }
      config.bRecordImage = TRUE;
    }
    else if (xnOSStrCaseCmp(argv[i], "verbose") == 0){
      // Control the log
      config.bVerbose = TRUE;
    }
    else if (xnOSStrCaseCmp(argv[i], "mirror") == 0){
      // Set mirror mode (does nothing if missing)
      config.bMirrorIndicated = TRUE;
      if (argc > i+1){
	if (xnOSStrCaseCmp(argv[i+1], "on") == 0){
	  config.bMirror = TRUE;
	  ++i;
	}
	else if (xnOSStrCaseCmp(argv[i+1], "off") ==0){
	  config.bMirror = FALSE;
	  ++i;
	}
      }
    }
    else if (xnOSStrCaseCmp(argv[i], "registration") == 0){
      // Set registration of depth to image
      config.bRegister = TRUE;
    }
    else if (xnOSStrCaseCmp(argv[i], "framesync") == 0){
      // Sync the image and the depth
      config.bFrameSync = TRUE;
    }
    else if (xnOSStrCaseCmp(argv[i], "outdir") == 0){
      // Set the directory in which the files will be created
      if (argc > i+1){
	xnOSStrCopy(config.strDirName, argv[i+1], XN_FILE_MAX_PATH);
	++i;
      }
    }
    else{
      printf("Unknown option %s\n", argv[i]);
    }
  }

  // Must record something!
  if (!config.bRecordDepth && !config.bRecordImage){
    printf("Recording nothing!\n");
    printUsage(argv[0]);
    return FALSE;
  }
  // No default time
  if (config.nDumpTime == 0){
    printf("Missing time\n");
    return FALSE;
  }

  return !bError;
}

void XN_CALLBACK_TYPE newUserCB(xn::UserGenerator &/*user*/, XnUserID nId,
				void* /*pCookie*/)
{
  XnUInt32 epochTime = 0;

  xnOSGetEpochTime(&epochTime);
  printf("%d New User %d\n", epochTime, nId);

  if (g_bNeedPose)
    g_userGen.GetPoseDetectionCap().StartPoseDetection(g_strPose, nId);
  else
    g_userGen.GetSkeletonCap().RequestCalibration(nId, TRUE);
}

void XN_CALLBACK_TYPE lostUserCB(xn::UserGenerator &/*user*/, XnUserID nId,
				 void* /*pCookie*/)
{
  XnUInt32 epochTime;

  xnOSGetEpochTime(&epochTime);
  printf("%d Lost user %d\n", epochTime, nId);
}

void XN_CALLBACK_TYPE poseDetectedCB(xn::PoseDetectionCapability &/*pose*/,
				     const XnChar* strPose, XnUserID nId,
				     void* /*pCookie*/)
{
  XnUInt32 epochTime = 0;

  xnOSGetEpochTime(&epochTime);
  printf("%d Pose %s detected for user %d\n", epochTime, strPose, nId);
  g_userGen.GetPoseDetectionCap().StopPoseDetection(nId);
  g_userGen.GetSkeletonCap().RequestCalibration(nId, TRUE);
}

void XN_CALLBACK_TYPE calibrationStartCB(xn::SkeletonCapability &/*skel*/,
					 XnUserID nId, void* /*pCookie*/)
{
  XnUInt32 epochTime = 0;
  xnOSGetEpochTime(&epochTime);
  printf("%d Calibration started for user %d\n", epochTime, nId);
}

void XN_CALLBACK_TYPE calibrationCompleteCB(xn::SkeletonCapability &/*skel*/,
					    XnUserID nId,
					    XnCalibrationStatus eStatus,
					    void* /*pCookie*/)
{
  XnUInt32 epochTime = 0;

  xnOSGetEpochTime(&epochTime);

  if (eStatus == XN_CALIBRATION_STATUS_OK){
    printf("%d Calibration complete, start tracking user %d\n", epochTime, nId);
    g_userGen.GetSkeletonCap().StartTracking(nId);
  }
  else{
    printf("%d Calibration failed for user %d\n", epochTime, nId);

    if (eStatus == XN_CALIBRATION_STATUS_MANUAL_ABORT){
      printf("Manual abort occured, stop attempting to calibrate\n");
      return;
    }

    if (g_bNeedPose)
      g_userGen.GetPoseDetectionCap().StartPoseDetection(g_strPose, nId);
    else
      g_userGen.GetSkeletonCap().RequestCalibration(nId, TRUE);
  }
}

XnStatus ConfigureGenerators(const RecConfiguration& config, xn::Context& context,
			     xn::DepthGenerator& depthGenerator,
			     xn::UserGenerator &userGen,
			     xn::ImageGenerator& imageGenerator)
{
  XnStatus nRetVal = XN_STATUS_OK;
  xn::EnumerationErrors errors;

  // Configure the depth, if needed
  if (config.bRecordDepth){
    nRetVal = context.CreateAnyProductionTree(XN_NODE_TYPE_DEPTH, NULL,
					      depthGenerator, &errors);
    CHECK_RC_ERR(nRetVal, "Create Depth", errors);
    nRetVal = depthGenerator.SetMapOutputMode(*config.pDepthMode);
    CHECK_RC(nRetVal, "Set Mode");
    if (config.bMirrorIndicated &&
	depthGenerator.IsCapabilitySupported(XN_CAPABILITY_MIRROR)){
      depthGenerator.GetMirrorCap().SetMirror(config.bMirror);
    }

    nRetVal = context.CreateAnyProductionTree(XN_NODE_TYPE_USER, NULL,
					      userGen, &errors);
    CHECK_RC_ERR(nRetVal, "Create User", errors);

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
      g_bNeedPose = TRUE;

      if (!userGen.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION)){
	printf("Pose required, but not supported\n");
	exit(EXIT_FAILURE);
      }

      nRetVal = userGen.GetPoseDetectionCap().
	RegisterToPoseDetected(poseDetectedCB, NULL, hPoseDetected);
      CHECK_RC(nRetVal, "Register to pose detected");
      userGen.GetSkeletonCap().GetCalibrationPose(g_strPose);
    }

    userGen.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);

    // Set Hole Filter
    depthGenerator.SetIntProperty("HoleFilter", TRUE);
  }
  // Configure the image, if needed
  if (config.bRecordImage){
    nRetVal = context.CreateAnyProductionTree(XN_NODE_TYPE_IMAGE, NULL, imageGenerator, &errors);
    CHECK_RC_ERR(nRetVal, "Create Image", errors);
    nRetVal = imageGenerator.SetMapOutputMode(*config.pImageMode);
    CHECK_RC(nRetVal, "Set Mode");

    if (config.bMirrorIndicated && imageGenerator.IsCapabilitySupported(XN_CAPABILITY_MIRROR)){
      imageGenerator.GetMirrorCap().SetMirror(config.bMirror);
    }
  }

  // Configuration for when there are both streams
  if (config.bRecordDepth && config.bRecordImage){
    // Registration
    if (config.bRegister && depthGenerator.IsCapabilitySupported(XN_CAPABILITY_ALTERNATIVE_VIEW_POINT)){
      nRetVal = depthGenerator.GetAlternativeViewPointCap().SetViewPoint(imageGenerator);
      CHECK_RC(nRetVal, "Registration");
    }
    // Frame Sync
    if (config.bFrameSync && depthGenerator.IsCapabilitySupported(XN_CAPABILITY_FRAME_SYNC)){
      if (depthGenerator.GetFrameSyncCap().CanFrameSyncWith(imageGenerator)){
	nRetVal = depthGenerator.GetFrameSyncCap().FrameSyncWith(imageGenerator);
	CHECK_RC(nRetVal, "Frame sync");
      }
    }
  }

  return XN_STATUS_OK;
}

void getSkeleton(rad::SkeletonInfo &skel)
{
  const xn::SkeletonCapability &sc = g_userGen.GetSkeletonCap();

  XnSkeletonJoint activeJoints[30];
  XnUInt16 nJoints = 30;

  sc.EnumerateActiveJoints(activeJoints, nJoints);

  std::vector<rad::JointInfo> &joints = skel.joints;
  joints.resize(nJoints);

  for (int i = 0; i < joints.size(); ++i){
    joints[i].type = activeJoints[i];
    sc.GetSkeletonJoint(skel.user, joints[i].type, joints[i].pose);
  }
}

// The cyclic buffer, to which frames will be added and from where they will be dumped to files
class CyclicBuffer
{
public:
  // Creation - set the OpenNI objects
  CyclicBuffer(xn::Context& context, xn::DepthGenerator& depthGenerator,
	       xn::ImageGenerator& imageGenerator,
	       const RecConfiguration& config) :
    m_context(context),
    m_depthGenerator(depthGenerator),
    m_imageGenerator(imageGenerator),
    m_pFrames(NULL)
  {
    m_bDepth = config.bRecordDepth;
    m_bImage = config.bRecordImage;
    m_nNextWrite = 0;
    m_nBufferSize = 0;
    m_nBufferCount = 0;
  }

  // Initialization - set outdir and time of each recording
  void Initialize(XnChar* strDirName, XnUInt32 nSeconds)
  {
    xnOSStrCopy(m_strDirName, strDirName, XN_FILE_MAX_PATH);
    m_nBufferSize = nSeconds*30;
    m_pFrames = XN_NEW_ARR(SingleFrame, m_nBufferSize);
  }
  // Save new data from OpenNI
  void Update(const xn::DepthGenerator& depthGenerator,
	      const xn::ImageGenerator& imageGenerator)
  {
    if (m_bDepth){
      // Save latest depth frame
      xn::DepthMetaData dmd;
      depthGenerator.GetMetaData(dmd);
      m_pFrames[m_nNextWrite].depthFrame.CopyFrom(dmd);

      XnUserID aUsers[MAX_NUM_USERS];
      XnUInt16 nUsers = MAX_NUM_USERS;
      rad::SkeletonInfo skels[MAX_NUM_USERS];
      rad::SensorMsg msg;

      g_userGen.GetUsers(aUsers, nUsers);
      memset(&msg, 0, sizeof(rad::SensorMsg));

      for (int i = 0; i < nUsers; ++i){
	if (!g_userGen.GetSkeletonCap().IsTracking(aUsers[i])){
	  skels[i].user = 0;
	  continue;
	}

	skels[i].user = aUsers[i];
	getSkeleton(skels[i]);
      }

      xn::SceneMetaData smd;

      g_userGen.GetUserPixels(0, smd);

      msg.nUsers = nUsers;
      msg.pSkels = skels;
      msg.pDepthMD = &dmd;
      msg.pSceneMD = &smd;

      g_sceneDrawer.notify(rad::RAD_SENSOR_MSG, &msg);
    }

    if (m_bImage){
      // Save latest image frame
      xn::ImageMetaData imd;
      imageGenerator.GetMetaData(imd);
      m_pFrames[m_nNextWrite].imageFrame.CopyFrom(imd);
    }

    // See if buffer is already full
    if (m_nBufferCount < m_nBufferSize){
      m_nBufferCount++;
    }
    // Make sure cylic buffer pointers are good
    m_nNextWrite++;
    if (m_nNextWrite == m_nBufferSize){
      m_nNextWrite = 0;
    }
  }

  // Save the current state of the buffer to a file
  XnStatus Dump()
  {
    xn::MockDepthGenerator mockDepth;
    xn::MockImageGenerator mockImage;

    xn::EnumerationErrors errors;
    XnStatus rc;

    // Create recorder
    rc = m_context.CreateAnyProductionTree(XN_NODE_TYPE_RECORDER, NULL,
					   m_recorder, &errors);
    CHECK_RC_ERR(rc, "Create recorder", errors);

    // Create name of new file
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    XnChar strFileName[XN_FILE_MAX_PATH];

    sprintf(strFileName, "%s/%04d%02d%02d-%02d%02d%02d.oni", m_strDirName,
	    timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday,
	    timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

    m_recorder.SetDestination(XN_RECORD_MEDIUM_FILE, strFileName);
    printf("Creating file %s\n", strFileName);

    if (m_bDepth){
      // Create mock nodes based on the depth generator, to save depth
      rc = m_context.CreateMockNodeBasedOn(m_depthGenerator, NULL, mockDepth);
      CHECK_RC(rc, "Create depth node");
      rc = m_recorder.AddNodeToRecording(mockDepth, XN_CODEC_16Z_EMB_TABLES);
      CHECK_RC(rc, "Add depth node");

      // Record user generator
      //      rc = m_recorder.AddNodeToRecording(g_userGen);
    }

    if (m_bImage){
      // Create mock nodes based on the image generator, to save image
      rc = m_context.CreateMockNodeBasedOn(m_imageGenerator, NULL, mockImage);
      CHECK_RC(rc, "Create image node");
      rc = m_recorder.AddNodeToRecording(mockImage, XN_CODEC_JPEG);
      CHECK_RC(rc, "Add image node");
    }

    // Write frames from next index (which will be next to be written, and so the first available)
    // this is only if a full loop was done, and this frame has meaningful data
    if (m_nNextWrite < m_nBufferCount){
      printf("Buffer wrapped around!\n");

      // Not first loop, right till end
      for (XnUInt32 i = m_nNextWrite; i < m_nBufferSize; ++i){
	if (m_bDepth){
	  mockDepth.SetData(m_pFrames[i].depthFrame);
	}
	if (m_bImage){
	  mockImage.SetData(m_pFrames[i].imageFrame);
	}

	m_recorder.Record();
      }
    }
    // Write frames from the beginning of the buffer to the last on written
    for (XnUInt32 i = 0; i < m_nNextWrite; ++i){
      if (m_bDepth){
	mockDepth.SetData(m_pFrames[i].depthFrame);
      }
      if (m_bImage){
	mockImage.SetData(m_pFrames[i].imageFrame);
      }

      m_recorder.Record();
    }

    // Close recorder
    m_recorder.Release();

    return XN_STATUS_OK;
  }
protected:
  struct SingleFrame
  {
    xn::DepthMetaData depthFrame;
    xn::ImageMetaData imageFrame;
  };

  XnBool m_bDepth, m_bImage;
  SingleFrame* m_pFrames;
  XnUInt32 m_nNextWrite;
  XnUInt32 m_nBufferSize;
  XnUInt32 m_nBufferCount;
  XnChar m_strDirName[XN_FILE_MAX_PATH];

  xn::Context& m_context;
  xn::DepthGenerator& m_depthGenerator;
  xn::ImageGenerator& m_imageGenerator;
  xn::Recorder m_recorder;

private:
  XN_DISABLE_COPY_AND_ASSIGN(CyclicBuffer);
};

void resizeCB(GLFWwindow* win, int w, int h)
{
  glViewport(0, 0, w, h);
}

// The recorder
int main(int argc, char** argv)
{
  // OpenNi objects
  xn::Context context;
  xn::ImageGenerator imageGenerator;

  // To count missed frames
  XnUInt64 nLastDepthTime = 0;
  XnUInt64 nLastImageTime = 0;
  XnUInt32 nMissedDepthFrames = 0;
  XnUInt32 nMissedImageFrames = 0;
  XnUInt32 nDepthFrames = 0;
  XnUInt32 nImageFrames = 0;

  RecConfiguration config;

  XnStatus nRetVal = XN_STATUS_OK;

  // Parse the command line arguments
  if (!ParseArgs(argc, argv, config)){
    printf("Parse error\n");
    return 1;
  }

  if (config.bVerbose){
    // Turn on log
    xnLogInitSystem();
    xnLogSetConsoleOutput(TRUE);
    xnLogSetMaskMinSeverity(XN_LOG_MASK_ALL, XN_LOG_VERBOSE);
  }

  // Initialize OpenNI
  nRetVal = context.Init();
  CHECK_RC(nRetVal, "Init");

  nRetVal = ConfigureGenerators(config, context, depthGenerator, g_userGen,
				imageGenerator);
  CHECK_RC(nRetVal, "Config generators");

  nRetVal = context.StartGeneratingAll();
  CHECK_RC(nRetVal, "Generate all");

  // Create and initialize the cyclic buffer
  CyclicBuffer cyclicBuffer(context, depthGenerator, imageGenerator, config);
  cyclicBuffer.Initialize(config.strDirName, config.nDumpTime);

  GLFWwindow* win;

  if (!glfwInit())
    return -1;

  win = glfwCreateWindow(config.pDepthMode->nXRes, config.pDepthMode->nYRes,
			 "ni_recorder", NULL, NULL);
  if (!win){
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(win);
  glfwSetFramebufferSizeCallback(win, resizeCB);

  // Mainloop
  for (;;){
    if (xnOSWasKeyboardHit()){
      char c = xnOSReadCharFromInput();
      XnBool bStop = FALSE;
      switch (c){
      case 27:
	bStop = TRUE;
	break;
      case 'd':
	cyclicBuffer.Dump();
	bStop = TRUE; // can only dump once
	break;
      }
      if (bStop){
	break;
      }
    }
    // Get next data
    context.WaitAndUpdateAll();

    glClear(GL_COLOR_BUFFER_BIT);

    // Save data and draw scene
    cyclicBuffer.Update(depthGenerator, imageGenerator);

    glfwSwapBuffers(win);
    glfwPollEvents();

    // Check for missed frames
    if (config.bRecordDepth){
      ++nDepthFrames;
      XnUInt64 nTimestamp = depthGenerator.GetTimestamp();
      if (nLastDepthTime != 0 && nTimestamp - nLastDepthTime > 35000){
	int missed = (int)(nTimestamp-nLastDepthTime)/32000 - 1;
	printf("Missed depth: %llu -> %llu = %d > 35000 - %d frames\n",
	       nLastDepthTime, nTimestamp, XnUInt32(nTimestamp-nLastDepthTime), missed);
	nMissedDepthFrames += missed;
      }
      nLastDepthTime = nTimestamp;
    }
    if (config.bRecordImage){
      ++nImageFrames;
      XnUInt64 nTimestamp = imageGenerator.GetTimestamp();
      if (nLastImageTime != 0 && nTimestamp - nLastImageTime > 35000){
	int missed = (int)(nTimestamp-nLastImageTime)/32000 - 1;
	printf("Missed image: %llu -> %llu = %d > 35000 - %d frames\n",
	       nLastImageTime, nTimestamp, XnUInt32(nTimestamp-nLastImageTime), missed);
	nMissedImageFrames += missed;
      }
      nLastImageTime = nTimestamp;
    }
  }

  if (config.bRecordDepth){
    printf("Missed %d of %d depth frames (%5.2f%%)\n", nMissedDepthFrames, (nMissedDepthFrames+nDepthFrames), (nMissedDepthFrames*100.0)/(nMissedDepthFrames+nDepthFrames));
  }
  if (config.bRecordImage){
    printf("Missed %d of %d image frames (%5.2f%%)\n", nMissedImageFrames, (nMissedImageFrames+nImageFrames), (nMissedImageFrames*100.0)/(nMissedImageFrames+nImageFrames));
  }

  imageGenerator.Release();
  g_userGen.Release();
  depthGenerator.Release();
  context.Release();

  glfwDestroyWindow(win);
  glfwTerminate();

  return 0;
}
