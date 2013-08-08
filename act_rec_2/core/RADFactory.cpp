/**
 * Author: Rajesh
 * Description: an implementation of RADFactory.hpp.
 */

#include <stdio.h>
#include <stdlib.h>
#include <map>

#include "RADFactory.hpp"

#define OPENNI_CONFIG_XML "SamplesConfig.xml"
#define TRAIN_DATA_FILE "../data_set/train.txt"
#define VIDEO_RECORD "video_to_use.txt"

using namespace rad;

RADFactory* RADFactory::pInst = NULL;

std::map<RADenum, BiClassifier*> bcs;

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
  case RAD_PUB_TYPE_VIDEO_DATA:
    {
      static char vfn[256];
      static bool onceThrough = false;

      if (!onceThrough){
	FILE* fh = fopen(VIDEO_RECORD, "r");

	if (fh == NULL){
	  printf("Could not open file %s\n", VIDEO_RECORD);
	  exit(EXIT_FAILURE);
	}

	fscanf(fh, "%s", vfn);
	fclose(fh);
	onceThrough = true;
      }
      pPub = VideoDataPub::getPtrToInst(vfn);
    }
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

BiClassifier* RADFactory::getPtrToBiClassifier(RADenum type)
{
  switch (type){
  case RAD_BC_TYPE_FIREBALL_START:
    if (bcs.find(type) != bcs.end())
      return bcs[type];
    else{
      bcs[type] = new BiClassifier(TRAIN_DATA_FILE);
      return bcs[type];
    }
    break;
  default:
    // do nothing
    break;
  }
}

void RADFactory::release(RADenum type, const void* pData)
{
  switch (type){
  case RAD_PUB_TYPE_KINECT_DATA:
    KinectDataPub::releaseInst();
    break;
  case RAD_PUB_TYPE_VIDEO_DATA:
    VideoDataPub::releaseInst();
    break;
  case RAD_BC:
    for (std::map<RADenum, BiClassifier*>::iterator it = bcs.begin();
	 it != bcs.end(); ++it){
      delete it->second;
    }
    break;
  default:
    {
      const SensorDataSub* pSub = (const SensorDataSub*)pData;
      delete pSub;
    }
    break;
  }
}
