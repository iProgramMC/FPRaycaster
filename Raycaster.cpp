#define OLC_PGE_APPLICATION
#include "Raycaster.hpp"
double playerX = 8.0, playerY = 8.0, playerAngle = 0.0;
double FOV = PI / 3.5;
double farPlane = mapHeight * 1.414; // approx. sqrt(2)
double playerRange = 8.0;

double* depthBuffer = nullptr;

int mapWidth = MAPWIDTH, mapHeight = MAPHEIGHT;
// include end character too
char map[MAPWIDTH * MAPHEIGHT + 1] =
"HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH"
"H..............................H"
"H.........................X....H"
"H.............X................H"
"H..............................H"
"H....X.........................H"
"H...............BBBB...........H"
"H..........................X...H"
"H..........#...................H"
"H.........##...................H"
"H...................X..........H"
"H.####.........................H"
"H........................X.....H"
"H...........X..................H"
"H..............................H"
"H#####....#####################H"
"H..............................H"
"H..............................H"
"H..............................H"
"H..............................H"
"H..............................H"
"H..............................H"
"H..............................H"
"H..............................H"
"H..............................H"
"H..............................H"
"H..............................H"
"H..............................H"
"H..............................H"
"H..............................H"
"H..............................H"
"HHHHHH....HHHHHHHHHHHHHHHHHHHHHH";

char GetTile(int x, int y) {
	if (x < 0 || x >= mapWidth || y < 0 || y >= mapHeight)
		return ' '; // oob
	return map[y * mapWidth + x];
}

void SetTile(int x, int y, char c) {
	if (x < 0 || x >= mapWidth || y < 0 || y >= mapHeight) return;
	map[y * mapWidth + x] = c;
}
bool CanRemove(char c) {
	return c != 'H';
}

bool GetCollisionType(char c) {
	switch (c) {
	case 'X':
	case '.':
		return false;
	}
	return true;
}

double sample(int min, int max, double degree) {
	return min + (max - min) * degree;
}

Raycaster::Raycaster()
{
	// Name you application
	sAppName = "Raycaster Test";
}
Raycaster::~Raycaster() 
{
	delete wall_sprite;
	delete tree_sprite;
	delete _box_sprite;
	delete bush_sprite;
	delete _unk_sprite;
	delete dirt_sprite;
	delete brockSprite;
	delete xhairSprite;
	delete[] depthBuffer;
}
bool Raycaster::OnUserCreate() {

	wall_sprite = new olc::Sprite("bricks.png");
	tree_sprite = new olc::Sprite("tree.png");
	_box_sprite = new olc::Sprite("crate.png");
	bush_sprite = new olc::Sprite("bush.png");
	_unk_sprite = new olc::Sprite("unk.png");
	dirt_sprite = new olc::Sprite("dirt.png");
	brockSprite = new olc::Sprite("bedrock.png");

	xhairSprite = new olc::Sprite("crosshair.png");

	depthBuffer = new double[ScreenWidth()];
	memset(depthBuffer, 0, ScreenWidth() * sizeof(double)); // always make sure its 0

	rtViewportW = ScreenWidth();
	rtViewportH = ScreenHeight();
	return true;
}


