/**
 * Author: Rajesh
 * Description: The implementation of the MIST class which is used to simulate
 *   mist in 3D scenes.
 */

#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <GL/freeglut.h>

#include "Mist.h"

using namespace rps;

Mist::Mist(float x_span, float y_span, float z_span,
	   float x, float y, float z, float density)
{
  size.x = x_span;
  size.y = y_span;
  size.z = z_span;
  position.x = x;
  position.y = y;
  position.z = z;
  this->density = density;

  const int numOfParticlesPerSample = 100;
  const int numOfSamples = (int)((x_span/stepWidth) *
				 (y_span/stepWidth) * (z_span/stepWidth));
  const int numOfParticles = numOfSamples * numOfParticlesPerSample * density;

  Particle p;

  srand(time(NULL));

  for (int i = 0; i < numOfParticles; ++i){
    initializeParticle(p, size);
    particles.push_back(p); 
  }		       
}

// Delegate constructor - new feature introduced in C++ 11
Mist::Mist(Volume3D size, Point3D position, float density)
  : Mist(size.x, size.y, size.z, position.x, position.y, position.z, density)
{
}

void Mist::draw()
{
  glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);
  glDisable(GL_LIGHTING);
  glEnable(GL_BLEND);

  glPushMatrix();

  glTranslatef(position.x, position.y, position.z);
  glBegin(GL_QUADS);

  Particle* p;

  for (int i = 0; i < particles.size(); ++i){
    p = &particles[i];
    glColor4f(p->color.r, p->color.g, p->color.b, p->color.a);
    glVertex3f(p->position.x - p->size/2.0f, p->position.y,
	       p->position.z - p->size/2.0f);
    glVertex3f(p->position.x + p->size/2.0f, p->position.y,
	       p->position.z - p->size/2.0f);
    glVertex3f(p->position.x + p->size/2.0f, p->position.y,
	       p->position.z + p->size/2.0f);
    glVertex3f(p->position.x - p->size/2.0f, p->position.y,
	       p->position.z + p->size/2.0f);
  }
  glEnd();

  glPopMatrix();

  glPopAttrib();
}

void Mist::initializeParticle(Particle &p, Volume3D vol)
{
  static const int levelOfDetail = 1 << 20;

  p.position.x = (float)((rand() % levelOfDetail /
			   (double)levelOfDetail - 0.5) * vol.x);
  p.position.y = (float)((rand() % levelOfDetail /
			   (double)levelOfDetail - 0.5) * vol.y);
  p.position.z = (float)pow(rand() % levelOfDetail /
			    (double)levelOfDetail, 3.0);
  p.color.r = p.color.g = p.color.b =
    (rand() % levelOfDetail / (double)levelOfDetail) * 0.7 + 0.3;
  p.color.a = 1.0f - p.position.z;
  p.size = 0.001f;
}
