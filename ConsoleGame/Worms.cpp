#include <iostream>
#include <string>
#include <algorithm>

using namespace std;

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

// Port DrawWireFrameModel function from Console Game Engine
void DrawWireFrameModel(olc::PixelGameEngine* engine, const std::vector<std::pair<float, float>>& vecModelCoordinates,
	float x, float y, float r = 0.0f, float s = 1.0f, olc::Pixel col = olc::WHITE)
{
	// vecModelCoordinates : the wire frame model
	// x, y : the screenposition where to draw it
	// r : angle of rotation
	// s : scaling factor

	// Create translated model vector of coordinate pairs
	std::vector<std::pair<float, float>> vecTransformedCoordinates;		// pair.first : x coordinate, pair.second : y coordinate
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

	// Default constructor that sets position
	cPhysicsObject(float x = 0.0f, float y = 0.0f)
	{
		px = x;
		py = y;
	}

	// Makes the class abstract
	virtual void Draw(olc::PixelGameEngine* engine, float fOffsetX, float fOffsetY) = 0;
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

	list<cPhysicsObject*> listObjects;		// Allows multiple types of objects in list

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

		return true;
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