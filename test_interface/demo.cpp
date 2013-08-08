/**
* ���ߣ����ٺ�
*/
#include "openglglut.h"
#include "Narudo.h"
#include "glm.h"
#include "camera.h"
#include "Sand.h"
#include "BarrierManager.h"
#include "InputManager.h"
#include "FreeType.h"
#include "Game.h"
#define KEYBOARD
//#define KINECT
#define MOTION_BLUR
#ifdef KINECT
#include "Particle.h"
#endif
#include <sstream>

#ifdef KINECT
#include "DriveKinect.h"
#endif
//#include <vld.h>
extern bool raise_hand;
const float DISTANCE = 10;
Narudo narudo;
GLMmodel* pmodel = NULL;
//GLMmodel* bananaModel = NULL;
bool motion_blur = false;
float speed_up_time = 0;
float MP = 0;

//GLMmodel* flag = NULL;
BarrierManager* barrierManager = NULL;
GLfloat light_ambient[]={ 1.0f, 1.0f, 1.0f, 1.0f };      /** ���廷����ǿ��*/
GLfloat light_diffuse[]={ 0.8f, 0.8f, 0.8f, 1.0f};      /** �����������ǿ��*/
GLfloat light_specular[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat light_position[4]={1.0f , 1.0f , 1.0f , 1.0f};     /** �����Դ��λ��*/
GLfloat shininess = 65.0; 
Camera* camera;
Sand* map;
bool line = false;
int scale = 1;
void initData();
void resetData();
void releaseData();
void update();
InputManager moveInputManager;
GameState gameState = READY;
freetype::font_data font;
void onRunning();
void onReady();
void onEnd();
void onHit();
void pushScreenCoordinateMatrix();
void pop_projection_matrix();
float current_time = 0.f;
//float now_time = 0.f;
//float pre_time = 0.f;
int init_length = 0;
float hit_time = 0.f;
float current_x = 0 ,current_y = 0,current_z = 1000;
float old_x = 0 ,old_y = 0,old_z = 1000;
float select_x = currentWidth/2,select_y = currentHeight*0.7f;
#ifdef KINECT
Particle* particle;
#endif KINECT

void TestGame(Game& g){
	g.RoadSize = 64;
	g.OnInitialized += [](Game const&){
		puts("Test Game : OnInitialized");
	};
	g.OnFinishGame += [](Game const&){
		puts("Test Game : OnFinishGame");
	};
	g.BeforeDrawRoad += [](Game const& g){
		glPushAttrib(GL_CURRENT_BIT);
		glDisable(GL_COLOR_MATERIAL);
		TextureManager::Inst()->BindTexture(MAP);
		glColor3f(0.5f,0.8, 1.0f);
		glPushMatrix();
		glScalef(g.RoadSize, g.RoadSize, g.RoadSize);
	};
	g.OnDrawRoad += [](Game const&, Game::RoadType type, int RoadID, std::vector<Game::Barrier> const& Barriers){
		if(Game::RoadType::none == type){
			glBegin(GL_QUADS);
			glTexCoord2i(0, 0); glVertex2f(-1.5, 0);
			glTexCoord2i(1, 0); glVertex2f(1.5, 0);
			glTexCoord2i(1, 1); glVertex2f(1.5, 3);
			glTexCoord2i(0, 1); glVertex2f(-1.5, 3);
			glEnd();
			return;
		}
		float dx = 0, dy = 0;
		glBegin(GL_QUADS);
		glTexCoord2i(0, 0); glVertex2f(-0.5 + dx, dy);
		glTexCoord2i(1, 0); glVertex2f(0.5 + dx, dy);
		glTexCoord2i(1, 1); glVertex2f(0.5 + dx, 1 + dy);
		glTexCoord2i(0, 1); glVertex2f(-0.5 + dx, 1 + dy);
		glEnd();
		dy = 1;
		glBegin(GL_QUADS);
		glTexCoord2i(0, 0); glVertex2f(-0.5 + dx, dy);
		glTexCoord2i(1, 0); glVertex2f(0.5 + dx, dy);
		glTexCoord2i(1, 1); glVertex2f(0.5 + dx, 1 + dy);
		glTexCoord2i(0, 1); glVertex2f(-0.5 + dx, 1 + dy);
		glEnd();
		if(type & Game::RoadType::left){
			dx = -1;
			glBegin(GL_QUADS);
			glTexCoord2i(0, 0); glVertex2f(-0.5 + dx, dy);
			glTexCoord2i(1, 0); glVertex2f(0.5 + dx, dy);
			glTexCoord2i(1, 1); glVertex2f(0.5 + dx, 1 + dy);
			glTexCoord2i(0, 1); glVertex2f(-0.5 + dx, 1 + dy);
			glEnd();
		}
		if(type & Game::RoadType::right){
			dx = 1;
			glBegin(GL_QUADS);
			glTexCoord2i(0, 0); glVertex2f(-0.5 + dx, dy);
			glTexCoord2i(1, 0); glVertex2f(0.5 + dx, dy);
			glTexCoord2i(1, 1); glVertex2f(0.5 + dx, 1 + dy);
			glTexCoord2i(0, 1); glVertex2f(-0.5 + dx, 1 + dy);
			glEnd();
		}
		if(type & Game::RoadType::straight){
			dx = 0;
			dy = 2;
			glBegin(GL_QUADS);
			glTexCoord2i(0, 0); glVertex2f(-0.5 + dx, dy);
			glTexCoord2i(1, 0); glVertex2f(0.5 + dx, dy);
			glTexCoord2i(1, 1); glVertex2f(0.5 + dx, 1 + dy);
			glTexCoord2i(0, 1); glVertex2f(-0.5 + dx, 1 + dy);
			glEnd();
		}
	};
	g.AfterDrawRoad += [](Game const&){
		glPopMatrix();
		glPopAttrib();
	};
	g.OnDrawPlayer += [](Game const& g, Game::TPlayer const& p){
		//narudo.onAction(Action::SUCCESS);
		//glScalef(g.RoadSize, g.RoadSize, g.RoadSize);
		//glTranslatef(p.offset, 0, 0);
		//narudo.DrawModels();
		glDisable(GL_COLOR_MATERIAL);
		glColor3f(1, 0, 0);
		//glutSolidSphere(10, 32, 32);
		glBegin(GL_QUADS);
		glNormal3i(0, 0, 1); glVertex3i(-5, -5, 0);
		glNormal3i(0, 0, 1); glVertex3i(-5, 5, 0);
		glNormal3i(0, 0, 1); glVertex3i(5, 5, 0);
		glNormal3i(0, 0, 1); glVertex3i(5, -5, 0);
		glEnd();
	};
	g.OnCreateRoad = [](Game const&, int RoadID, std::vector<Game::Barrier>&, Game::RoadType mask){
		puts("Test Game : OnCreateRoad");
		auto r = Game::RoadType::all & RoadID & mask;
		return Game::RoadType(r ? r : Game::RoadType::straight);
	};
	g.OnPlayerUpdate = [](Game const&, Game::TPlayer& p, float, float, std::vector<Game::Barrier>&){
		p.position += 0.0625;
	};
	g.OnHitWall += [](Game& g, Game::TPlayer&, float, float){
		g.End();
	};
}
Game g(TestGame);
int main(int argc, char** argv)
{
	/** ��ʼ������ ����������*/
#ifdef KINECT
	initialKinect();
#endif
	createWindow("Climp Tree",&argc,argv);
	//glewInit();
	//glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE ,GLUT_ACTION_CONTINUE_EXECUTION);
	/** ע��glut��һЩ����*/
	glutReshapeFunc(glutResize);     //���ڵ������� ʹ�õ�����ͼ����ʾ�������ı�
	glutDisplayFunc(glutDisplay);    //�ػ溯��     ʹ�ÿ����ڴ����л���ͼ��
	glutSpecialFunc(glutSpecial);    //���ⰴ������ ʹ��ʵ��ȫ��/�����л�
	glutTimerFunc(24,glutTimer,0);
#ifdef KEYBOARD
	glutKeyboardFunc(glutKeyboard);
	glutKeyboardUpFunc(glutKeyboardUp);
#endif
	glutIdleFunc(glutIdle);
	/** ��ʼ��opengl��һЩ����*/
	InitOpenGL();                      
	InitDepthTest();                            /** ������Ȳ���*/
	InitTexture();
	InitLight();
	//InitFog();
	//InitBlend();
	//InitCullFace();
	glEnable(GL_ALPHA_TEST);  
	glAlphaFunc(GL_GREATER, 0.8f);
	//glEnable(GL_MULTISAMPLE);//�������ػ���
	initData();
	/** �������ѭ��*/
	glutMainLoop();
	releaseData();
#ifdef KINECT
	quitKinect();
#endif
	//system("pause");
	return 0;
}

void InitTexture(){
	glEnable(GL_TEXTURE_2D);
}

void InitBlend(void)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

}
void InitLight(void)
{
	/** ����������1�Ź�Դ*/
	glLightfv(GL_LIGHT1,GL_AMBIENT,light_ambient);                  /** ��ʼ����Դ1������*/
	glLightfv(GL_LIGHT1,GL_DIFFUSE,light_diffuse);                  /** ��ʼ����Դ1ɢ���*/
	glMaterialfv(GL_FRONT, GL_SPECULAR, light_specular);
	glMaterialf(GL_FRONT, GL_SHININESS, shininess);	
	glLightfv(GL_LIGHT1,GL_POSITION,light_position);                /** ��ʼ����Դ1λ��*/

	//glLightModelfv(GL_LIGHT_MODEL_AMBIENT,light_ambient);
	/** liggt_position ����(x,y,z,w) wΪ0 ��ʾ�����Թ�Դ,w��0��ʾλ���Թ�Դ*/
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHTING);

}

