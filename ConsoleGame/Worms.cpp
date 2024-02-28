#include <iostream>
#include <string>
#include <algorithm>

using namespace std;

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

// Port DrawWireFrameModel function from Console Game Engine
void DrawWireFrameModel(olc::PixelGameEngine* engine, const vector<pair<float, float>>& vecModelCoordinates,
	float x, float y, float r = 0.0f, float s = 1.0f, olc::Pixel col = olc::WHITE)
{
	// vecModelCoordinates : the wire frame model
	// x, y : the screenposition where to draw it
	// r : angle of rotation
	// s : scaling factor

	// Create translated model vector of coordinate pairs
	vector<pair<float, float>> vecTransformedCoordinates;		// pair.first : x coordinate, pair.second : y coordinate
	int verts = vecModelCoordinates.size();		// Keep the model vector static
	vecTransformedCoordinates.resize(verts);		// Create another vector the same size as the model vector

	// Rotate
	for (int i = 0; i < verts; i++)
	{
		vecTransformedCoordinates[i].first = vecModelCoordinates[i].first * cosf(r) - vecModelCoordinates[i].second * sinf(r);
		vecTransformedCoordinates[i].second = vecModelCoordinates[i].first * sinf(r) + vecModelCoordinates[i].second * cosf(r);
	}

	// Scale
	for (int i = 0; i < verts; i++)
	{
		vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first * s;
		vecTransformedCoordinates[i].second = vecTransformedCoordinates[i].second * s;
	}

	// Translate
	for (int i = 0; i < verts; i++)
	{
		vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first + x;
		vecTransformedCoordinates[i].second = vecTransformedCoordinates[i].second + y;
	}

	// Draw closed polygon
	for (int i = 0; i < verts + 1; i++)
	{
		int j = (i + 1);
		engine->DrawLine(vecTransformedCoordinates[i % verts].first, vecTransformedCoordinates[i % verts].second,
			vecTransformedCoordinates[j % verts].first, vecTransformedCoordinates[j % verts].second, col);
	}
}

// Physics engine
class cPhysicsObject
{
public:
	// Position
	float px = 0.0f;
	float py = 0.0f;
	// Velocity
	float vx = 0.0f;
	float vy = 0.0f;
	// Acceleration
	float ax = 0.0f;
	float ay = 0.0f;

	float radius = 4.0f;		// Represents collision boundary of an object
	bool bStable = false;		// Represents whether object is stable/stopped moving
	float fFriction = 0.8f;		// Represents the dampening factor for an object's collision

	int nBounceBeforeDeath = -1;		// Represents number of times an object can bounce before 'dying'; -1 means infinite bounces
	bool bDead = false;					// Represents indicator to check if object should be removed

	// Default constructor that sets position
	cPhysicsObject(float x = 0.0f, float y = 0.0f)
	{
		px = x;
		py = y;
	}

	// Makes the class abstract
	virtual void Draw(olc::PixelGameEngine* engine, float fOffsetX, float fOffsetY) = 0;
	virtual int BounceDeathAction() = 0;
};

class cDummy : public cPhysicsObject
{
public:
	cDummy(float x = 0.0f, float y = 0.0f) : cPhysicsObject(x, y)
	{

	}

	virtual void Draw(olc::PixelGameEngine* engine, float fOffsetX, float fOffsetY)
	{
		DrawWireFrameModel(engine, vecModel, px - fOffsetX, py - fOffsetY, atan2f(vy, vx), radius, olc::WHITE);
		// vecModel : Drawn model data
		// p - fOffset :  (x,y) Coordinates
		// atan2f() : Angle object is rotated
		// radius : Scales object's size
		// olc::WHITE : Makes characters white
	}

	virtual int BounceDeathAction()
	{
		return 0;		// Does nothing, just fades away
	}

private:
	static vector<pair<float, float>> vecModel;		// Allows one model to be shared across all objects of the same class
};

