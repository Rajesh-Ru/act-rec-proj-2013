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
****************************************************************************/
//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnOpenNI.h>
#include <XnLog.h>
#include <XnCppWrapper.h>
#include <XnPropNames.h>
#include <GLFW/glfw3.h>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "scene_drawer/SceneDrawer.hpp"

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
#define SAMPLE_XML_PATH "../../Config/SamplesConfig.xml"
#define MAX_NUM_USERS 1
#define DESCRIPTOR_SIZE 64

//---------------------------------------------------------------------------
// Macros
//---------------------------------------------------------------------------
#define CHECK_RC(rc, what)						\
  if (rc != XN_STATUS_OK){						\
    printf("%s failed: %s\n", what, xnGetStatusString(rc));		\
    return rc;								\
  }

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------

using namespace xn;

void transformDepthMD(DepthMetaData& depthMD)
{
  DepthMap& depthMap = depthMD.WritableDepthMap();
  for (XnUInt32 y = 0; y < depthMap.YRes(); y++){
    for (XnUInt32 x = 0; x < depthMap.XRes(); x++){
      //Punch vertical cut lines in the depth image
      if ((x % 2) == 0){
	depthMap(x, y) = 0;
      }
    }
  }
}

DepthGenerator depth;
UserGenerator g_userGen;
rad::SceneDrawer g_sceneDrawer;

XnBool g_bNeedPose = FALSE;
XnChar g_strPose[20] = "";

bool g_bGetNextFrame = true;
rad::SensorMsg msg;

cv::Mat g_descriptors; // each row is a descriptor
cv::Mat g_jointOffsets; // relative to the neck joint

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

void resizeCB(GLFWwindow* win, int w, int h)
{
  glViewport(0, 0, w, h);
}

bool g_bLeftMBPressed = false;
bool g_bJointSelected = false;
bool g_bRedisplay = false;
int preWinX, preWinY;

void mouseButtonCB(GLFWwindow* win, int button, int action, int mods)
{
  if (g_bGetNextFrame)
    return; // do nothing when getting frames

  if (button != GLFW_MOUSE_BUTTON_LEFT)
    return;

  if (action == GLFW_PRESS){
    preWinX = preWinY = -1;
    g_bLeftMBPressed = true;
  }
  else{
    g_bLeftMBPressed = false;
    g_bJointSelected = false;
  }
}

void cursorPosCB(GLFWwindow* win, double x, double y)
{
  if (g_bGetNextFrame || !g_bLeftMBPressed)
    return;

  //  int viewport[4];

  //  glGetIntegerv(GL_VIEWPORT, viewport);

  int winx = (int)floor(x), winy = (int)floor(y);
  // allow at most one user
  std::vector<rad::JointInfo> &joints = msg.pSkels[0].joints;

  static int selectedJoint;

  if (!g_bJointSelected){
    for (int i = 0; i < joints.size(); ++i){
      XnPoint3D worldPt = joints[i].pose.position.position;
      XnPoint3D winPt;

      depth.ConvertRealWorldToProjective(1, &worldPt, &winPt);

      if (abs(winx-winPt.X) < 5 && abs(winy - winPt.Y) < 5){
	// this joint is selected
	selectedJoint = i;
	g_bJointSelected = true;
	break;
      }
    }
  }

  if (g_bJointSelected && (winx != preWinX || winy != preWinY)){
    XnPoint3D pt;

    pt.X = winx;
    pt.Y = winy;
    pt.Z = (*msg.pDepthMD)(winx, winy);
    depth.ConvertProjectiveToRealWorld(1, &pt, &pt);

    joints[selectedJoint].pose.position.position = pt;
    joints[selectedJoint].pose.position.fConfidence = 1.0f;

    preWinX = winx;
    preWinY = winy;
    g_bRedisplay = true;
  }
}