void InitFog()
{
	GLfloat fogColor[4]= {0.5f, 0.5f, 0.5f, 0.4f};
	glFogi(GL_FOG_MODE, GL_LINEAR );		// ����������ģʽ
	glFogfv(GL_FOG_COLOR, fogColor);			// ���������ɫ
	glFogf(GL_FOG_DENSITY, 0.35f);			// ��������ܶ�
	glHint(GL_FOG_HINT, GL_DONT_CARE);			// ����ϵͳ��μ�������
	//glFogf(GL_FOG_START, -20.0f);				// �����Ŀ�ʼλ��
	//glFogf(GL_FOG_END, -25.0f);				// �����Ľ���λ��
	glEnable(GL_FOG);					// ʹ������
}

void InitCullFace()
{
	glEnable (GL_CULL_FACE);	
	glCullFace (GL_BACK);
}

/** ʵ��ͷ�ļ����ж���Ļ��ƺ��� **/
void glutDisplay(void)
{
	//glMatrixMode(GL_PROJECTION);/** ����ͶӰ����*/
	//glPushMatrix();
	//glLoadIdentity();
	//gluOrtho2D(0,currentWidth,0,currentHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);/** ����ͶӰ����*/
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0,currentWidth,0,currentHeight);
	//glViewport(0, 0, currentWidth, currentHeight);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	TextureManager::Inst()->BindTexture(BACKGROUND);
	glDepthMask(GL_FALSE);
	glDisable(GL_COLOR_MATERIAL);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	//glBegin(GL_QUADS);
	//glTexCoord2i(0,0); glVertex2i(0,0); 
	//glTexCoord2i(1,0); glVertex2i(currentWidth,0); 
	//glTexCoord2i(1,1); glVertex2i(currentWidth,currentHeight); 
	//glTexCoord2i(0,1); glVertex2i(0,currentHeight);
	//glEnd();
	glDepthMask(GL_TRUE);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glLightfv(GL_LIGHT1,GL_POSITION,light_position);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity(); //���ȳ�ʼ����λ����
	//glPushMatrix();
	//glTranslatef(0,0.0,-1);
	//glRotatef(10,1,0,0);
	//camera->update();
	gluLookAt(0, 0, 1024, 0, 0, 0, 0, 1, 0);
	//glEnable(GL_BLEND);
	//glFogf(GL_FOG_START, -25);				// �����Ŀ�ʼλ��
	//glFogf(GL_FOG_END, -20);	
	//glDisable(GL_BLEND);
	// �����Ľ���λ��
	//glPushMatrix();
	//Point temp = map->getField();
	//glTranslatef(-temp.x/2,-temp.y-5,0);
	//glScalef(128, 128, 128);
	//narudo.onAction(Action::SUCCESS);
	//narudo.DrawModels();
	g.update();//map->draw();
	//g.TryTurnRight();
	//glPolygonMode (GL_FRONT, GL_LINE);			// ����������
	//glLineWidth (5);				// �����߿�
	//glEnable(GL_LINE_SMOOTH);
	//glEnable (GL_CULL_FACE);	
	////glCullFace (GL_FRONT);				// �޳�ǰ��Ķ����
	//glDepthFunc (GL_LEQUAL);				// �ı����ģʽ
	//glColor4f (0,0,0,1);			// �涨������ɫ
	////glUseProgram(0);
	//glDisable(GL_TEXTURE_2D);
	//map->draw();
	//glDepthFunc (GL_LESS);				// ������Ȳ���ģʽ
	////glCullFace (GL_BACK);				// �����޳���������
	//glPolygonMode (GL_FRONT, GL_FILL);			// ���ñ������λ��Ʒ�ʽ
	//glDisable (GL_CULL_FACE);
	//glEnable(GL_TEXTURE_2D);
	//glLineWidth (1);
	//glPopMatrix();
	//glPopMatrix();
	//glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
	/*for(int i = 0;i<128;i++){
	if(fabs(-10*i-camera->getPosition().z)>100) continue;
	glPushMatrix();
	glTranslatef(5,map->getHeight(2,-10*i)-1,-10*i);
	glRotatef(-45,0,0,1);
	glScalef(5,5,5);
	glmDraw(pmodel, GLM_SMOOTH |GLM_TEXTURE|GLM_COLOR);
	glPopMatrix();
	}

	for(int i = 0;i<128;i++){
	if(fabs(-10*i-camera->getPosition().z)>100) continue;
	glPushMatrix();
	glTranslatef(-1,map->getHeight(2,-10*i)-1,-10*i);
	glRotatef(45,0,0,1);
	glScalef(5,5,5);
	glmDraw(pmodel, GLM_SMOOTH |GLM_TEXTURE|GLM_COLOR);
	glPopMatrix();
	}*/
	//barrierManager->draw(camera,pmodel);

	//glPushMatrix();
	////glLoadIdentity();
	////glScalef(0.05,0.05,0.05);
	//glTranslatef(2,map->getHeight(2,-20),-20);
	//glRotatef(90,0,1,0);
	////glmDraw(bananaModel, GLM_SMOOTH |GLM_TEXTURE|GLM_COLOR);
	//glPopMatrix();

	//narudo.DrawModels();
	
	glMatrixMode(GL_PROJECTION);/** ����ͶӰ����*/
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0,currentWidth,0,currentHeight);

	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
	glLoadIdentity();
	glColor3f(0.f,0.f,0.f);
	std::stringstream time;
	int hour = (int)current_time/3600;
	int minute = (int)current_time/60%60;
	int second = (int)current_time%60;
	if(hour<10){
		time<<0;
	}
	time<<hour<<':';
	if(minute<10){
		time<<0;
	}
	time<<minute<<':';
	if(second<10){
		time<<0;
	}
	time<<second;
	freetype::print(font, (float)(currentWidth/2-80), (float)(currentHeight-70), time.str().c_str());
	switch(gameState)
	{
	case READY:
		freetype::print(font, (float)currentWidth/2-180, (float)currentHeight/2+50,"Raise your hand to go!");
		break;
	case END:
		freetype::print(font, (float)currentWidth/2-180, (float)currentHeight/2+50," Game end!Replay?N/Y");
		break;
	}
	freetype::print(font, (float)currentWidth-150, (float)currentHeight-70,"%d M",(int)(init_length-camera->getPosition().z));

	//glPushMatrix();
	//glDisable(GL_DEPTH_TEST);
	glPopMatrix();

	/*glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0f, (GLfloat) currentWidth, 0.0f, 
	(GLfloat) currentHeight);
	glViewport(0, 0, currentWidth, currentHeight);
	*/
	#ifdef KINECT
	particle->draw();
	#endif

	//glEnable(GL_LIGHTING);
	glLoadIdentity();
	//glColor3f(1.f,0.f,0.f);
	//glBegin(GL_QUADS);
	//	glVertex2f(select_x-20,select_y-20);
	//	glVertex2f(select_x+20,select_y-20);
	//	glVertex2f(select_x+20,select_y+20);
	//	glVertex2f(select_x-20,select_y+20);
	//glEnd();*/
	//glPopMatrix();
	glPushAttrib( GL_CURRENT_BIT  | GL_ENABLE_BIT );
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);	
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_ALPHA_TEST);  
	TextureManager::Inst()->BindTexture(QI);
	//printf("%f\n",MP);
	if(MP<5){
		glColor3f(0.f,1.f,1.f);
	}
	else if(MP>=5&&MP<10){
		glColor3f(1.f,0.5f,0.f);
	}
	else{
		glColor3f(1.f,0.f,0.f);
	}
	glBegin(GL_QUADS);
	glTexCoord2i(0,0);glVertex2f(currentWidth-90-64,90-64);
	glTexCoord2i(1,0);glVertex2f(currentWidth-90+64,90-64);
	glTexCoord2i(1,1);glVertex2f(currentWidth-90+64,90+64);
	glTexCoord2i(0,1);glVertex2f(currentWidth-90-64,90+64);
	glEnd();
	glPopAttrib();	
	//glEnable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
