/**
 * Author: Rajesh
 * Description: an implementation of Fireball.hpp.
 */

#include <GL/freeglut.h>
//#include <GL/gl.h>
#define __USE_BSD
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "Fireball.hpp"

using namespace ao;

Fireball::Fireball(float r)
{
  radius = r;
  maxLifespan = (int)(r / rDelta * 2.0f);

  for (int i = 0; i < maxLifespan; ++i)
    update();
}

void Fireball::update()
{
  float dr;

  for(int i = 0; i < livePars.size(); ++i){
    ParticleR &p = livePars[i];

    if (p.lifespan > 0){
      p.lifespan--;

      if (p.lifespan <= 0)
	deadParIdx.push_back(i);
      else{
	dr = (float)(rand() % 1000) / 1000.0f * rDelta;
	p.position.x += dr * cos(p.pitch) * cos(p.yaw);
	p.position.y += dr * cos(p.pitch) * sin(p.yaw);
	p.position.z += dr * sin(p.pitch);
      }
    }
  }

  addNewParticles();
}

void Fireball::draw()
{
  glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);
  glEnable(GL_COLOR_MATERIAL);
  glDisable(GL_DEPTH_TEST);

  glBegin(GL_POINTS);
  for (std::vector<ParticleR>::iterator it = livePars.begin();
       it != livePars.end(); ++it){
    glColor4f(1.0f, 0.6f, 0.0f, (float)it->lifespan/(float)maxLifespan);
    glVertex3f(it->position.x, it->position.y, it->position.z);
    glVertex3f(it->position.x+0.005f, it->position.y, it->position.z);
    glVertex3f(it->position.x-0.005f, it->position.y, it->position.z);
    glVertex3f(it->position.x, it->position.y+0.005f, it->position.z);
    glVertex3f(it->position.x, it->position.y-0.005f, it->position.z);
  }
  glEnd();

  glPopAttrib();
}

void Fireball::addNewParticles()
{
  ParticleR pr;
  int i;

  for (float y = 0.0f; y < 360.0f; y += yStepWidth){
    for (float p = 90.0f; p > -90.0f; p -= pStepWidth){
      pr.position.x = pr.position.y = pr.position.z = 0.0f;
      pr.lifespan = maxLifespan;
      pr.yaw = (y+(float)(rand()%1000)/1000.0f*yStepWidth)/180.0f*M_PI;
      pr.pitch = (p-(float)(rand()%1000)/1000.0f*pStepWidth)/180.0f*M_PI;

      if (deadParIdx.size() > 0){
	i = deadParIdx.back();
	deadParIdx.pop_back();
	livePars[i] = pr;
      }
      else
	livePars.push_back(pr);
    }
  }
}