void processCurFrame(bool saveResult = true)
{
  xn::DepthMetaData &depthMD = *msg.pDepthMD;
  xn::SceneMetaData &sceneMD = *msg.pSceneMD;

  const XnDepthPixel* pDPixels = depthMD.Data();
  const XnLabel* pLabels = sceneMD.Data();
  int xRes = depthMD.XRes(), yRes = depthMD.YRes(), zRes = depthMD.ZRes();
  int minx = xRes, miny = yRes, maxx = 0, maxy = 0, minz = zRes;

  // allow only one user: User 1
  for (int y = 0; y < yRes; ++y){
    for (int x = 0; x < xRes; ++x, ++pDPixels, ++pLabels){
      if (*pLabels != 1)
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
      if (*pLabels != 1 || *pDPixels > minz+50)
	userPixels.at<float>(y-miny, x-minx) = 0.0f;
      else
	userPixels.at<float>(y-miny, x-minx) = zRes-(float)(*pDPixels-minz);
    }
    pDPixels += xRes - maxx + minx;
    pLabels += xRes - maxx + minx;
  }

  cv::Mat test;
  userPixels.convertTo(test, CV_8UC1);
  cv::namedWindow("test", CV_WINDOW_AUTOSIZE);
  cv::imshow("test", test);
  cv::waitKey(100);
  cv::destroyWindow("test");

  if (!saveResult)
    return;

  cv::Mat scaledUP(DESCRIPTOR_SIZE, DESCRIPTOR_SIZE, CV_32FC1);

  cv::resize(userPixels, scaledUP, scaledUP.size());
  scaledUP = scaledUP.reshape(0, 1); // reshape to a row vector

  g_descriptors.push_back(scaledUP);

  // allow only one user
  std::vector<rad::JointInfo> &joints = msg.pSkels[0].joints;
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
  cv::Mat relJoints(1, 42, CV_32FC1);

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

  g_jointOffsets.push_back(relJoints);
}

void keyboardCB(GLFWwindow* win, int key, int scancode, int action, int mods)
{
  switch (key){
  case GLFW_KEY_ESCAPE:
    if (action == GLFW_PRESS)
      exit(EXIT_SUCCESS);
    break;
  case GLFW_KEY_N:
    if (action == GLFW_PRESS){
      processCurFrame();
      g_bGetNextFrame = true;
    }
    break;
  case GLFW_KEY_S:
    if (action == GLFW_PRESS)
      g_bGetNextFrame = true; // skip the current frame
    break;
  case GLFW_KEY_P:
    if (action == GLFW_PRESS)
      processCurFrame(false);
    break;
  default:
    // do nothing
    break;
  }
}

void getSkeleton(rad::SkeletonInfo &skel)
{
  const xn::SkeletonCapability &sc = g_userGen.GetSkeletonCap();

  XnSkeletonJoint activeJoints[30];
  XnUInt16 nJoints = 30;

  sc.EnumerateActiveJoints(activeJoints, nJoints);

  std::vector<rad::JointInfo> &joints = skel.joints;
  joints.resize(nJoints);

  XnPoint3D torsoPos;

  for (int i = 0; i < joints.size(); ++i){
    joints[i].type = activeJoints[i];
    sc.GetSkeletonJoint(skel.user, joints[i].type, joints[i].pose);

    // based on the believe that the torso joint is always confident
    if (joints[i].type == XN_SKEL_TORSO)
      torsoPos = joints[i].pose.position.position;
  }

  for (int i = 0; i < joints.size(); ++i){
    if (joints[i].pose.position.fConfidence < 0.5f){
      joints[i].pose.position.position = torsoPos;
      joints[i].pose.position.position.X += 50.0f;
      joints[i].pose.position.fConfidence = 0.5f;
    }
  }
}