#ifdef MOTION_BLUR
	glAccum(GL_MULT,0.8);          // �������˼�ǰ��ۻ����������*0.8��д����ۻ����壬���ֵԽ��1��ģ����Խ��
	glAccum(GL_ACCUM,0.2);       // �����˼�ǰ�֡���������*0.2���뵽�ۻ������������ֵԽ�ӽ�1ģ��ԽС
	if(motion_blur){
		glAccum(GL_RETURN,1.0);
	}
#endif
	glFlush();
	glutSwapBuffers();                 //ǿ�ƻ�ͼ����ִ�л����ڻ�������������
}
void glutSpecial(int value, int x, int y)
{
	switch (value)
	{
	case GLUT_KEY_F1:        // ��F1��ʱ�л�����/ȫ��ģʽ
		if(isFullScreen)
		{
			glutReshapeWindow(GL_WIN_WIDTH, GL_WIN_HEIGHT);
			glutPositionWindow(GL_WIN_INITIAL_X, GL_WIN_INITIAL_Y);
			isFullScreen = false;
		}
		else
		{
			glutFullScreen();
			isFullScreen = true;
		}
		return;
	case GLUT_KEY_F2:        // ��F1��ʱ�л�����/ȫ��ģʽ
		if(line){
			glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		}
		else{
			glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
		}
		line = !line;
		return;
	case GLUT_KEY_F4:        // ��F1��ʱ�л�����/ȫ��ģʽ
		printf("F4 press\n");
		if(scale == 1){
			scale = 100;
		}
		else{
			scale = 1;
		}
		return;
	case GLUT_KEY_LEFT:
		g.TryTurnLeft();
		break;
	case GLUT_KEY_RIGHT:
		g.TryTurnRight();
		break;
	default:
		return;
	}
}

