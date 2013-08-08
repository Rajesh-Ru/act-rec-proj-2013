/**
 * Author: Rajesh
 * Description: an implementation of Fire.hpp.
 */

#include <GL/freeglut.h>
#define __USE_BSD
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include "Fire.hpp"

using namespace ao;

Fire::Fire(float l, float r)
{
  length = l;
  radius = r;
  maxLifespan = (int)(l / lStepWidth);

  srand(time(NULL));

  for (int i = 0; i < maxLifespan; ++i)
    update();
}

void Fire::update()
{
  for (std::list<Particle>::iterator it = particles.begin();
       it != particles.end(); ++it){
    --(it->lifespan);

    if (it->lifespan <= 0){
      it = particles.erase(it);
      --it;
      continue;
    }

    it->position.x += lStepWidth;
  }

  addNewParticles();
}

void Fire::draw()
{
  glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);
  glEnable(GL_COLOR_MATERIAL);
  glDisable(GL_DEPTH_TEST);

  glBegin(GL_POINTS);
  for (std::list<Particle>::iterator it = particles.begin();
       it != particles.end(); ++it){
    glColor4f(1.0f, 0.6f, 0.0f, (float)it->lifespan/(float)maxLifespan);
    glVertex3f(it->position.x, it->position.y, it->position.z);
    glVertex3f(it->position.x+0.005f, it->position.y, it->position.z);
    glVertex3f(it->position.x-0.005f, it->position.y, it->position.z);
    glVertex3f(it->position.x, it->position.y+0.005f, it->position.z);
    glVertex3f(it->position.x, it->position.y-0.005f, it->position.z);
    glVertex3f(it->position.x+0.005f, it->position.y+0.005f, it->position.z);
    glVertex3f(it->position.x-0.005f, it->position.y+0.005f, it->position.z);
    glVertex3f(it->position.x-0.005f, it->position.y-0.005f, it->position.z);
    glVertex3f(it->position.x+0.005f, it->position.y-0.005f, it->position.z);
  }
  glEnd();

  glPopAttrib();
}

// add one slice of new particles
inline void Fire::addNewParticles()
{
  Particle p;

  for (float theta = 0.0f; theta < 360.0f; theta += aStepWidth){
    for (float d = rStepWidth; d < radius; d += rStepWidth){
      p.position.x = (rand() % 1000 / 1000.0f) * lStepWidth;
      p.position.y = d * sin(theta/180.0f*M_PI);
      p.position.z = d * cos(theta/180.0f*M_PI);
      p.lifespan = sqrt(1.0f - d/radius) *
	(rand() % 1500 + 500.0f) / 2000.0f * (float)maxLifespan;

      particles.insert(particles.begin(), p);
    }
  }
}
