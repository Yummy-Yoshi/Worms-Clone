#include <iostream>
#include <string>
#include <algorithm>

using namespace std;

#include "olcConsoleGameEngine.h"

class Worms : public olcConsoleGameEngine
{
public:
	Worms()
	{
		m_sAppName = L"Worms";
	}

private:
	virtual bool OnUserCreate()
	{
		return true;
	}

	virtual bool OnUserUpdate(float fElapsedTime)
	{
		return true;
	}

};

int main()
{
	Worms game;
	game.ConstructConsole(256, 160, 6, 6);
	game.Start();

	return 0;
}