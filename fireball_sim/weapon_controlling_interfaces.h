/**
 * Author: Rajesh
 * Description: external interfaces for manipulating weapons.
 */

#include "AnimatedObject.hpp"

#ifndef WEAPON_CONTROLLING_INTERFACES_H
#define WEAPON_CONTROLLING_INTERFACES_H

void wd_add(ao::WeaponType type,
	    float posX, float posY, float posZ,
	    float dirX, float dirY, float dirZ,
	    float speed, float dist);

void wd_update();

void wd_draw();

void wd_destroy();

#endif // WEAPON_CONTROLLING_INTERFACES_H
