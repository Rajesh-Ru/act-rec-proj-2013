/**
 * Author: Rajesh
 * Description: manages underlying logical representation of the map.
 */

#include <stdlib.h>
#include <math.h>
#include <vector>
#include <list>
#include <map>
#include <string.h>
#include <stdio.h>

#include "../common/common.h"

Road g_map[MAP_DIM][MAP_DIM];
std::list<Index2D> g_endpoints;
int g_iStartX, g_iStartY;
std::map<int, Index2D> ori2dir;
bool g_bIsInitialized = false;
std::vector<Index2D> g_anticipatedNextRoads;

int mp(int x)
{
  while (x < 0)
    x += MAP_DIM;
  return x;
}

Object randObject()
{
  Object obj;
  float r = (float)(rand() % 1000) / 1000.0f;

  if (r < 0.5)
    obj = {NOTHING, 0.0f, 0.0f};
  else if (r < 0.65)
    obj = {TOKEN, 0.0f, 0.0f};
  else if (r < 0.75)
    obj = {DEMON_SHOCK, 0.0f, 0.0f};
  else if (r < 0.85)
    obj = {TREE, (float)(rand()%2000)/1000.0f-1.0f, 0.0f};
  else if (r < 0.9)
    obj = {FIRE_SERPENT, (float)(rand()%2000)/1000.0f-1.0f, 0.0f};
  else
    obj = {PITFALL, 0.0f, 0.0f};

  return obj;
}

bool addOneRoadPattern(int mx, int my, RoadType type, float theta)
{
  float orientation;
  Index2D idx, idx2;

  if (type == STRAIGHT)
    orientation = theta;
  else if (type == RIGHT)
    orientation = (int)(theta + 270.0f) % 360;
  else if (type == LEFT || type == LEFT_RIGHT)
    orientation = (int)(theta + 90.0f) % 360;
  else{
    printf("ERROR: Invalid road type.\n");
    exit(EXIT_FAILURE);
  }

  idx = ori2dir[(int)orientation % 360];
  idx2 = ori2dir[(int)(orientation + 180.0f) % 360];

  int p_mx = mp(mx + 3 * idx.x) % MAP_DIM,
    p_my = mp(my + 3 * idx.y) % MAP_DIM,
    p_mx2 = mp(mx + 3 * idx2.x) % MAP_DIM,
    p_my2 = mp(my + 3 * idx2.y) % MAP_DIM;

  if (g_map[(p_my + 1) % MAP_DIM][p_mx].type != EMPTY ||
      g_map[mp(p_my - 1) % MAP_DIM][p_mx].type != EMPTY ||
      g_map[p_my][(p_mx + 1) % MAP_DIM].type != EMPTY ||
      g_map[p_my][mp(p_mx - 1) % MAP_DIM].type != EMPTY)
    return false;
  if (type == LEFT_RIGHT &&
      (g_map[(p_my2 + 1) % MAP_DIM][p_mx2].type != EMPTY ||
       g_map[mp(p_my2 - 1) % MAP_DIM][p_mx2].type != EMPTY ||
       g_map[p_my2][(p_mx2 + 1) % MAP_DIM].type != EMPTY ||
       g_map[p_my2][mp(p_mx2 - 1) % MAP_DIM].type != EMPTY))
    return false;

  g_map[my][mx].type = type;
  g_map[my][mx].orientation = theta;
  g_map[my][mx].object = {};

  g_map[mp(my+idx.y) % MAP_DIM][mp(mx+idx.x) % MAP_DIM].type =
    g_map[mp(my+2*idx.y) % MAP_DIM][mp(mx+2*idx.x) % MAP_DIM].type =
    STRAIGHT;

  g_map[mp(my+idx.y) % MAP_DIM][mp(mx+idx.x) % MAP_DIM].orientation =
    g_map[mp(my+2*idx.y) % MAP_DIM][mp(mx+2*idx.x) % MAP_DIM].orientation =
    orientation;

  g_map[mp(my+idx.y) % MAP_DIM][mp(mx+idx.x) % MAP_DIM].object =
    randObject();
  g_map[mp(my+2*idx.y) % MAP_DIM][mp(mx+2*idx.x) % MAP_DIM].object =
    randObject();

  if (type == LEFT_RIGHT){
    g_map[mp(my+idx2.y) % MAP_DIM][mp(mx+idx2.x) % MAP_DIM].type =
      g_map[mp(my+2*idx2.y) % MAP_DIM][mp(mx+2*idx2.x) % MAP_DIM].type =
      STRAIGHT;

    g_map[mp(my+idx2.y) % MAP_DIM][mp(mx+idx2.x) % MAP_DIM].orientation =
      g_map[mp(my+2*idx2.y) % MAP_DIM][mp(mx+2*idx2.x) % MAP_DIM].orientation =
      (int)(orientation + 180.0f) % 360;

    g_map[mp(my+idx2.y) % MAP_DIM][mp(mx+idx2.x) % MAP_DIM].object =
      randObject();
    g_map[mp(my+2*idx2.y) % MAP_DIM][mp(mx+2*idx2.x) % MAP_DIM].object =
      randObject();
  }

  return true;
}