void glutKeyboard(unsigned char key,int x,int y)
{
	//printf("%c\n",key);
	switch(key){
	case 'j':
		if(gameState == READY){
			gameState = RUNNING;
			narudo.onAction(DASH);
		}
		break;
	case 'n':
		if(gameState == END){
			glutLeaveMainLoop();
		}
		break;
	case 'y':
		if(gameState == END){
			gameState = READY;
			resetData();
		}
		break;
	case 'q':
		glutLeaveMainLoop();
		break;
	case 'w':
		/*camera->startMotion(DOWN);
		if(narudo.getCurrentAction() == IDLE){
		narudo.onAction(DASH);
		}*/
		//printf("%f\n",MP);
		if(MP >= 10){
			camera->speed = 1;
			motion_blur = true;
			speed_up_time = 3;
			MP = 0;
		}
		else if(MP>=5&&MP<10){
			camera->speed = 1;
			motion_blur = true;
			speed_up_time = 1;
			MP = 0;
		}

		break;
	case 'a':
		//camera->startMotion(LEFT);
		moveInputManager.startMotion(LEFT,0.2f);
		break;
	case 'd':
		//camera->startMotion(RIGHT);
		moveInputManager.startMotion(RIGHT,0.2f);
		break;
		/*case 'm':
		motion_blur = !motion_blur;
		break;*/
	}
}

