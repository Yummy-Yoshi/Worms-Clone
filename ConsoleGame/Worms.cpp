#include <iostream>
#include <string>
#include <algorithm>

using namespace std;

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

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