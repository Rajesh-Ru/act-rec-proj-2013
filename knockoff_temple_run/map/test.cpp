/**
 * Author: Rajesh
 * Description: test the map logic module.
 */

#include <time.h>
#include <stdlib.h>
#include <iostream>

#include "../common/common.h"
#include "map_logic.h"

extern Road g_map[MAP_DIM][MAP_DIM];

float wx = 0.0f, wy = 0.0f;

extern int mp(int x);

void drawMap()
{
  std::cout << std::endl;

  for (int y = MAP_DIM - 1; y >= 0; --y){
    for (int x = 0; x < MAP_DIM; ++x){
      if (x == (int)mp(wx / 3.0f) % 51 &&
	  y == (int)mp(wy / 3.0f) % 51){
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

int main(int argc, char** argv)
{
  srand(time(NULL));

  ml_init(wx, wy);

  drawMap();

  char c = std::cin.get();
  std::cin.ignore();

  while (true){
    switch (c){
    case 'w':
      wy += 9.0f;
      break;
    case 's':
      wy -= 9.0f;
      break;
    case 'a':
      wx -= 9.0f;
      break;
    case 'd':
      wx += 9.0f;
      break;
    case 'q':
      exit(0);
    }

    ml_update(wx, wy);
    drawMap();

    c = std::cin.get();
    std::cin.ignore();
  }

  return 0;
}
