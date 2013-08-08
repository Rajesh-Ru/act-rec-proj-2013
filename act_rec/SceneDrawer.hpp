/**
 * Author: Rajesh
 * Description: draw depth map and users' skeletons.
 */

#include <GLFW/glfw3.h>

#include "SensorDataSub.hpp"

#ifndef SCENEDRAWER_HPP
#define SCENEDRAWER_HPP

namespace rad
{
class SceneDrawer : public SensorDataSub
{
public:
  SceneDrawer();
  ~SceneDrawer();
  void notify(RADenum msgType, const void* msg);
  void getSceneDim(int* sceneWidth, int* sceneHeight);

private:
  void updateScene(const xn::DepthMetaData &dmd, const xn::SceneMetaData &smd);
  void drawScene(const SkeletonInfo* pSkels, XnUInt16 nUsers);
  void drawSkel(const SkeletonInfo* pSkel);
  void drawLine2D(float x1, float y1, float x2, float y2);
  void drawTexture();
  void drawCircle(float x, float y, float r);
  void init(unsigned int xRes, unsigned int yRes, unsigned int zRes);

  bool initialized;
  GLuint depthTexID;
  unsigned char* pDepthTexBuf;
  unsigned int texWidth, texHeight, zRes;
  GLfloat verts[8];
  GLfloat texCoords[8];
  float* pDepthHist;
};
}

#endif // SCENEDRAWER_HPP