void Raycaster::DrawBillboarded(olc::Sprite* spr, double x, double y) {
	if (!spr) return;
	double vecX = x - playerX;
	double vecY = y - playerY;
	double distPlayer = (double)sqrtf((float)(vecX * vecX + vecY * vecY));

	double eyeX = (double)sinf((float)playerAngle);
	double eyeY = (double)cosf((float)playerAngle);

	double objectAngle = atan2f((float)eyeY, (float)eyeX) - atan2f((float)vecY, (float)vecX);
	if (objectAngle < -PI) objectAngle += 2 * PI;
	if (objectAngle > PI) objectAngle -= 2 * PI;

	bool inPlayerFov = fabs((float)objectAngle) < (FOV) / 2;
	if (!inPlayerFov || distPlayer < 0.5 || distPlayer >= farPlane) return;

	int swidth = rtViewportW;
	int sheight = rtViewportH;

	double objCeiling = (double)(sheight / 2.0) - sheight / ((double)distPlayer);
	double objFloor = sheight - objCeiling;
	double objHeight = objFloor - objCeiling;
	double objAspRatio = (double)spr->height / (double)spr->width;
	double objWidth = objHeight / objAspRatio;
	double midObject = (0.5 * (objectAngle / (FOV / 2.0)) + 0.5) * (double)swidth;
	for (double lx = 0; lx < objWidth; lx++) {
		for (double ly = 0; ly < objHeight; ly++) {
			float sampleX = (float)(lx / objWidth);
			float sampleY = (float)(ly / objHeight);
			olc::Pixel p = spr->Sample(sampleX, sampleY);
			int objColumn = (int)(midObject + lx - (objWidth / 2.0));
			if (objColumn < 0 || objColumn >= swidth) continue;
			if (objCeiling + ly < 0 || objCeiling + ly >= sheight) continue;
			if (depthBuffer[objColumn] < distPlayer) continue;
			Draw(rtViewportX + objColumn, rtViewportY + (int)objCeiling + (int)ly, p);
			depthBuffer[objColumn] = distPlayer;
		}
	}
}