// (mx, my) is the endpoint of the pattern to be rolled back
void handleRollBack(int &mx, int &my)
{
  if (g_map[my][mx].type == EMPTY){
    printf("Could not roll back due to invalid endpoint position.\n");
    exit(EXIT_FAILURE);
  }

  float theta = (int)(g_map[my][mx].orientation + 180.0f) % 360;
  Index2D idx = ori2dir[(int)theta];

  g_map[my][mx] = g_map[mp(my+idx.y)%MAP_DIM][mp(mx+idx.x)%MAP_DIM] = EMPTY;

  mx = mp(mx + 2*idx.x) % MAP_DIM;
  my = mp(my + 2*idx.y) % MAP_DIM;

  RoadType exType = g_map[my][mx].type;


  if ((int)exType == 6)
    // Not done yet. It will be finished later.
}

void addOneLevel()
{
  for (std::list<Index2D>::iterator it = g_endpoints.begin();
       it != g_endpoints.end(); ++it){
    while (g_map[it->y][it->x].type == EMPTY){
      it = g_endpoints.erase(it);

      if (it == g_endpoints.end())
	return;
    }

    float theta = g_map[it->y][it->x].orientation;
    Index2D idx = ori2dir[(int)theta % 360];
    int mx = mp(it->x + idx.x) % MAP_DIM,
      my = mp(it->y + idx.y) % MAP_DIM;

    int r = rand() % 4;
    int count = 0;

    while (count < 4){
      if (r == 0){
	// straight
	if (addOneRoadPattern(mx, my, STRAIGHT, theta)){
	  it->x = mp(mx + 2 * idx.x) % MAP_DIM;
	  it->y = mp(my + 2 * idx.y) % MAP_DIM;
	  break;
	}
	else{
	  r = (r + 1) % 4;
	  count++;
	}
      }
      else if (r == 1){
	// left
	if (addOneRoadPattern(mx, my, LEFT, theta)){
	  idx = ori2dir[(int)(theta + 90.0f) % 360];
	  it->x = mp(mx + 2 * idx.x) % MAP_DIM;
	  it->y = mp(my + 2 * idx.y) % MAP_DIM;
	  break;
	}
	else{
	  r = (r + 1) % 4;
	  count++;
	}
      }
      else if (r == 2){
	// right
	if (addOneRoadPattern(mx, my, RIGHT, theta)){
	  idx = ori2dir[(int)(theta + 270.0f) % 360];
	  it->x = mp(mx + 2 * idx.x) % MAP_DIM;
	  it->y = mp(my + 2 * idx.y) % MAP_DIM;
	  break;
	}
	else{
	  r = (r + 1) % 4;
	  count++;
	}
      }
      else if (r == 3){
	// left and right
	if (addOneRoadPattern(mx, my, LEFT_RIGHT, theta)){
	  idx = ori2dir[(int)(theta + 90.0f) % 360];
	  it->x = mp(mx + 2 * idx.x) % MAP_DIM;
	  it->y = mp(my + 2 * idx.y) % MAP_DIM;

	  idx = ori2dir[(int)(theta + 270.0f) % 360];
	  Index2D new_endpoint;
	  new_endpoint.x = mp(mx + 2 * idx.x) % MAP_DIM;
	  new_endpoint.y = mp(my + 2 * idx.y) % MAP_DIM;
	  g_endpoints.push_front(new_endpoint);
	  break;
	}
	else{
	  r = (r + 1) % 4;
	  count++;
	}
      }
    }

    if (count >= 4){
      // roll back one level since no valid pattern can be generated.
      // TODO!!
      printf("Could not add level.\n");
      exit(EXIT_FAILURE);
    }
  }
}

typedef struct Entry
{
  Index2D position;
  float orientation;
  int level;
} Entry;


