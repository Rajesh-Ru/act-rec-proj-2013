/**
 * ���ߣ����ٺ�
 */
#ifndef CONFIG_H
#define CONFIG_H
enum Motion{UP,DOWN,LEFT,RIGHT,SPEED_UP,SPEED_DOWN,NONE};
enum Action{IDLE = 1,DASH = 9,HIT_BY_TREE = 76,SUCCESS = 2};
enum TextureID{MAP,BACKGROUND,READY_TEXTURE,GO_TEXTURE,PARTICLE,QI,CRATE};
enum GameState{READY,RUNNING,END,HIT};
#endif