void glutKeyboardUp(unsigned char key,int x,int y)
{
	switch(key){
		/*case 'w':
		if(narudo.getCurrentAction() == DASH){
		narudo.onAction(IDLE);
		}
		camera->endMotion();
		break;
		case 's':
		if(narudo.getCurrentAction() == DASH){
		narudo.onAction(IDLE);
		}
		camera->endMotion();
		break;*/
	case 'a':
		//camera->endMotion();
		break;
	case 'd':
		//camera->endMotion();
		break;
	}
}

void glutTimer(int value) 
{ 
	update();
	glutPostRedisplay(); 
	glutTimerFunc(24,glutTimer,0); 
} 

void glutIdle(void)
{
	/*if(now_time-pre_time>0.03){

	pre_time = now_time;
	}*/
#ifdef KINECT
	updateKinect();
#endif
}

void initData()
{
	narudo.initFromFile("Data/naruto_2.mdl");
	//narudo.setPosition(Point(0,0,-1));
	narudo.setRotation(Point(-90,0,90));
	narudo.setScale(0.02f);
	/*if(narudo.getCurrentAction() == IDLE){
	narudo.onAction(DASH);
	}*/
	if (!pmodel) {
		pmodel = glmReadOBJ("obj/broadleavedtreeD.obj");
		if (!pmodel) glutLeaveMainLoop();
		glmScale(pmodel,2);
		glmUnitize(pmodel);
		glmFacetNormals(pmodel);
		glmVertexNormals(pmodel, 90.f,true);
		//glmScale(pmodel,1000);
	}

	//if (!bananaModel) {
	//       bananaModel = glmReadOBJ("obj/knife.obj");
	//       if (!bananaModel) glutLeaveMainLoop();
	//	//glmScale(bananaModel,2);
	//       glmUnitize(bananaModel);
	//       glmFacetNormals(bananaModel);
	//       glmVertexNormals(bananaModel, 90.f,true);
	//	//glmScale(pmodel,1000);
	//   }

	map = new Sand("Data/map.bmp","Data/barkD_texture.bmp",9,256,0.5,5);
	camera = new Camera(2.25, map->getHeight(2.25,-10)+2.0, -10,  2.25,  map->getHeight(2.25,-10)+1, -11,  90, 0);
	init_length = (int)camera->getPosition().z;
	TextureManager::Inst()->LoadTexture("Data/bg.bmp",BACKGROUND,GL_REPEAT,GL_LINEAR,GL_RGB);
	TextureManager::Inst()->LoadTexture("Data/Crate_24.bmp",CRATE,GL_REPEAT,GL_LINEAR,GL_RGB);
	TextureManager::Inst()->LoadTexture("Data/qi.png",QI,GL_REPEAT,GL_LINEAR,GL_RGBA);
	barrierManager = new BarrierManager(map);
	camera->onMotion(map);
	Point temp(camera->getPosition().x,map->getHeight(camera->getPosition().x,camera->getPosition().z)+0.4f,camera->getPosition().z);
	narudo.setPosition(temp);
	narudo.setRotation(Point(-90,0,180-camera->getAngle()));
	font.init("Data/Test.ttf", 24);
	#ifdef KINECT
	particle = new Particle(select_x,select_y,0,"Data/particle.png");
	#endif
}

