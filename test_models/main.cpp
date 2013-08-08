/**
 * Author: Rajesh
 */

#include <stdlib.h>
#include <string.h>
#include <GL/freeglut.h>
#include "glm.h"

static char* g_strModelFilePath = NULL;
static GLMmodel *g_pModel = NULL;
static float g_fRotateAngle = 0.0f;
static int g_iGLMDrawOptions = 0;
static float dx = 0.0f, dy = 0.0f, dz = 0.0f;

void init()
{
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glShadeModel(GL_SMOOTH);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  glEnable(GL_DEPTH_TEST);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  g_pModel = glmReadOBJ(g_strModelFilePath); // call glmDelete when done
}

void reshape(int w, int h)
{
  static const double clipDim = 2.5;

  glViewport(0, 0, (GLsizei)w, (GLsizei)h);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  if (w <= h)
    glOrtho(-clipDim, clipDim, -clipDim*(double)h/(double)w, clipDim*(double)h/(double)h, -clipDim, clipDim);
  else
    glOrtho(-clipDim*(double)w/(double)h, clipDim*(double)w/(double)h, -clipDim, clipDim, -clipDim, clipDim);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void timerCB(int val)
{
  g_fRotateAngle += 0.5f;

  glutPostRedisplay();

  glutTimerFunc(50, timerCB, 0);
}

void display(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glPushMatrix();
  glTranslatef(dx, dy, dz);
  glRotatef(g_fRotateAngle, 0.0f, 1.0f, 0.0f);
  glRotatef(-60.0f, 1.0f, 0.0f, 0.0f);
  glmDraw(g_pModel, g_iGLMDrawOptions);
  glPopMatrix();

  glutSwapBuffers();
}

int main(int argc, char** argv)
{
  if (argc < 2)
    return -1;

  for (int i = 1; i < argc; ++i){
    if (argv[i][0] != '-' && g_strModelFilePath == NULL)
      g_strModelFilePath = argv[i];
    else {
      for (int j = 1; argv[i][j] != '\0'; ++j){
	if (argv[i][j] == 'm'){
	  g_iGLMDrawOptions |= GLM_MATERIAL;
	}
	else if (argv[i][j] == 't'){
	  g_iGLMDrawOptions |= GLM_TEXTURE;
	}
	else if (argv[i][j] == 'x' && argv[i][j+1] == '\0'){
	  dx = atof(argv[++i]);
	  break;
	}
	else if (argv[i][j] == 'y' && argv[i][j+1] == '\0'){
	  dy = atof(argv[++i]);
	  break;
	}
	else if (argv[i][j] == 'z' && argv[i][j+1] == '\0'){
	  dz = atof(argv[++i]);
	  break;
	}
	else
	  return -1;
      }
    }
  }
  g_iGLMDrawOptions |= GLM_SMOOTH;

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(500, 500);
  glutCreateWindow("Test Models");
  init();
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutTimerFunc(50, timerCB, 0);
  glutMainLoop();

  return 0;
}