vector<pair<float, float>> DefineDummy()		// Creates a unit circle with a line fom center to edge
{
	vector<pair<float, float>> vecModel;
	vecModel.push_back({ 0.0f, 0.0f });

	for (int i = 0; i < 10; i++)
		vecModel.push_back({ cosf(i / 9.0f * 2.0f * 3.14159f), sinf(i / 9.0f * 2.0f * 3.14159f) });

	return vecModel;
}
vector<pair<float, float>> cDummy::vecModel = DefineDummy();

class cDebris : public cPhysicsObject // a small rock that bounces
{
public:
	cDebris(float x = 0.0f, float y = 0.0f) : cPhysicsObject(x, y)
	{
		// Sets velocity to random direction and size to make a "boom" effect
		vx = 10.0f * cosf(((float)rand() / (float)RAND_MAX) * 2.0f * 3.14159f);
		vy = 10.0f * sinf(((float)rand() / (float)RAND_MAX) * 2.0f * 3.14159f);
		radius = 1.0f;
		fFriction = 0.8f;
		nBounceBeforeDeath = 5;
	}

	virtual void Draw(olc::PixelGameEngine* engine, float fOffsetX, float fOffsetY)
	{
		DrawWireFrameModel(engine, vecModel, px - fOffsetX, py - fOffsetY, atan2f(vy, vx), radius, olc::DARK_GREEN);
	}

	virtual int BounceDeathAction()
	{
		return 0;		// Does nothing, just fades away
	}

private:
	static vector<pair<float, float>> vecModel;
};

vector<pair<float, float>> DefineDebris()
{
	// A small unit rectangle
	vector<pair<float, float>> vecModel;
	vecModel.push_back({ 0.0f, 0.0f });
	vecModel.push_back({ 1.0f, 0.0f });
	vecModel.push_back({ 1.0f, 1.0f });
	vecModel.push_back({ 0.0f, 1.0f });
	return vecModel;
}
vector<pair<float, float>> cDebris::vecModel = DefineDebris();

class cMissile : public cPhysicsObject // A projectile weapon
{
public:
	cMissile(float x = 0.0f, float y = 0.0f, float _vx = 0.0f, float _vy = 0.0f) : cPhysicsObject(x, y)
	{
		radius = 2.5f;
		fFriction = 0.5f;
		vx = _vx;
		vy = _vy;
		bDead = false;
		nBounceBeforeDeath = 1;
	}

	virtual void Draw(olc::PixelGameEngine* engine, float fOffsetX, float fOffsetY)
	{
		DrawWireFrameModel(engine, vecModel, px - fOffsetX, py - fOffsetY, atan2f(vy, vx), radius, olc::YELLOW);
	}

	virtual int BounceDeathAction()
	{
		return 20;		// Gives the Boom function a radius of 20 to make big explosions
	}

private:
	static vector<pair<float, float>> vecModel;
};

vector<pair<float, float>> DefineMissile()
{
	// Defines a rocket like shape
	vector<pair<float, float>> vecModel;
	vecModel.push_back({ 0.0f, 0.0f });
	vecModel.push_back({ 1.0f, 1.0f });
	vecModel.push_back({ 2.0f, 1.0f });
	vecModel.push_back({ 2.5f, 0.0f });
	vecModel.push_back({ 2.0f, -1.0f });
	vecModel.push_back({ 1.0f, -1.0f });
	vecModel.push_back({ 0.0f, 0.0f });
	vecModel.push_back({ -1.0f, -1.0f });
	vecModel.push_back({ -2.5f, -1.0f });
	vecModel.push_back({ -2.0f, 0.0f });
	vecModel.push_back({ -2.5f, 1.0f });
	vecModel.push_back({ -1.0f, 1.0f });
	
	for (auto& v : vecModel)		// Scales points to make the shape unit sized
	{
		v.first /= 2.5f;
		v.second /= 2.5f;
	}
	return vecModel;
}
vector<pair<float, float>> cMissile::vecModel = DefineMissile();

