/**
 * Author: Rajesh
 * Description: an implementation of SceneDrawer.hpp.
 */

#define __USE_BSD
#include <math.h>

#include "SceneDrawer.hpp"

using namespace rad;

const XnFloat colors[][3] = { {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f},
			      {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f},
			      {1.0f, 0.0f, 0.0f}, {1.0f, 0.5f, 0.0f},
			      {0.5f, 1.0f, 0.0f}, {0.0f, 0.5f, 1.0f},
			      {0.5f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.5f},
			      {1.0f, 1.0f, 1.0f} };
const XnUInt32 nColors = 10;

extern xn::DepthGenerator depth;

SceneDrawer::SceneDrawer()
{
  initialized = false;
  pDepthTexBuf = NULL;
  pDepthHist = NULL;
}

SceneDrawer::~SceneDrawer()
{
  delete pDepthTexBuf;
  delete pDepthHist;
}

void SceneDrawer::notify(RADenum msgType, const void* msg)
{
  if (msgType != RAD_SENSOR_MSG)
    return;

  const SensorMsg* pMsg = (const SensorMsg*)msg;

  updateScene(*pMsg->pDepthMD, *pMsg->pSceneMD);
  drawScene(pMsg->pSkels, pMsg->nUsers);
}

void SceneDrawer::getSceneDim(int* sceneWidth, int* sceneHeight)
{
  if (!initialized)
    return;

  *sceneWidth = texWidth;
  *sceneHeight = texHeight;
}

void SceneDrawer::updateScene(const xn::DepthMetaData &dmd,
			      const xn::SceneMetaData &smd)
{
  if (!initialized)
    init(dmd.XRes(), dmd.YRes(), dmd.ZRes());

  unsigned int val = 0, numOfPoints = 0;
  unsigned int xRes = dmd.XRes(), yRes = dmd.YRes();
  unsigned char* pDestImage = pDepthTexBuf;
  const XnDepthPixel* pDepth = dmd.Data();
  const XnLabel* pLabels = smd.Data();

  // calculate the accumulative depth histogram
  memset(pDepthHist, 0, zRes*sizeof(float));

  int numOfIterations = xRes * yRes;

  for (int i = 0; i < numOfIterations; ++i, ++pDepth){
    val = *pDepth;

    if (val != 0){
      pDepthHist[val]++;
      numOfPoints++;
    }
  }

  for (int i = 1; i < zRes; ++i)
    pDepthHist[i] += pDepthHist[i-1];

  if (numOfPoints > 0){
    for (int i = 0; i < zRes; ++i)
      pDepthHist[i] = floor(256.0f*(1.0f-pDepthHist[i]/(float)numOfPoints));
  }

  // turn depth map to a colored texture image
  pDepth = dmd.Data();

  XnUInt32 ci;
  XnLabel label;
  unsigned int histVal;

  for (int i = 0; i < numOfIterations;
       ++i, ++pDepth, ++pLabels, pDestImage += 3){
    val = *pDepth;
    label = *pLabels;

    if (label != 0)
      ci = label % nColors;
    else
      ci = nColors;

    if (val != 0){
      histVal = pDepthHist[val];

      pDestImage[0] = histVal * colors[ci][0];
      pDestImage[1] = histVal * colors[ci][1];
      pDestImage[2] = histVal * colors[ci][2];
    }
    else
      pDestImage[0] = pDestImage[1] = pDestImage[2] = 0;
  }

  glBindTexture(GL_TEXTURE_2D, depthTexID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB,
	       GL_UNSIGNED_BYTE, pDepthTexBuf);
}

void SceneDrawer::drawScene(const SkeletonInfo* pSkels, XnUInt16 nUsers)
{
  if (!initialized)
    return;

  drawTexture();

  for (int i = 0; i < nUsers; ++i)
    if (pSkels[i].user != 0)
      drawSkel(&pSkels[i]);
}