void releaseData()
{
	glmDelete(pmodel);
	//glmDelete(bananaModel);
	font.clean();
	delete map;
	delete camera;
	delete barrierManager;
	#ifdef KINECT
	delete particle;
	#endif
}

void update()
{
	//particle->update(select_x,select_y,0);
#ifdef KINECT
	//updateKinect();
	particle->update(select_x,select_y,0);
	if(raise_hand&&gameState == READY){
		gameState = RUNNING;
		narudo.onAction(DASH);
	}
	current_x = getKinectHandPosition()->X;
	current_y = getKinectHandPosition()->Y;
	current_z = getKinectHandPosition()->Z;
	select_x -= (current_x-old_x)*current_z/1000*2*currentWidth/640;
	select_y -= (current_y-old_y)*current_z/1000*2*currentHeight/480;
	select_x = select_x<0?0:select_x;
	select_x = select_x>currentWidth?currentWidth:select_x;
	select_y = select_y<0?0:select_y;
	select_y = select_y>currentHeight?currentHeight:select_y;
	if(current_x-old_x<-DISTANCE&&old_x!=0){
		moveInputManager.startMotion(RIGHT,0.2f);
	}
	else if(current_x-old_x>DISTANCE&&old_x!=0){
		moveInputManager.startMotion(LEFT,0.2f);
	}
	else if(current_z-old_z<-DISTANCE*4&&old_z!=0){
		if(MP >= 10){
			camera->speed = 1;
			motion_blur = true;
			speed_up_time = 3;
			MP = 0;
		}
		else if(MP>=5&&MP<10){
			camera->speed = 1;
			motion_blur = true;
			speed_up_time = 1;
			MP = 0;
		}
	}
	old_x = current_x;
	old_y = current_y;
	old_z = current_z;
#endif
	//now_time += 0.024f;
	if(motion_blur){
		speed_up_time -= 0.024f;
		if(speed_up_time <= 0){
			motion_blur = false;
			camera->speed = 0.5f;
			speed_up_time = 0;
		}
	}
	switch(gameState){
	case READY:
		onReady();
		break;
	case RUNNING:
		onRunning();
		break;
	case END:
		onEnd();
		break;
	case HIT:
		onHit();
		break;
	}

}

