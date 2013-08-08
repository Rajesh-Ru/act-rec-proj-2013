/**
 * Author: Rajesh
 * Decription: defines the interfaces necessary for draw local models.
 */

#include "../glm/glm.h"
#include "../common/common.h"
#include "Token.hpp"
#include "DemonShock.hpp"
#include "Tree.hpp"
#include "FireSerpent.hpp"

#ifndef LOCAL_MODEL_DRAWING_INTERFACES_H
#define LOCAL_MODEL_DRAWING_INTERFACES_H

void lmdi_loadModels();

void lmdi_draw(RoadType roadType, Object *pObject);

void lmdi_destroy();

#endif // LOCAL_MODEL_DRAWING_INTERFACES_H
