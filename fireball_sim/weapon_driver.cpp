/**
 * Author: Rajesh
 * Description: A simple driver that controls weapons.
 */

#include <stdio.h>
#include <GL/gl.h>
#include <math.h>
#include <list>

#include "Fireball.hpp"
#include "AnimatedObject.hpp"

using namespace ao;

std::vector<Weapon> g_weapons;
std::vector<int> g_expiredWeaponIdx;

void wd_add(WeaponType type,
	    float posX, float posY, float posZ,
	    float dirX, float dirY, float dirZ,
	    float speed, float dist)
{
  Weapon wp;
  float vecLen = sqrt(dirX*dirX + dirY*dirY + dirZ*dirZ);

  wp.position.x = posX;
  wp.position.y = posY;
  wp.position.z = posZ;

  wp.direction.x = dirX / vecLen;
  wp.direction.y = dirY / vecLen;
  wp.direction.z = dirZ / vecLen;

  wp.speed = speed;
  wp.dist = dist;

  switch (type){
  case FIREBALL:
    wp.pObject = new Fireball(0.5f);
    break;
  default:
    // do nothing
    break;
  }

  if (!g_expiredWeaponIdx.empty()){
    int i = g_expiredWeaponIdx.back();

    g_expiredWeaponIdx.pop_back();
    g_weapons[i] = wp;
  }
  else
    g_weapons.push_back(wp);
}

void wd_update()
{
  for (int i = 0; i < g_weapons.size(); ++i){
    Weapon &wp = g_weapons[i];

    if (wp.dist > 0){
      wp.position.x += wp.direction.x * wp.speed;
      wp.position.y += wp.direction.y * wp.speed;
      wp.position.z += wp.direction.z * wp.speed;
      wp.dist -= wp.speed;
      wp.pObject->update();
   
      if (wp.dist < 0){
	g_expiredWeaponIdx.push_back(i);
	delete wp.pObject;
	wp.pObject = NULL;
      }
    }
  }
}

void wd_draw()
{
  for (int i = 0; i < g_weapons.size(); ++i){
    Weapon &wp = g_weapons[i];

    if (wp.dist > 0){
      glPushMatrix();
      glTranslatef(wp.position.x, wp.position.y, wp.position.z);
      wp.pObject->draw();
      glPopMatrix();
    }
  }
}

void wd_destroy()
{
  for (int i = 0; i < g_weapons.size(); ++i){
    Weapon &wp = g_weapons[i];

    if (wp.pObject != NULL)
      delete wp.pObject;
  }
}