void onRunning()
{
	current_time += 1.f;
	moveInputManager.update(0.024f);
	camera->startMotion(UP);
	camera->onMotion(map);
	camera->startMotion(moveInputManager.getMotion());
	camera->onMotion(map);
	Point temp(camera->getPosition().x,map->getHeight(camera->getPosition().x,camera->getPosition().z)+0.4f,camera->getPosition().z);
	narudo.setPosition(temp);
	narudo.setRotation(Point(-90,0,180-camera->getAngle()));
	if(camera->getPosition().z<=-1270){
		//temp.y -= 0.2;
		//narudo.setPosition(temp);
		narudo.onAction(SUCCESS);
		gameState = END;
	}
	if(barrierManager->detectCollision(camera)){
		gameState = HIT;
		narudo.onAction(HIT_BY_TREE);
	}
	MP += 0.001f;
	if(barrierManager->detectBonus(camera)){
		MP += 5;
		if(MP>10){
			MP = 10;
		}
	}
}

void onReady()
{
	//current_time += 1.f;
}

void onEnd()
{

}

void onHit()
{
	current_time += 1.f;
	camera->startMotion(DOWN);
	camera->onMotion(map);
	Point temp(camera->getPosition().x,map->getHeight(camera->getPosition().x,camera->getPosition().z)+0.4f,camera->getPosition().z);
	narudo.setPosition(temp);
	narudo.setRotation(Point(-90,0,180-camera->getAngle()));
	hit_time += 0.024f;
	if(hit_time>=0.5f){
		narudo.onAction(DASH);
		gameState = RUNNING;
		hit_time = 0.f;
	}
}