bool Raycaster::OnUserUpdate(float fElapsedTime)
{
	int sWidth = rtViewportW;  // it cant possibly change during the frame :)
	int sHeight = rtViewportH;
	SetCursor(NULL);
	if (IsFocused()) {
		int centerX = sWidth / 2 + 34, centerY = sHeight / 2 + 57;
		int deltaX = GetMouseX() - sWidth / 2;
		int deltaY = GetMouseY() - sHeight / 2;
		SetCursorPos(centerX, centerY);

		playerAngle += 0.2 * (double)deltaX * fElapsedTime;

		if (playerAngle < 0) playerAngle += 2 * PI;
		if (playerAngle >= 2 * PI) playerAngle -= 2 * PI;

		bool ctrlHeld = GetKey(olc::CTRL).bHeld;
		if (GetKey(olc::W).bHeld)
		{
			double d = fElapsedTime * 5.0 * (ctrlHeld ? 2.0 : 1.0);
			double mx = sinf((float)playerAngle) * d;
			double my = cosf((float)playerAngle) * d;
			playerX += mx;
			playerY += my;
			if (GetCollisionType(GetTile((int)playerX, (int)playerY)) != 0) {
				playerX -= mx;
				playerY -= my;
			}
		}
		if (GetKey(olc::S).bHeld)
		{
			double d = fElapsedTime * 5.0;
			double mx = sinf((float)playerAngle) * d;
			double my = cosf((float)playerAngle) * d;
			playerX -= mx;
			playerY -= my;
			if (GetCollisionType(GetTile((int)playerX, (int)playerY)) != 0) {
				playerX += mx;
				playerY += my;
			}
		}
		if (GetKey(olc::A).bHeld)
		{
			double d = fElapsedTime * 5.0;
			double mx = sinf((float)playerAngle) * d;
			double my = cosf((float)playerAngle) * d;
			playerX -= my;
			playerY += mx;
			if (GetCollisionType(GetTile((int)playerX, (int)playerY)) != 0) {
				playerX += my;
				playerY -= mx;
			}
		}
		if (GetKey(olc::D).bHeld)
		{
			double d = fElapsedTime * 5.0;
			double mx = sinf((float)playerAngle) * d;
			double my = cosf((float)playerAngle) * d;
			playerX += my;
			playerY -= mx;
			if (GetCollisionType(GetTile((int)playerX, (int)playerY)) != 0) {
				playerX -= my;
				playerY += mx;
			}
		}
	}

	for (int x = 0; x < sWidth; x++)
	{
		double rayAngle = (playerAngle - FOV / 2) + ((double)x / (double)sWidth) * FOV;
		double distToWall = 0.0;
		bool hitWall = false;
		char wallHit = '.';
		double eyeX = sinf((float)rayAngle), eyeY = cosf((float)rayAngle);
		bool boundary = false;
		double sampleX = 0.0;
		while (!hitWall) {
			distToWall += 0.01;
			int testX = (int)(playerX + eyeX * distToWall);
			int testY = (int)(playerY + eyeY * distToWall);
			char tileGot = GetTile(testX, testY);
			if (GetCollisionType(tileGot) != 0) {
				hitWall = true;
				wallHit = tileGot;

				double blockMidX = (double)testX + 0.5;
				double blockMidY = (double)testY + 0.5;

				double testPointX = playerX + eyeX * distToWall;
				double testPointY = playerY + eyeY * distToWall;

				double testAngle = atan2f((float)(testPointY - blockMidY), (float)(testPointX - blockMidX));

				if (testAngle >= -PI * 0.25 && testAngle < PI * 0.25)
					sampleX = testPointY - (double)testY;
				if (testAngle >= PI * 0.25 && testAngle < PI * 0.75)
					sampleX = testPointX - (double)testX;
				if (testAngle < -PI * 0.25 && testAngle >= -PI * 0.75)
					sampleX = testPointX - (double)testX;
				if (testAngle >= PI * 0.75 || testAngle < -PI * 0.75)
					sampleX = testPointY - (double)testY;
			}
		}

		double rd = playerAngle - rayAngle;
		if (rd < 0)       rd += 2 * PI;
		if (rd >= 2 * PI) rd -= 2 * PI;
		distToWall *= cosf((float)rd);

		int ceiling = (int)((double)(sHeight / 2.0f) - sHeight / ((double)distToWall));
		int floor = sHeight - ceiling;

		depthBuffer[x] = distToWall;

		int ceiling2 = sHeight / 2;
		int floor2 = sHeight - ceiling2;
		for (int y = 0; y < sHeight; y++) {

			if (y < ceiling2) {
				int s = (int)sample(190, 255, (double)y / (double)sHeight * 2);
				if (s < 0) s = 0;
				Draw(rtViewportX + x, rtViewportY + y, olc::Pixel((int)((double)s * 0.25), (int)((double)s * 0.75), s, 255));
			}
			else {
				int s = (int)sample(190, 255, (double)(sHeight / 2 - y) / (double)sHeight * 2);
				if (s < 0) s = 0;
				Draw(rtViewportX + x, rtViewportY + y, olc::Pixel(0, s, 0, 255));
			}

			if (y >= ceiling && y <= floor) {
				if (distToWall < farPlane) {
					double sampleY = ((double)y - (double)ceiling) / ((double)floor - (double)ceiling);
					olc::Sprite* spr = _unk_sprite;
					switch (wallHit) {
					case '#':
						spr = wall_sprite;
						break;
					case 'B':
						spr = _box_sprite;
						break;
					case ' ':
						spr = bush_sprite;
						break;
					case 'M':
						spr = dirt_sprite;
						break;
					case 'H':
						spr = brockSprite;
						break;
					}
					Draw(rtViewportX + x, rtViewportY + y, spr->Sample((float)sampleX, (float)sampleY));
				}
				else {
					Draw(rtViewportX + x, rtViewportY + y, olc::BLACK);
				}
			}
		}
	}

	// Draw objects
	for (int y = 0; y < mapHeight; y++) {
		for (int x = 0; x < mapWidth; x++) {
			if (GetTile(x, y) == 'X') {
				DrawBillboarded(tree_sprite, x + 0.5, y + 0.5);
			}
		}
	}

	// Draw crosshair
	int xhairx = ((ScreenWidth() - 32) / 2), xhairy = ((ScreenHeight() - 32) / 2);
	olc::vd2d xHairVec = { (double)xhairx, (double)xhairy };
	DrawSprite(xHairVec, xhairSprite);
	return true;
}