// (my, mx) must at a joint point
void removeLevels(int mx, int my, int threshold = 2)
{
  std::list<Entry> todo;
  Index2D idx;
  int dx, dy;
  float theta;
  int level = 0;

  theta = g_map[my][mx].orientation;

  while (true){
    idx = ori2dir[(int)(theta + 180.0f) % 360];
    dx = idx.x;
    dy = idx.y;

      //      printf("dx = %d, dy = %d\n", dx, dy);

    if (g_map[mp(my+dy) % MAP_DIM][mp(mx+dx) % MAP_DIM].type == EMPTY)
      break;

    if (level >= threshold)
      g_map[mp(my+dy) % MAP_DIM][mp(mx+dx) % MAP_DIM].type =
	g_map[mp(my+2*dy) % MAP_DIM][mp(mx+2*dx) % MAP_DIM].type = EMPTY;

    mx = mp(mx + 3 * dx) % MAP_DIM;
    my = mp(my + 3 * dy) % MAP_DIM;

    if (g_map[my][mx].type == LEFT_RIGHT){
      Entry e;
      e.position.x = mx;
      e.position.y = my;
      e.orientation = (int)(theta + 180.0f) % 360;
      e.level = level;
      todo.push_back(e);
    }

    theta = g_map[my][mx].orientation;

    if (level >= threshold)
      g_map[my][mx].type = EMPTY;

    level++;
  }

  while (!todo.empty()){
    Entry e = todo.front();
    todo.pop_front();

    theta = e.orientation;
    mx = e.position.x;
    my = e.position.y;
    level = e.level;

    while (true){
      idx = ori2dir[(int)theta % 360];
      dx = idx.x;
      dy = idx.y;

      if (level >= threshold)
	g_map[my][mx].type =
	  g_map[mp(my+dy) % MAP_DIM][mp(mx+dx) % MAP_DIM].type =
	  g_map[mp(my+2*dy) % MAP_DIM][mp(mx+2*dx) % MAP_DIM].type = EMPTY;

      mx = mp(mx + 3*dx) % MAP_DIM;
      my = mp(my + 3*dy) % MAP_DIM;

      level++;

      bool isRoadEnd = false;

      switch (g_map[my][mx].type){
      case LEFT:
	theta = (int)(theta + 90.0f) % 360;
	break;
      case RIGHT:
	theta = (int)(theta + 270.0f) % 360;
	break;
      case LEFT_RIGHT:
	theta = (int)(theta + 90.0f) % 360;
	e.orientation = (int)(theta + 180.0f) % 360;
	e.position.x = mx;
	e.position.y = my;
	e.level = level;
	todo.push_back(e);
	break;
      case EMPTY:
	isRoadEnd = true;
	break;
      }

      if (isRoadEnd)
	break;
    }
  }
}

void ml_init(float wx, float wy, int initDepth)
{
  wx = wx + 1.5f;

  int mx = WORLD_TO_MAP(wx), my = WORLD_TO_MAP(wy);

  g_iStartX = mx;
  g_iStartY = my;

  Index2D idx;

  idx = {0, 1};
  ori2dir[0] = idx;
  idx = {-1, 0};
  ori2dir[90] = idx;
  idx = {0, -1};
  ori2dir[180] = idx;
  idx = {1, 0};
  ori2dir[270] = idx;

  memset(g_map, 0, MAP_DIM*MAP_DIM*sizeof(RoadType));

  g_map[my][mx] = {START, 0.0f, {}};
  g_map[(my+1)%MAP_DIM][mx] = g_map[(my+2)%MAP_DIM][mx] =
    {STRAIGHT, 0.0f, {}};

  Index2D pt = {mx, (my+2)%MAP_DIM};

  g_endpoints.push_back(pt);

  pt = {mx, (my+1)%MAP_DIM};
  g_anticipatedNextRoads.push_back(pt);

  for (int i = 0; i < initDepth; ++i)
    addOneLevel();

  g_bIsInitialized = true;
}

bool isTimeToUpdate(int mx, int my)
{
  static int count = 0;

  for (int i = 0; i < g_anticipatedNextRoads.size(); ++i){
    if (g_anticipatedNextRoads[i].x == mx &&
	g_anticipatedNextRoads[i].y == my){
      ++count;
      g_anticipatedNextRoads.clear();

      Road &r = g_map[my][mx];
      float theta = r.orientation;

      if (r.type == LEFT || r.type == LEFT_RIGHT)
	theta = (int)(theta + 90.0f) % 360;
      else if (r.type == RIGHT)
	theta = (int)(theta + 270.0f) % 360;

      Index2D idx = ori2dir[(int)theta];
      Index2D nextRoadIdx = {mp(mx+idx.x)%MAP_DIM, mp(my+idx.y)%MAP_DIM};

      g_anticipatedNextRoads.push_back(nextRoadIdx);

      if (r.type == LEFT_RIGHT)
	theta = (int)(theta + 180.0f) % 360;

      idx = ori2dir[(int)theta];
      nextRoadIdx = {mp(mx+idx.x)%MAP_DIM, mp(my+idx.y)%MAP_DIM};
      g_anticipatedNextRoads.push_back(nextRoadIdx);

      break;
    }
  }

  if (count == 3){
    count = 0;
    return true;
  }
  return false;
}

void ml_update(float wx, float wy)
{
  if (!g_bIsInitialized)
    return;

  wx = wx + 1.5f; // map world to logical coordinates

  int mx = WORLD_TO_MAP(wx), my = WORLD_TO_MAP(wy);

  if (!isTimeToUpdate(mx, my))
    return;

  float theta = g_map[my][mx].orientation;
  Index2D idx = ori2dir[(int)(theta + 180.0f) % 360];

  while (mx % 3 != 0)
    mx = mp(mx + idx.x) % MAP_DIM;

  while (my % 3 != 0)
    my = mp(my + idx.y) % MAP_DIM;

  removeLevels(mx, my);
  addOneLevel();
}