// draw the skeleton of a single user
void SceneDrawer::drawSkel(const SkeletonInfo* pSkel)
{
  const std::vector<JointInfo> &joints = pSkel->joints;
  XnPoint3D pts[30]; // at most 24 joints
  bool isConfident[30] = {false};
  xn::DepthGenerator* pDepthGen = &depth;

  int ci = pSkel->user % nColors;
  glColor3f(1.0f-colors[ci][0], 1.0f-colors[ci][1], 1.0f-colors[ci][2]);

  for (int i = 0; i < joints.size(); ++i){
    if (joints[i].pose.position.fConfidence >= 0.5){
      int id = joints[i].type;

      isConfident[id] = true;
      pts[id] = joints[i].pose.position.position;
      pDepthGen->ConvertRealWorldToProjective(1, &pts[id], &pts[id]);
      drawCircle(pts[id].X, pts[id].Y, 2);
    }
  }
  
  glBegin(GL_LINES);

  if (isConfident[XN_SKEL_HEAD] && isConfident[XN_SKEL_NECK])
    drawLine2D(pts[XN_SKEL_HEAD].X, pts[XN_SKEL_HEAD].Y,
	       pts[XN_SKEL_NECK].X, pts[XN_SKEL_NECK].Y);

  if (isConfident[XN_SKEL_NECK] && isConfident[XN_SKEL_LEFT_SHOULDER])
    drawLine2D(pts[XN_SKEL_NECK].X, pts[XN_SKEL_NECK].Y,
	       pts[XN_SKEL_LEFT_SHOULDER].X, pts[XN_SKEL_LEFT_SHOULDER].Y);
  if (isConfident[XN_SKEL_LEFT_SHOULDER] && isConfident[XN_SKEL_LEFT_ELBOW])
    drawLine2D(pts[XN_SKEL_LEFT_SHOULDER].X, pts[XN_SKEL_LEFT_SHOULDER].Y,
	       pts[XN_SKEL_LEFT_ELBOW].X, pts[XN_SKEL_LEFT_ELBOW].Y);
  if (isConfident[XN_SKEL_LEFT_ELBOW] && isConfident[XN_SKEL_LEFT_WRIST]){
    drawLine2D(pts[XN_SKEL_LEFT_ELBOW].X, pts[XN_SKEL_LEFT_ELBOW].Y,
	       pts[XN_SKEL_LEFT_WRIST].X, pts[XN_SKEL_LEFT_WRIST].Y);
    if (isConfident[XN_SKEL_LEFT_HAND]){
      drawLine2D(pts[XN_SKEL_LEFT_WRIST].X, pts[XN_SKEL_LEFT_WRIST].Y,
		 pts[XN_SKEL_LEFT_HAND].X, pts[XN_SKEL_LEFT_HAND].Y);
      if (isConfident[XN_SKEL_LEFT_FINGERTIP])
	drawLine2D(pts[XN_SKEL_LEFT_HAND].X, pts[XN_SKEL_LEFT_HAND].Y,
		   pts[XN_SKEL_LEFT_FINGERTIP].X, pts[XN_SKEL_LEFT_FINGERTIP].Y);
    }
  }
  else{
    if (isConfident[XN_SKEL_LEFT_ELBOW] && isConfident[XN_SKEL_LEFT_HAND])
      drawLine2D(pts[XN_SKEL_LEFT_ELBOW].X, pts[XN_SKEL_LEFT_ELBOW].Y,
		 pts[XN_SKEL_LEFT_HAND].X, pts[XN_SKEL_LEFT_HAND].Y);
  }

  if (isConfident[XN_SKEL_NECK] && isConfident[XN_SKEL_RIGHT_SHOULDER])
    drawLine2D(pts[XN_SKEL_NECK].X, pts[XN_SKEL_NECK].Y,
	       pts[XN_SKEL_RIGHT_SHOULDER].X, pts[XN_SKEL_RIGHT_SHOULDER].Y);
  if (isConfident[XN_SKEL_RIGHT_SHOULDER] && isConfident[XN_SKEL_RIGHT_ELBOW])
    drawLine2D(pts[XN_SKEL_RIGHT_SHOULDER].X, pts[XN_SKEL_RIGHT_SHOULDER].Y,
	       pts[XN_SKEL_RIGHT_ELBOW].X, pts[XN_SKEL_RIGHT_ELBOW].Y);
  if (isConfident[XN_SKEL_RIGHT_ELBOW] && isConfident[XN_SKEL_RIGHT_WRIST]){
    drawLine2D(pts[XN_SKEL_RIGHT_ELBOW].X, pts[XN_SKEL_RIGHT_ELBOW].Y,
	       pts[XN_SKEL_RIGHT_WRIST].X, pts[XN_SKEL_RIGHT_WRIST].Y);
    if (isConfident[XN_SKEL_RIGHT_HAND]){
      drawLine2D(pts[XN_SKEL_RIGHT_WRIST].X, pts[XN_SKEL_RIGHT_WRIST].Y,
		 pts[XN_SKEL_RIGHT_HAND].X, pts[XN_SKEL_RIGHT_HAND].Y);
      if (isConfident[XN_SKEL_RIGHT_FINGERTIP])
	drawLine2D(pts[XN_SKEL_RIGHT_HAND].X, pts[XN_SKEL_RIGHT_HAND].Y,
		   pts[XN_SKEL_RIGHT_FINGERTIP].X,
		   pts[XN_SKEL_RIGHT_FINGERTIP].Y);
    }
  }
  else{
    if (isConfident[XN_SKEL_RIGHT_ELBOW] && isConfident[XN_SKEL_RIGHT_HAND])
      drawLine2D(pts[XN_SKEL_RIGHT_ELBOW].X, pts[XN_SKEL_RIGHT_ELBOW].Y,
		 pts[XN_SKEL_RIGHT_HAND].X, pts[XN_SKEL_RIGHT_HAND].Y);
  }

  if (isConfident[XN_SKEL_LEFT_SHOULDER] && isConfident[XN_SKEL_TORSO])
    drawLine2D(pts[XN_SKEL_LEFT_SHOULDER].X, pts[XN_SKEL_LEFT_SHOULDER].Y,
	       pts[XN_SKEL_TORSO].X, pts[XN_SKEL_TORSO].Y);
  if (isConfident[XN_SKEL_RIGHT_SHOULDER] && isConfident[XN_SKEL_TORSO])
    drawLine2D(pts[XN_SKEL_RIGHT_SHOULDER].X, pts[XN_SKEL_RIGHT_SHOULDER].Y,
	       pts[XN_SKEL_TORSO].X, pts[XN_SKEL_TORSO].Y);

  if (isConfident[XN_SKEL_TORSO] && isConfident[XN_SKEL_LEFT_HIP])
    drawLine2D(pts[XN_SKEL_TORSO].X, pts[XN_SKEL_TORSO].Y,
	       pts[XN_SKEL_LEFT_HIP].X, pts[XN_SKEL_LEFT_HIP].Y);
  if (isConfident[XN_SKEL_LEFT_HIP] && isConfident[XN_SKEL_LEFT_KNEE])
    drawLine2D(pts[XN_SKEL_LEFT_HIP].X, pts[XN_SKEL_LEFT_HIP].Y,
	       pts[XN_SKEL_LEFT_KNEE].X, pts[XN_SKEL_LEFT_KNEE].Y);
  if (isConfident[XN_SKEL_LEFT_KNEE] && isConfident[XN_SKEL_LEFT_FOOT])
    drawLine2D(pts[XN_SKEL_LEFT_KNEE].X, pts[XN_SKEL_LEFT_KNEE].Y,
	       pts[XN_SKEL_LEFT_FOOT].X, pts[XN_SKEL_LEFT_FOOT].Y);

  if (isConfident[XN_SKEL_TORSO] && isConfident[XN_SKEL_RIGHT_HIP])
    drawLine2D(pts[XN_SKEL_TORSO].X, pts[XN_SKEL_TORSO].Y,
	       pts[XN_SKEL_RIGHT_HIP].X, pts[XN_SKEL_RIGHT_HIP].Y);
  if (isConfident[XN_SKEL_RIGHT_HIP] && isConfident[XN_SKEL_RIGHT_KNEE])
    drawLine2D(pts[XN_SKEL_RIGHT_HIP].X, pts[XN_SKEL_RIGHT_HIP].Y,
	       pts[XN_SKEL_RIGHT_KNEE].X, pts[XN_SKEL_RIGHT_KNEE].Y);
  if (isConfident[XN_SKEL_RIGHT_KNEE] && isConfident[XN_SKEL_RIGHT_FOOT])
    drawLine2D(pts[XN_SKEL_RIGHT_KNEE].X, pts[XN_SKEL_RIGHT_KNEE].Y,
	       pts[XN_SKEL_RIGHT_FOOT].X, pts[XN_SKEL_RIGHT_FOOT].Y);

  if (isConfident[XN_SKEL_LEFT_HIP] && isConfident[XN_SKEL_RIGHT_HIP])
    drawLine2D(pts[XN_SKEL_LEFT_HIP].X, pts[XN_SKEL_LEFT_HIP].Y,
	       pts[XN_SKEL_RIGHT_HIP].X, pts[XN_SKEL_RIGHT_HIP].Y);

  glEnd();
}