class cWorm : public cPhysicsObject		// A unit, aka a Worm
{
public:
	cWorm(float x = 0.0f, float y = 0.0f) : cPhysicsObject(x, y)
	{
		radius = 3.5f;
		fFriction = 0.2f;
		bDead = false;
		nBounceBeforeDeath = -1;
		
		if (sprWorm == nullptr)		// Loads sprite data from sprite file
			sprWorm = new olc::Sprite("Sprites/worms.png");
	}

	virtual void Draw(olc::PixelGameEngine* engine, float fOffsetX, float fOffsetY)
	{
		engine->SetPixelMode(olc::Pixel::Mode::MASK);
		engine->DrawPartialSprite(px - fOffsetX - radius, py - fOffsetY - radius, sprWorm, 0, 0, 8, 8);
		engine->SetPixelMode(olc::Pixel::Mode::NORMAL);
	}

	virtual int BounceDeathAction()
	{
		return 0;		// Nothing
	}

private:
	static olc::Sprite* sprWorm;
};

olc::Sprite* cWorm::sprWorm = nullptr;

class Worms : public olc::PixelGameEngine
{
public:
	Worms()
	{
		sAppName = "Worms";
	}

private:
	// For map size
	int nMapWidth = 1024;
	int nMapHeight = 512;
	unsigned char* map = nullptr;

	// For camera control
	float fCameraPosX = 0.0f;
	float fCameraPosY = 0.0f;

	list<unique_ptr<cPhysicsObject>> listObjects;		// Allows multiple types of objects in list

	virtual bool OnUserCreate()
	{
		// Create the map
		map = new unsigned char[nMapWidth * nMapHeight];		// Allocate memory for 2D array
		memset(map, 0, nMapWidth * nMapHeight * sizeof(unsigned char));		// Clear all to 0
		CreateMap();

		return true;
	}

