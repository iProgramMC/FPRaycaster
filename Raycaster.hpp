#pragma once
#include "olcPixelGameEngine.h"
#include <Windows.h> // for SetCursorPos and other shit

#define MAPWIDTH 32
#define MAPHEIGHT 32
#define PI 3.1415926

extern double playerX, playerY, playerAngle, FOV, farPlane, playerRange;
extern double* depthBuffer;
extern int mapWidth, mapHeight;
// include end character too
extern char map[];

char GetTile(int x, int y);
void SetTile(int x, int y, char c);
bool CanRemove(char c);
bool GetCollisionType(char c);
double sample(int min, int max, double degree);

// Override base class with your custom functionality
class Raycaster : public olc::PixelGameEngine
{
	// sprites for raycaster
	olc::Sprite* wall_sprite;
	olc::Sprite* tree_sprite;
	olc::Sprite* bush_sprite;
	olc::Sprite* _box_sprite;
	olc::Sprite* _unk_sprite;
	olc::Sprite* dirt_sprite;
	olc::Sprite* xhairSprite;
	olc::Sprite* brockSprite;
public:
	Raycaster();
	~Raycaster();

public:
	bool OnUserCreate() override;
	int rtViewportW, rtViewportH;
	int rtViewportX = 0, rtViewportY = 0;
	void DrawBillboarded(olc::Sprite* spr, double x, double y);
	bool OnUserUpdate(float fElapsedTime) override;
};