inline void SceneDrawer::drawLine2D(float x1, float y1, float x2, float y2)
{
  glVertex2f(x1, y1);
  glVertex2f(x2, y2);
}

inline void SceneDrawer::drawTexture()
{
  glEnable(GL_TEXTURE_2D);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
  glVertexPointer(2, GL_FLOAT, 0, verts);

  glColor3f(.75f, .75f, .75f);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisable(GL_TEXTURE_2D);
}

inline void SceneDrawer::drawCircle(float x, float y, float r)
{
  glBegin(GL_TRIANGLE_FAN);

  float theta;

  for (int i = 0; i < 360; i += 20){
    theta = (float)i / 180.0f * M_PI;
    glVertex2f(x + cos(theta)*r, y + sin(theta)*r);
  }
  glEnd();
}

void SceneDrawer::init(unsigned int xRes, unsigned int yRes, unsigned int zRes)
{
  texWidth = xRes;
  texHeight = yRes;
  this->zRes = zRes;

  glGenTextures(1, &depthTexID);
  pDepthTexBuf = new unsigned char[texWidth*texHeight*3];
  glBindTexture(GL_TEXTURE_2D, depthTexID);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  memset(texCoords, 0, 8*sizeof(float));

  texCoords[0] = texCoords[2] = (float)xRes/(float)texWidth;
  texCoords[1] = texCoords[7] = (float)yRes/(float)texHeight;

  memset(verts, 0, 8*sizeof(float));

  verts[0] = verts[2] = (float)texWidth;
  verts[1] = verts[7] = (float)texHeight;

  pDepthHist = new float[zRes];

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, texWidth, texHeight, 0.0, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  initialized = true;
}
