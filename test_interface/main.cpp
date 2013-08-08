#include "Game.h"
#include <stdio.h>

Random::Random()
{
}

Game::Game(std::function<void(Game &)> initializer)
{
}

Game::~Game()
{
}

void drawRoadPreliminaries(Game const &game)
{
  printf("This is drawRoadPreliminaries.\n");
}

void drawRoad(Game const &game, Game::RoadType type, int id,
	      std::vector<Game::Barrier> const &barriers)
{
  printf("This is drawRoad.\n");
}

void drawRoadFollowups(Game const &game)
{
  printf("This is drawRoadFollowups.\n");
}

int main(int argc, char** argv)
{
  Game game([](Game&){});

  game.BeforeDrawRoad += drawRoadPreliminaries;
  game.OnDrawRoad += drawRoad;
  game.AfterDrawRoad += drawRoadFollowups;

  game.BeforeDrawRoad.begin()->second(game);
  game.OnDrawRoad.begin()->second(game, Game::RoadType::straight, 0, std::vector<Game::Barrier>());
  game.AfterDrawRoad.begin()->second(game);

  return 0;
}
