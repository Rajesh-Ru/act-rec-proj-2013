/**
 * Author: Rajesh
 * Description: an implementation of RADFactory.hpp.
 */

#include "RADFactory.hpp"

#define OPENNI_CONFIG_XML "SamplesConfig.xml"

using namespace rad;

RADFactory* RADFactory::pInst = NULL;

RADFactory* RADFactory::getPtrToInst()
{
  if (pInst == NULL)
    pInst = new RADFactory();

  return pInst;
}

void RADFactory::releaseInst()
{
  if (pInst != NULL){
    delete pInst;
    pInst = NULL;
  }
}

RADFactory::~RADFactory()
{
  // release possibly created singletons
  KinectDataPub::releaseInst();
}

SensorDataPub* RADFactory::getPtrToPub(RADenum type)
{
  SensorDataPub* pPub = NULL;

  switch (type){
  case RAD_PUB_TYPE_KINECT_DATA:
    pPub = KinectDataPub::getPtrToInst(OPENNI_CONFIG_XML);
    break;
  default:
    // do nothing
    break;
  }

  return pPub;
}

SensorDataSub* RADFactory::getPtrToSub(RADenum type)
{
  SensorDataSub* pSub;

  switch (type){
  case RAD_SUB_TYPE_FIREBALL_ACT:
    pSub = new FireBallAction();
    break;
  case RAD_SUB_TYPE_SCENE_DRAWER:
    pSub = new SceneDrawer();
    break;
  default:
    // do nothing
    break;
  }

  return pSub;
}

void RADFactory::release(RADenum type, const void* pData)
{
  switch (type){
  case RAD_PUB_TYPE_KINECT_DATA:
    KinectDataPub::releaseInst();
    break;
  default:
    {
      const SensorDataSub* pSub = (const SensorDataSub*)pData;
      delete pSub;
    }
    break;
  }
}