void resetData()
{
	camera->setData(2.25, map->getHeight(2.25,-10)+2.0, -10,  2.25,  map->getHeight(2.25,-10)+1, -11,  90, 0);
	camera->onMotion(map);
	Point temp(camera->getPosition().x,map->getHeight(camera->getPosition().x,camera->getPosition().z)+0.4f,camera->getPosition().z);
	narudo.setPosition(temp);
	narudo.setRotation(Point(-90,0,180-camera->getAngle()));
	narudo.onAction(IDLE);
	current_time = 0.f;
	/*pre_time = 0.f;
	now_time = 0.f;*/
}

 void pushScreenCoordinateMatrix() {
	glPushAttrib(GL_TRANSFORM_BIT);
	GLint	viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(viewport[0],viewport[2],viewport[1],viewport[3]);
	glPopAttrib();
}

/// Pops the projection matrix without changing the current
/// MatrixMode.
void pop_projection_matrix() {
	glPushAttrib(GL_TRANSFORM_BIT);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glPopAttrib();
}





/************************************************************************/
/*Brief:  Test all kinds of Textures in OpenGL                          */
/*Author: tiny                                                          */
/*Date:   09/29/2008                                                    */
/************************************************************************/
//#include "TextureManager.h"
//#include <GL/glut.h>
//
////ȫ����ͼID
//GLuint texture[1];
//
//void init()
//{
//	//��2D��ͼ״̬��
//	glEnable( GL_TEXTURE_2D );
//
//	//������ͼ����
//	//������ش�·�����ļ����ѡ��.\\�����ĸ�ʽ
//	TextureManager::Inst()->LoadTexture( "Data/bg.bmp", texture[0], GL_REPEAT, GL_LINEAR, GL_RGB ); 
//
//	//���Թ���һ��Ҫ�ŵ���������ĺ���
//	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);    // �����˲�
//	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);    // �����˲�
//
//	glClearColor( 0.5, 0.5, 0.5, 0.5 );
//}
//
//void display()
//{
//	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
//
//	//������
//	TextureManager::Inst()->BindTexture( texture[0] );
//
//	//��Ⱦ
//	glBegin( GL_QUADS );
//	glTexCoord2d( 0, 0 ); glVertex3f( -5.0f, -5.0f, 0.0f );
//	glTexCoord2d( 0, 1 ); glVertex3f( -5.0f, 5.0f, 0.0f );
//	glTexCoord2d( 1, 1 ); glVertex3f( 5.0f, 5.0f, 0.0f );
//	glTexCoord2d( 1, 0 ); glVertex3f( 5.0f, -5.0f, 0.0f );
//	glEnd();
//	glFlush();
//
//	glutSwapBuffers();
//}
//
//void reshape( int w, int h )
//{
//	glViewport( 0, 0, GLsizei( w ), GLsizei( h ) );
//	glMatrixMode( GL_PROJECTION );
//	glLoadIdentity();
//	gluPerspective( 45, ( GLdouble ) w / ( GLdouble ) h, 1.0f, 1000.0f );
//	glMatrixMode( GL_MODELVIEW );
//	glLoadIdentity();
//	gluLookAt( 0, 0, 20, 0, 0, 0, 0, 1, 0 );
//}
//
//void keyboard( unsigned char key, int x, int y )
//{
//	if ( key == 27 )
//	{
//		//�ͷŵ���ͼ����ֹ�ڴ�й¶
//		TextureManager::Inst()->UnloadTexture( texture[0] );
//		exit( 0 );
//	}
//
//}
//
//
//int main( int argc, char *argv[] )
//{
//	glutInit( &argc, argv );
//	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH );
//	glutInitWindowPosition( 300, 300 );
//	glutInitWindowSize( 400, 300 );
//	glutCreateWindow( "OpenGL Texture Test" );
//	init();
//	glutReshapeFunc( reshape );
//	glutKeyboardFunc( keyboard );
//	glutDisplayFunc( display );
//	glutMainLoop();
//
//	return 0;
//}