int main(int argc, char* argv[])
{
  XnStatus nRetVal = XN_STATUS_OK;
  nRetVal = xnLogInitFromXmlFile(SAMPLE_XML_PATH);
  if (nRetVal != XN_STATUS_OK){
    printf("Log couldn't be opened: %s. Running without log\n",
	   xnGetStatusString(nRetVal));
  }

  if (argc < 3){
    printf("usage: %s <inputFile> <outputFile>\n", argv[0]);
    return -1;
  }

  const char* strInputFile = argv[1];
  const char* strOutputFile = argv[2];

  Context context;
  nRetVal = context.Init();
  CHECK_RC(nRetVal, "Init");

  // open input file
  Player player;
  nRetVal = context.OpenFileRecording(strInputFile, player);
  CHECK_RC(nRetVal, "Open input file");

  // Get depth node from recording
  nRetVal = context.FindExistingNode(XN_NODE_TYPE_DEPTH, depth);
  CHECK_RC(nRetVal, "Find depth generator");

  // Create a user generator
  nRetVal = context.FindExistingNode(XN_NODE_TYPE_USER, g_userGen);

  if (nRetVal != XN_STATUS_OK){
    nRetVal = g_userGen.Create(context);
    CHECK_RC(nRetVal, "Create user generator");
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
    CHECK_RC(nRetVal, "Register to pose detected");
    g_userGen.GetSkeletonCap().GetCalibrationPose(g_strPose);
  }

  g_userGen.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);

  nRetVal = player.SetRepeat(FALSE);
  XN_IS_STATUS_OK(nRetVal);

  XnUInt32 nNumFrames = 0;

  nRetVal = player.GetNumFrames(depth.GetName(), nNumFrames);
  CHECK_RC(nRetVal, "Get player number of frames");

  GLFWwindow* win;

  if (!glfwInit())
    return -1;

  XnMapOutputMode depthMode;

  depth.GetMapOutputMode(depthMode);
  win = glfwCreateWindow(depthMode.nXRes, depthMode.nYRes, "skel_editor",
			 NULL, NULL);
  if (!win){
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(win);
  glfwSetFramebufferSizeCallback(win, resizeCB);
  glfwSetCursorPosCallback(win, cursorPosCB);
  glfwSetMouseButtonCallback(win, mouseButtonCB);
  glfwSetKeyCallback(win, keyboardCB);

  DepthMetaData depthMD;
  SceneMetaData sceneMD;
  XnUserID aUsers[MAX_NUM_USERS];
  XnUInt16 nUsers;
  rad::SkeletonInfo skels[MAX_NUM_USERS];

  nRetVal = context.StartGeneratingAll();
  CHECK_RC(nRetVal, "Start generating all");

  while (true){
    // handle user input for the current frame
    while (!g_bGetNextFrame){
      if (g_bRedisplay){
	glClear(GL_COLOR_BUFFER_BIT);
	g_sceneDrawer.notify(rad::RAD_SENSOR_MSG, &msg);
	g_bRedisplay = false;
	glfwSwapBuffers(win);
      }
      glfwPollEvents();
    }

    if((nRetVal = context.WaitAndUpdateAll()) == XN_STATUS_EOF)
      break;

    CHECK_RC(nRetVal, "Read next frame");
    g_bGetNextFrame = false;

    depth.GetMetaData(depthMD);
    g_userGen.GetUserPixels(0, sceneMD);

    nUsers = MAX_NUM_USERS;
    g_userGen.GetUsers(aUsers, nUsers);
    memset(&msg, 0, sizeof(rad::SensorMsg));

    for (int i = 0; i < nUsers; ++i){
      if (!g_userGen.GetSkeletonCap().IsTracking(aUsers[i])){
	skels[i].user = 0;
	g_bGetNextFrame = true;
	continue;
      }

      skels[i].user = aUsers[i];
      getSkeleton(skels[i]);
    }

    if (nUsers == 0)
      g_bGetNextFrame = true;

    msg.nUsers = nUsers;
    msg.pSkels = skels;
    msg.pDepthMD = &depthMD;
    msg.pSceneMD = &sceneMD;

    glClear(GL_COLOR_BUFFER_BIT);

    g_sceneDrawer.notify(rad::RAD_SENSOR_MSG, &msg);
    //    nRetVal = depthMD.MakeDataWritable();
    //    CHECK_RC(nRetVal, "Make depth data writable");

    //    transformDepthMD(depthMD);

    glfwSwapBuffers(win);
    glfwPollEvents();
  }

  // dump g_descriptors and g_jointOffsets into a file
  cv::FileStorage fs(strOutputFile, cv::FileStorage::WRITE);
  fs << "descriptors" << g_descriptors
     << "joint_offsets" << g_jointOffsets;
  fs.release();

  g_userGen.Release();
  depth.Release();
  context.Release();

  glfwDestroyWindow(win);
  glfwTerminate();

  return 0;
}