	virtual bool OnUserUpdate(float fElapsedTime)
	{
		if (GetKey(olc::Key::M).bReleased)		// Whenever 'M' key is released, generate new map
			CreateMap();

		if (GetMouse(0).bReleased)		// Lanches debris wherever the left mouse button is released
			Boom(GetMouseX() + fCameraPosX, GetMouseY() + fCameraPosY, 10.0f);
		
		if (GetMouse(1).bReleased)		// Drops a missile wherever the right mouse button is released
			listObjects.push_back(unique_ptr<cMissile>(new cMissile(GetMouseX() + fCameraPosX, GetMouseY() + fCameraPosY)));

		if (GetMouse(2).bReleased)		// Creates a Worm/unit object wherever the middle mouse button is released
			listObjects.push_back(unique_ptr<cWorm>(new cWorm(GetMouseX() + fCameraPosX, GetMouseY() + fCameraPosY)));

		// Controller camera for mouse edge map scrolling
		float fMapScrollSpeed = 400.0f;
		if (GetMouseX() < 5)		// If within 5 pixels on screen edge, move camera position
			fCameraPosX -= fMapScrollSpeed * fElapsedTime;
		if (GetMouseX() > ScreenWidth() - 5)
			fCameraPosX += fMapScrollSpeed * fElapsedTime;
		if (GetMouseY() < 5)
			fCameraPosY -= fMapScrollSpeed * fElapsedTime;
		if (GetMouseY() > ScreenWidth() - 5)
			fCameraPosY += fMapScrollSpeed * fElapsedTime;

		// Clamp map boundaries to keep camera in bounds
		if (fCameraPosX < 0)
			fCameraPosX = 0;
		if (fCameraPosX >= nMapWidth - ScreenWidth())
			fCameraPosX = nMapWidth - ScreenWidth();
		if (fCameraPosY < 0)
			fCameraPosY = 0;
		if (fCameraPosY >= nMapHeight - ScreenHeight())
			fCameraPosY = nMapHeight - ScreenHeight();

		for (int z = 0; z < 10; z++)
		{
			for (auto& p : listObjects)		// Updates physics of all physical objects
			{
				// Applies gravity
				p->ay += 2.0f;

				// Updates Velocity
				p->vx += p->ax * fElapsedTime;
				p->vy += p->ay * fElapsedTime;

				// Updates potential future position
				float fPotentialX = p->px + p->vx * fElapsedTime;
				float fPotentialY = p->py + p->vy * fElapsedTime;

				// Resets acceleration and stability
				p->ax - 0.0f;
				p->ay = 0.0f;
				p->bStable = false;

				// Checks colision with the map 
				float fAngle = atan2f(p->vy, p->vx);		
				float fResponseX = 0;
				float fResponseY = 0;
				bool bCollision = false;

				// Iterates though a semicircle of an object's radius that's rotated towards the direction of travel
				for (float r = fAngle - 3.14159f / 2.0f; r < fAngle + 3.14159f / 2.0f; r += 3.14159f / 8.0f)
				{
					// Calculates the test point on circumference of circle
					float fTestPosX = (p->radius) * cosf(r) + fPotentialX;
					float fTestPosY = (p->radius) * sinf(r) + fPotentialY;

					// Constrains to test within the map's boundary
					if (fTestPosX >= nMapWidth) fTestPosX = nMapWidth - 1;
					if (fTestPosY >= nMapHeight) fTestPosX = nMapHeight - 1;
					if (fTestPosX < 0) fTestPosX = 0;
					if (fTestPosY < 0) fTestPosY = 0;

					// Tests if any of the points on an object's semicircle intersects with the terrian
					if (map[(int)fTestPosY * nMapWidth + (int)fTestPosX] != 0)
					{
						// Accumulates collision points to define the normal vector for escape response
						fResponseX += fPotentialX - fTestPosX;
						fResponseY += fPotentialY - fTestPosY;
						bCollision = true;

					}
				}

					// Calculates magnitudes of response and velocity vectors
					float fMagVelocity = sqrtf(p->vx * p->vx + p->vy * p->vy);
					float fMagResponse = sqrtf(fResponseX * fResponseX + fResponseY * fResponseY);

					// Find angle of collision
					if (bCollision)		// If collision has occured, respond
					{
						p->bStable = true;
												
						// Calculates reflection vector of objects velocity vector, using response vector as normal
						float dot = p->vx * (fResponseX / fMagResponse) + p->vy * (fResponseY / fMagResponse);

						// Uses the friction coefficient to dampen response (approximates energy loss)
						p->vx = p->fFriction * (-2.0f * dot * (fResponseX / fMagResponse) + p->vx);
						p->vy = p->fFriction * (-2.0f * dot * (fResponseY / fMagResponse) + p->vy);

						if (p->nBounceBeforeDeath > 0)		// Makes some objects 'die' after several bounces
						{
							p->nBounceBeforeDeath--;
							p->bDead = p->nBounceBeforeDeath == 0;

							if (p->bDead)		// Action upon an objects death; If greater than 0 creates an explosion
							{
								int nResponse = p->BounceDeathAction();
								if (nResponse > 0)
									Boom(p->px, p->py, nResponse);
							}

						}
					}
					else		// Else allow it to use the new potential positions
					{
						// Updates objects position with potential x,y coordinates
						p->px = fPotentialX;
						p->py = fPotentialY;
					}

					// Makes objects stop moving when velocity is low
					if (fMagVelocity < 0.1f)
						p->bStable = true;
			}
			// Removes objects from list if dead flag is true; Because it is a unique ptr, will go out of scope and automatically delete
			listObjects.remove_if([](unique_ptr<cPhysicsObject>& o) {return o->bDead;});
		}

		// Draws landscape terrain
		for (int x = 0; x < ScreenWidth(); x++)		// Iterate through all pixels on screen
			for (int y = 0; y < ScreenHeight(); y++)
			{
				switch (map[(y + (int)fCameraPosY) * nMapWidth + (x + (int)fCameraPosX)])		// Find location in map & color in for sky or terrain
				{
				case 0:
					Draw(x, y, olc::CYAN);
					break;
				case 1:
					Draw(x, y, olc::DARK_GREEN);
					break;
				}
			}
		
		for (auto& p : listObjects)		// Draws Objects
			p->Draw(this, fCameraPosX, fCameraPosY);

		return true;
	}

