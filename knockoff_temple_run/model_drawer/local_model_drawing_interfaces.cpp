/**
 * Author: Rajesh
 * Decription: defines the interfaces necessary for draw local models.
 */

#include <stdio.h>

#include "../glm/glm.h"
#include "../common/common.h"
#include "Token.hpp"
#include "DemonShock.hpp"
#include "Tree.hpp"
#include "FireSerpent.hpp"

#ifndef LOCAL_MODEL_DRAWING_INTERFACES_HPP
#define LOCAL_MODEL_DRAWING_INTERFACES_HPP

#define NUM_OF_ROAD_TYPES 5
#define NUM_OF_OBJECT_TYPES 5

static GLMmodel* g_pRoadModels[NUM_OF_ROAD_TYPES];
static ao::AnimatedObject* g_pAnimatedObjects[NUM_OF_OBJECT_TYPES];
static bool g_bModelsLoaded = false;

void lmdi_loadModels()
{
  if (!g_bModelsLoaded){
    g_pRoadModels[0] = glmReadOBJ("obj/straight_road.obj");
    g_pRoadModels[1] = glmReadOBJ("obj/left_turning_road.obj");
    g_pRoadModels[2] = glmReadOBJ("obj/right_turning_road.obj");
    g_pRoadModels[3] = glmReadOBJ("obj/left_right_road.obj");
    g_pRoadModels[4] = glmReadOBJ("obj/start_road.obj");

    g_pAnimatedObjects[0] = new ao::Token("obj/token.obj");
    g_pAnimatedObjects[1] = new ao::DemonShock("obj/demon_arm.obj");
    g_pAnimatedObjects[2] = new ao::Tree("obj/tree.obj"); // tree_left
    g_pAnimatedObjects[3] = new ao::Tree("obj/tree_right.obj");
    g_pAnimatedObjects[4] = new ao::FireSerpent("obj/serpent.obj");

    g_bModelsLoaded = true;
  }
}

void drawObject(ObjectType type, float x, float y)
{
  glPushMatrix();

  switch (type){
  case TOKEN:
    g_pAnimatedObjects[0]->update();
    glTranslatef(x, y, 1.0f);
    g_pAnimatedObjects[0]->draw();
    break;
  case DEMON_SHOCK:
    g_pAnimatedObjects[1]->update();
    glTranslatef(0.0f, y, -1.0f);
    g_pAnimatedObjects[1]->draw();
    break;
  case TREE:
    if (x < 0.0f){
      glTranslatef(-2.5f, y, 1.5f);
      g_pAnimatedObjects[2]->draw();
    }
    else{
      glTranslatef(2.5f, y, 1.5f);
      g_pAnimatedObjects[3]->draw();
    }
    break;
  case FIRE_SERPENT:
    g_pAnimatedObjects[4]->update();
    if (x < 0.0f){
      glTranslatef(-1.0f, y, -0.4f);
      g_pAnimatedObjects[4]->draw();
    }
    else{
      glTranslatef(1.0f, y, -0.4f);
      glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
      g_pAnimatedObjects[4]->draw();
    }
    break;
  default:
    // do nothing
    break;
  }

  glPopMatrix();
}

void drawRoad(RoadType type)
{
  glPushMatrix();

  switch (type){
  case STRAIGHT:
    glTranslatef(0.0f, 0.0f, -1.0f);
    glmDraw(g_pRoadModels[0], GLM_SMOOTH | GLM_TEXTURE);
    break;
  case LEFT:
    glTranslatef(0.0f, 0.0f, -1.0f);
    glmDraw(g_pRoadModels[1], GLM_SMOOTH | GLM_TEXTURE);
    break;
  case RIGHT:
    glTranslatef(0.0f, 0.0f, -1.0f);
    glmDraw(g_pRoadModels[2], GLM_SMOOTH | GLM_TEXTURE);
    break;
  case LEFT_RIGHT:
    glTranslatef(0.0f, 0.0f, -1.0f);
    glmDraw(g_pRoadModels[3], GLM_SMOOTH | GLM_TEXTURE);
    break;
  case START:
    glTranslatef(0.0f, 0.0f, -1.0f);
    glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
    glmDraw(g_pRoadModels[4], GLM_SMOOTH | GLM_TEXTURE);
    break;
  default:
    // do nothing
    break;
  }

  glPopMatrix();
}

void lmdi_draw(RoadType roadType, Object *pObject)
{
  if (!g_bModelsLoaded)
    lmdi_loadModels();

  drawRoad(roadType);

  if (roadType == START)
    return;

  if (pObject != NULL)
    drawObject(pObject->type, pObject->x, pObject->y);
}

void lmdi_destroy()
{
  if (g_bModelsLoaded){
    for (int i = 0; i < NUM_OF_ROAD_TYPES; ++i)
      glmDelete(g_pRoadModels[i]);

    for (int i = 0; i < NUM_OF_OBJECT_TYPES; ++i)
      delete g_pAnimatedObjects[i];

    g_bModelsLoaded = false;
  }
}

#endif // LOCAL_MODEL_DRAWING_INTERFACES_HPP
