/**
 * Author: Rajesh
 * Description: draw map.
 */

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <map>
#include <list>
#include <GLFW/glfw3.h>

#include "../model_drawer/local_model_drawing_interfaces.h"

extern Road g_map[MAP_DIM][MAP_DIM];
extern std::map<int, Index2D> ori2dir;

void drawMap(float wx, float wy)
{
  std::cout << std::endl;

  for (int y = MAP_DIM - 1; y >= 0; --y){
    for (int x = 0; x < MAP_DIM; ++x){
      if (x == (int)mp((int)floor(wx / 3.0f)) % 51 &&
          y == (int)mp((int)floor(wy / 3.0f)) % 51){
	std::cout << 0;
        continue;
      }

      if(g_map[y][x].type != EMPTY)
	std::cout << g_map[y][x].type;
      else
	std::cout << ' ';
    }
    std::cout << std::endl;
  }
}

void md_init()
{
  lmdi_loadModels();
}

void md_destroy()
{
  lmdi_destroy();
}

typedef struct Entry
{
  Index2D position;
  float orientation;
  std::vector<float> mvMat;
} Entry;

void md_draw(float wx, float wy, bool drawBackward)
{
  int mx = WORLD_TO_MAP(wx + 1.5f), my = WORLD_TO_MAP(wy);
  Road* pRoad = &g_map[my][mx];
  std::vector<Entry> todo;
  Entry e;

  if (pRoad->type == EMPTY){
    printf("%f %f\n", wx + 1.5f, wy);
    printf("%d %d\n", mx, my);
    printf("%f\n", pRoad->orientation);
    drawMap(wx + 1.5f, wy);

    printf("The given player's position has no road!\n");
    exit(EXIT_FAILURE);
  }

  float theta = pRoad->orientation;

  glPushMatrix();
  glTranslatef(-wx+floor((wx+1.5f)/3.0f)*3.0f,
	       1.5f-wy+floor(wy/3.0f)*3.0f, 0.0f);

  while (pRoad->type != EMPTY || !todo.empty()){
    if (pRoad->type == EMPTY){
      e = todo.back();
      todo.pop_back();

      mx = e.position.x;
      my = e.position.y;
      pRoad = &g_map[my][mx];
      theta = e.orientation;

      float* mvMat = &e.mvMat[0];

      glLoadMatrixf(mvMat);
    }

    glPushMatrix();
    glRotatef(theta, 0.0f, 0.0f, 1.0f);
    lmdi_draw(pRoad->type, &pRoad->object);
    glPopMatrix();

    if (pRoad->type == LEFT || pRoad->type == LEFT_RIGHT)
      theta = (int)(theta + 90.0f) % 360;
    else if (pRoad->type == RIGHT)
      theta = (int)(theta + 270.0f) % 360;

    if (pRoad->type == LEFT_RIGHT){
      e.orientation = (int)(theta + 180.0f) % 360;

      Index2D idx = ori2dir[(int)e.orientation];
      float mvMat[16];

      glPushMatrix();
      glTranslatef(idx.x*3.0f, idx.y*3.0f, 0.0f);
      glGetFloatv(GL_MODELVIEW_MATRIX, mvMat);
      e.mvMat.assign(mvMat, mvMat+16);
      glPopMatrix();

      e.position.x = mp(mx + idx.x) % MAP_DIM;
      e.position.y = mp(my + idx.y) % MAP_DIM;
      todo.push_back(e);
    }

    Index2D idx = ori2dir[(int)theta];

    glTranslatef(idx.x*3.0f, idx.y*3.0f, 0.0f);

    mx = mp(mx + idx.x) % MAP_DIM;
    my = mp(my + idx.y) % MAP_DIM;
    pRoad = &g_map[my][mx];
  }

  glPopMatrix();

  if (!drawBackward)
    return;

  todo.clear();

  mx = WORLD_TO_MAP(wx + 1.5f);
  my = WORLD_TO_MAP(wy);
  theta = (int)(g_map[my][mx].orientation + 180.0f) % 360;
  Index2D idx = ori2dir[(int)theta];
  mx = mp(mx + idx.x) % MAP_DIM;
  my = mp(my + idx.y) % MAP_DIM;
  pRoad = &g_map[my][mx];

  glPushMatrix();
  glTranslatef(-wx+floor((wx+1.5f)/3.0f)*3.0f,
	       1.5f-wy+floor(wy/3.0f)*3.0f, 0.0f);

  while (pRoad->type != EMPTY){
    glTranslatef(idx.x*3.0f, idx.y*3.0f, 0.0f);

    glPushMatrix();
    glRotatef(pRoad->orientation, 0.0f, 0.0f, 1.0f);
    lmdi_draw(pRoad->type, &pRoad->object);
    glPopMatrix();

    if (pRoad->type == LEFT_RIGHT){
      e.orientation = theta;

      Index2D idx2 = ori2dir[(int)e.orientation];
      float mvMat[16];

      glPushMatrix();
      glTranslatef(idx2.x*3.0f, idx2.y*3.0f, 0.0f);
      glGetFloatv(GL_MODELVIEW_MATRIX, mvMat);
      e.mvMat.assign(mvMat, mvMat+16);
      glPopMatrix();
      e.position.x = mp(mx + idx.x) % MAP_DIM;
      e.position.y = mp(my + idx.y) % MAP_DIM;
      todo.push_back(e);
    }

    theta = (int)(pRoad->orientation + 180.0f) % 360;
    idx = ori2dir[(int)theta];
    mx = mp(mx + idx.x) % MAP_DIM;
    my = mp(my + idx.y) % MAP_DIM;
    pRoad = &g_map[my][mx];
  }

  while (pRoad->type != EMPTY || !todo.empty()){
    if (pRoad->type == EMPTY){
      e = todo.back();
      todo.pop_back();

      mx = e.position.x;
      my = e.position.y;
      pRoad = &g_map[my][mx];
      theta = e.orientation;

      float* mvMat = &e.mvMat[0];
      glLoadMatrixf(mvMat);
    }

    glPushMatrix();
    glRotatef(theta, 0.0f, 0.0f, 1.0f);
    lmdi_draw(pRoad->type, &pRoad->object);
    glPopMatrix();

    if (pRoad->type == LEFT || pRoad->type == LEFT_RIGHT)
      theta = (int)(theta + 90.0f) % 360;
    else if (pRoad->type == RIGHT)
      theta = (int)(theta + 270.0f) % 360;

    if (pRoad->type == LEFT_RIGHT){
      e.orientation = (int)(theta + 180.0f) % 360;
      idx = ori2dir[(int)e.orientation];

      float mvMat[16];

      glPushMatrix();
      glTranslatef(idx.x*3.0f, idx.y*3.0f, 0.0f);
      glGetFloatv(GL_MODELVIEW_MATRIX, mvMat);
      e.mvMat.assign(mvMat, mvMat+16);
      glPopMatrix();
      e.position.x = mp(mx + idx.x) % MAP_DIM;
      e.position.y = mp(my + idx.y) % MAP_DIM;
      todo.push_back(e);
    }

    idx = ori2dir[(int)theta];

    glTranslatef(idx.x*3.0f, idx.y*3.0f, 0.0f);

    mx = mp(mx + idx.x) % MAP_DIM;
    my = mp(my + idx.y) % MAP_DIM;
    pRoad = &g_map[my][mx];
  }

  glPopMatrix();
}