	void Boom(float fWorldX, float fWorldY, float fRadius)		// Launches debris
	{
		auto CircleBresenham = [&](int xc, int yc, int r)		// Bresenham's midpoint circle algorithm sourced from wikipedia
		{
			int x = 0;
			int y = r;
			int p = 3 - 2 * r;
			if (!r) return;

			auto drawline = [&](int sx, int ex, int ny)
			{
				for (int i = sx; i < ex; i++)
					if (ny >= 0 && ny < nMapHeight && i >= 0 && i < nMapWidth)
						map[ny * nMapWidth + i] = 0;
			};

			while (y >= x)		// Only makes 1/8 of the circle
			{
				// Modified to draw scan-lines instead of edges
				drawline(xc - x, xc + x, yc - y);
				drawline(xc - y, xc + y, yc - x);
				drawline(xc - x, xc + x, yc + y);
				drawline(xc - y, xc + y, yc + x);
				if (p < 0) p += 4 * x++ + 6;
				else p += 4 * (x++ - y--) + 10;
			}
		};

		CircleBresenham(fWorldX, fWorldY, fRadius);		// Erases terrain to form a crater

		for (auto& p : listObjects)		// Knocks back other objects in range using pythagorean theorem
		{
			float dx = p->px - fWorldX;
			float dy = p->py - fWorldY;
			float fDist = sqrt(dx * dx + dy * dy);

			if (fDist < 0.0001f) fDist = 0.0001f;		// Prevents possible division by zero

			if (fDist < fRadius)		// Closer objects to explosion get bigger boost
			{
				p->vx = (dx / fDist) * fRadius;
				p->vy = (dy / fDist) * fRadius;
				p->bStable = false;
			}
		}


		for (int i = 0; i < (int)fRadius; i++)		// Radius allows big explosions to make lots of debris and small ones to make fewer
			listObjects.push_back(unique_ptr<cDebris>(new cDebris(fWorldX, fWorldY)));

	}


	void CreateMap()
	{
		// 1D Perlin noise generation
		float* fSurface = new float[nMapWidth];
		float* fNoiseSeed = new float[nMapWidth];

		for (int i = 0; i < nMapWidth; i++)		// Generates noise for map generation
			fNoiseSeed[i] = (float)rand() / (float)RAND_MAX;

		fNoiseSeed[0] = 0.5f;		// Terrain will start & end halfway up screen
		PerlinNoise1D(nMapWidth, fNoiseSeed, 8, 2.0f, fSurface);

		for (int x = 0; x < nMapWidth; x++)		// Scroll through all elements in map & compare with surface array heights
			for (int y = 0; y < nMapHeight; y++)
			{
				if (y >= fSurface[x] * nMapHeight)		// If map pixel > surface pixel, make it land
					map[y * nMapWidth + x] = 1;
				else
					map[y * nMapWidth + x] = 0;			// Else leave it as sky
			}

		delete[] fSurface;
		delete[] fNoiseSeed;
	}

	// Function taken from seperate project
	void PerlinNoise1D(int nCount, float* fSeed, int nOctaves, float fBias, float* fOutput)
	{
		// 1D Perlin noise generation
		for (int x = 0; x < nCount; x++)
		{
			float fNoise = 0.0f;
			float fScaleAcc = 0.0f;
			float fScale = 1.0f;

			for (int o = 0; o < nOctaves; o++)
			{
				int nPitch = nCount >> o;
				int nSample1 = (x / nPitch) * nPitch;
				int nSample2 = (nSample1 + nPitch) % nCount;

				float fBlend = (float)(x - nSample1) / (float)nPitch;

				float fSample = (1.0f - fBlend) * fSeed[nSample1] + fBlend * fSeed[nSample2];

				fScaleAcc += fScale;
				fNoise += fSample * fScale;
				fScale = fScale / fBias;
			}

			// Scales to seed range
			fOutput[x] = fNoise / fScaleAcc;
		}
	}

};

int main()
{
	Worms game;
	game.Construct(600, 400, 2, 2);
	game.Start();

	return 0;
}