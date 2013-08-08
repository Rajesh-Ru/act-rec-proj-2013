/**
 * Author: Rajesh
 * Description: external inferfaces of handling map logic.
 */

#ifndef MAP_INTERFACES_H
#define MAP_INTERFACES_H

void ml_init(float wx, float wy, int initDepth = 3);

void ml_update(float wx, float wy);

void md_init();

void md_draw(float wx, float wy, bool drawBackward = false);

void md_destroy();

#endif // MAP_INTERFACES_H
