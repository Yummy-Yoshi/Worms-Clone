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
	int verts = vecModelCoordinates.size();				// Keep the model vector static
	vecTransformedCoordinates.resize(verts);			// Create another vector the same size as the model vector

	// Rotates
	for (int i = 0; i < verts; i++)
	{
		vecTransformedCoordinates[i].first = vecModelCoordinates[i].first * cosf(r) - vecModelCoordinates[i].second * sinf(r);
		vecTransformedCoordinates[i].second = vecModelCoordinates[i].first * sinf(r) + vecModelCoordinates[i].second * cosf(r);
	}

	// Scales
	for (int i = 0; i < verts; i++)
	{
		vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first * s;
		vecTransformedCoordinates[i].second = vecTransformedCoordinates[i].second * s;
	}

	// Translates
	for (int i = 0; i < verts; i++)
	{
		vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first + x;
		vecTransformedCoordinates[i].second = vecTransformedCoordinates[i].second + y;
	}

	// Draws closed polygon
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
	bool bDead = false;			// Represents indicator to check if object should be removed

	// Default constructor that sets position
	cPhysicsObject(float x = 0.0f, float y = 0.0f)
	{
		px = x;
		py = y;
	}

	// Makes the class abstract
	virtual void Draw(olc::PixelGameEngine* engine, float fOffsetX, float fOffsetY, bool bPixel = false) = 0;
	virtual int BounceDeathAction() = 0;
	virtual bool Damage(float d) = 0;
};

class cDummy : public cPhysicsObject		// Does nothing, shows a marker that helps with physics debug and test
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

/* Dummys for debugging purposes
vector<pair<float, float>> DefineDummy()		// Creates a unit circle with a line fom center to edge
{
	vector<pair<float, float>> vecModel;
	vecModel.push_back({ 0.0f, 0.0f });

	for (int i = 0; i < 10; i++)
		vecModel.push_back({ cosf(i / 9.0f * 2.0f * 3.14159f), sinf(i / 9.0f * 2.0f * 3.14159f) });

	return vecModel;
}
vector<pair<float, float>> cDummy::vecModel = DefineDummy();
*/

class cDebris : public cPhysicsObject // A small rock that bounces
{
public:
	cDebris(float x = 0.0f, float y = 0.0f) : cPhysicsObject(x, y)
	{
		// Sets velocity to random direction and size to make a "boom" effect
		vx = 10.0f * cosf(((float)rand() / (float)RAND_MAX) * 2.0f * 3.14159f);
		vy = 10.0f * sinf(((float)rand() / (float)RAND_MAX) * 2.0f * 3.14159f);
		radius = 1.0f;
		fFriction = 0.8f;
		bDead = false;
		bStable = false;
		nBounceBeforeDeath = 2;		// Deletes after bouncing 2 times		
	}

	virtual void Draw(olc::PixelGameEngine* engine, float fOffsetX, float fOffsetY, bool bPixel = false)
	{
		DrawWireFrameModel(engine, vecModel, px - fOffsetX, py - fOffsetY, atan2f(vy, vx), bPixel ? 0.5f : radius, olc::DARK_GREEN);
	}

	virtual int BounceDeathAction()
	{
		return 0;		// Does nothing, just fades away
	}

	virtual bool Damage(float d)
	{
		return true;		// Can't be damaged
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

class cMissile : public cPhysicsObject		// A projectile weapon
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
		bStable = false;
	}

	virtual void Draw(olc::PixelGameEngine* engine, float fOffsetX, float fOffsetY, bool bPixel = false)
	{
		DrawWireFrameModel(engine, vecModel, px - fOffsetX, py - fOffsetY, atan2f(vy, vx), bPixel ? 0.5f : radius, olc::BLACK);
	}

	virtual int BounceDeathAction()
	{
		return 20;		// Gives the Boom Function a radius of 20 to make big explosions
	}

	virtual bool Damage(float d)
	{
		return true;
	}

private:
	static vector<pair<float, float>> vecModel;
};

vector<pair<float, float>> DefineMissile()
{
	// Defines a rocket-like shape
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
		v.first /= 1.5f;
		v.second /= 1.5f;
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
		bStable = false;
		
		if (sprWorm == nullptr)		// Loads sprite data from sprite file
			sprWorm = new olc::Sprite("Sprites/worms1.png");
	}

	virtual void Draw(olc::PixelGameEngine* engine, float fOffsetX, float fOffsetY, bool bPixel = false)
	{
		engine->SetPixelMode(olc::Pixel::MASK);

		if (bIsPlayable)		// Draws Worm Sprite with health bar, in its team's colors
		{
			engine->DrawPartialSprite(px - fOffsetX - radius, py - fOffsetY - radius, sprWorm, nTeam * 8, 0, 8, 8);

			for (int i = 0; i < 11 * fHealth; i++)		// Draws health bar for worm
			{
				engine->Draw(px - 5 + i - fOffsetX, py + 5 - fOffsetY, olc::BLUE);
				engine->Draw(px - 5 + i - fOffsetX, py + 6 - fOffsetY, olc::BLUE);
			}
		}
		else		// Draws tombstone sprite for team's color
		{
			engine->DrawPartialSprite(px - fOffsetX - radius, py - fOffsetY - radius, sprWorm, nTeam * 8, 8, 8, 8);
		}

		engine->SetPixelMode(olc::Pixel::NORMAL);
	}

	virtual int BounceDeathAction()
	{
		return 0;		// Nothing
	}

	virtual bool Damage(float d) // Reduce worm's health by said amount
	{
		fHealth -= d;
		if (fHealth <= 0)		// Worm has died, no longer playable
		{ 
			fHealth = 0.0f;
			bIsPlayable = false;
		}
		return fHealth > 0;
	}

public:
	float fShootAngle = 0.0f;
	float fHealth = 1.0f;
	bool bIsPlayable = true;
	int nTeam = 0;		// The ID of which team this worm belongs to

private:
	static olc::Sprite* sprWorm;
};

olc::Sprite* cWorm::sprWorm = nullptr;

class cTeam		// Defines a group of worms
{
public:
	vector<cWorm*> vecMembers;
	int nCurrentMember = 0;		// Index into vector for current worms turn
	int nTeamSize = 0;		// Total number of worms in team

	bool IsTeamAlive()		// Iterates though all team members, if any of them have >0 health, return true
	{
		bool bAllDead = false;
		for (auto w : vecMembers)
			bAllDead |= (w->fHealth > 0.0f);
		return bAllDead;
	}

	cWorm* GetNextMember()		// Returns a pointer to the next team member that is valid for control
	{
		do {
			nCurrentMember++;
			if (nCurrentMember >= nTeamSize)
				nCurrentMember = 0;
		} while (vecMembers[nCurrentMember]->fHealth <= 0);
		
		return vecMembers[nCurrentMember];
	}
};

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
	char* map = nullptr;

	// For camera control
	float fCameraPosX = 0.0f;
	float fCameraPosY = 0.0f;
	float fCameraPosXTarget = 0.0f;
	float fCameraPosYTarget = 0.0f;

	enum GAME_STATE		// State machine for game events
	{
		GS_RESET = 0,
		GS_GENERATE_TERRAIN = 1,
		GS_GENERATING_TERRAIN,
		GS_ALLOCATE_UNITS,
		GS_ALLOCATING_UNITS,
		GS_START_PLAY,
		GS_CAMERA_MODE,
		GS_GAME_OVER1,
		GS_GAME_OVER2
	} nGameState, nNextState;

	enum AI_STATE		// State machine for AI player control
	{
		AI_ASSESS_ENVIRONMENT = 0,
		AI_MOVE,
		AI_CHOOSE_TARGET,
		AI_POSITION_FOR_TARGET,
		AI_AIM,
		AI_FIRE,
	} nAIState, nAINextState;

	bool bGameIsStable = false;		// Represents overall stablity of game
	bool bPlayerHasControl = false;		// Represents whether player has control over character
	bool bPlayerActionComplete = false;	// Represents whether player has finished an action

	list<unique_ptr<cPhysicsObject>> listObjects;		// Allows multiple types of objects in list; The list of objects in game

	cPhysicsObject* pObjectUnderControl = nullptr;		// Pointer for object under control; Directs user input towards an onject
	cPhysicsObject* pCameraTrackingObject = nullptr;	// Pointer for object the camera should be following

	bool bEnergising = false;		// Indicates if user is charging up a shot
	float fEnergyLevel = 0.0f;		// Amount that's been charged so far
	bool bFireWeapon = false;		// Trigger that handles firing of weapon

	float fTurnTime = 0.0f;				// Time left to take your turn
	bool bZoomOut = false;				// Renders the whole map
	bool bEnablePlayerControl = true;		// The player is in control, keyboard input enabled
	bool bEnableComputerControl = false;		// The AI is in control
	bool bPlayerHasFired = false;			// Weapon has been fired
	bool bShowCountDown = false;			// Displays turn time counter on screen

	vector<cTeam> vecTeams;		// Vector to store teams

	int nCurrentTeam = 0;		// Current team being controlled

	// AI control flags
	bool bAI_Jump = false;			// AI has pressed "JUMP" key
	bool bAI_AimLeft = false;		// AI has pressed "AIM_LEFT" key
	bool bAI_AimRight = false;		// AI has pressed "AIM_RIGHT" key
	bool bAI_Energise = false;		// AI has pressed "FIRE" key


	float fAITargetAngle = 0.0f;		// Angle AI should aim for
	float fAITargetEnergy = 0.0f;		// Energy level AI should aim for
	float fAISafePosition = 0.0f;		// X-Coordinate considered safe for AI to move to
	cWorm* pAITargetWorm = nullptr;		// Pointer to worm AI has selected as target
	float fAITargetX = 0.0f;		// X-Coordinate of target missile location
	float fAITargetY = 0.0f;		// Y-Coordinate of target missile location

	virtual bool OnUserCreate()		// Creates the map
	{
		map = new char[nMapWidth * nMapHeight];				// Allocate memory for 2D array
		memset(map, 0, nMapWidth * nMapHeight * sizeof(char));		// Clear all to 0

		// State machine creates map
		nGameState = GS_RESET;
		nNextState = GS_RESET;
		nAIState = AI_ASSESS_ENVIRONMENT;
		nAINextState = AI_ASSESS_ENVIRONMENT;

		bGameIsStable = false;

		return true;
	}

	virtual bool OnUserUpdate(float fElapsedTime)
	{
		// Tab key toggles between whole map view and up close view
		if (GetKey(olc::Key::TAB).bReleased)
			bZoomOut = !bZoomOut;

		/* Debugging items
		if (GetKey(olc::Key::M).bReleased)		// Whenever 'M' key is released, generate new map
			CreateMap();

		if (GetMouse(0).bReleased)		// Lanches debris wherever the left mouse button is released
			Boom(GetMouseX() + fCameraPosX, GetMouseY() + fCameraPosY, 10.0f);
		
		if (GetMouse(1).bReleased)		// Drops a missile wherever the right mouse button is released
			listObjects.push_back(unique_ptr<cMissile>(new cMissile(GetMouseX() + fCameraPosX, GetMouseY() + fCameraPosY)));

		if (GetMouse(2).bReleased)		// Creates a Worm/unit object wherever the middle mouse button is released
		{
			cWorm* worm = new cWorm(GetMouseX() + fCameraPosX, GetMouseY() + fCameraPosY);
			pObjectUnderControl = worm;
			pCameraTrackingObject = worm;
			listObjects.push_back(unique_ptr<cWorm>(worm));
		}
		*/

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

		// Control supervisor
		switch (nGameState)
		{
		case GS_RESET:		// Sets game variables to know state
		{
			bEnablePlayerControl = false;
			bGameIsStable = false;
			bPlayerHasFired = false;
			bShowCountDown = false;
			nNextState = GS_GENERATE_TERRAIN;
		}
		break;

		case GS_GENERATE_TERRAIN:		// Creates a new terrain
		{
			bZoomOut = true;
			CreateMap();
			bGameIsStable = false;
			bShowCountDown = false;
			nNextState = GS_GENERATING_TERRAIN;
		}
		break;

		case GS_GENERATING_TERRAIN:		// Waits for terrain to finish creating
		{
			bShowCountDown = false;
			if (bGameIsStable)
				nNextState = GS_ALLOCATE_UNITS;
		}
		break;

		case GS_ALLOCATE_UNITS:		// Adds a unit to the top of the screen
		{
			// Deploys teams
			int nTeams = 4;
			int nWormsPerTeam = 4;

			// Calculates the spacing of worms and teams
			float fSpacePerTeam = (float)nMapWidth / (float)nTeams;
			float fSpacePerWorm = fSpacePerTeam / (nWormsPerTeam * 2.0f);

			for (int t = 0; t < nTeams; t++)		// Creates teams
			{
				vecTeams.emplace_back(cTeam());
				float fTeamMiddle = (fSpacePerTeam / 2.0f) + (t * fSpacePerTeam);
				for (int w = 0; w < nWormsPerTeam; w++)
				{
					float fWormX = fTeamMiddle - ((fSpacePerWorm * (float)nWormsPerTeam) / 2.0f) + w * fSpacePerWorm;
					float fWormY = 0.0f;

					// Add worms to teams
					cWorm* worm = new cWorm(fWormX, fWormY);
					worm->nTeam = t;
					listObjects.push_back(unique_ptr<cWorm>(worm));
					vecTeams[t].vecMembers.push_back(worm);
					vecTeams[t].nTeamSize = nWormsPerTeam;
				}

				vecTeams[t].nCurrentMember = 0;
			}

			// Selects players first worm for control and camera tracking
			pObjectUnderControl = vecTeams[0].vecMembers[vecTeams[0].nCurrentMember];
			pCameraTrackingObject = pObjectUnderControl;
			bShowCountDown = false;
			nNextState = GS_ALLOCATING_UNITS;
		}
		break;

		case GS_ALLOCATING_UNITS:		// Stays in this state while units are deploying
		{
			if (bGameIsStable)
			{
				bEnablePlayerControl = true;
				bEnableComputerControl = false;
				fTurnTime = 15.0f;
				bZoomOut = false;
				nNextState = GS_START_PLAY;
			}
		}
		break;

		case GS_START_PLAY:		// Player is in control of unit
		{
			bShowCountDown = true;

			// If player has fired weapon, or turn time is up, move on to next state
			if (bPlayerHasFired || fTurnTime <= 0.0f)
				nNextState = GS_CAMERA_MODE;
		}
		break;

		case GS_CAMERA_MODE:		// Camera follows the object of interest until the physics engine has settled
		{
			bEnableComputerControl = false;
			bEnablePlayerControl = false;
			bPlayerHasFired = false;
			bShowCountDown = false;
			fEnergyLevel = 0.0f;

			if (bGameIsStable)		// Once settled, chooses the next worm
			{
				// Gets next team, if there is no next team, game is over
				int nOldTeam = nCurrentTeam;
				do {
					nCurrentTeam++;
					nCurrentTeam %= vecTeams.size();
				} while (!vecTeams[nCurrentTeam].IsTeamAlive());

				// Locks controls if AI team is currently playing
				if (nCurrentTeam == 0)		// The Player Team
				{
					// Swap these around for a complete AI battle
					bEnablePlayerControl = true;
					bEnableComputerControl = false;
				}
				else		// The AI Team
				{
					bEnablePlayerControl = false;
					bEnableComputerControl = true;
				}

				// Sets control and camera
				pObjectUnderControl = vecTeams[nCurrentTeam].GetNextMember();
				pCameraTrackingObject = pObjectUnderControl;
				fTurnTime = 15.0f;
				bZoomOut = false;
				nNextState = GS_START_PLAY;

				if (nCurrentTeam == nOldTeam)		// If no different team could be found, game is over, current team wins
					nNextState = GS_GAME_OVER1;
			}
		}
		break;

		case GS_GAME_OVER1:		// Zooms out and launch loads of missiles!
		{
			bEnableComputerControl = false;
			bEnablePlayerControl = false;
			bZoomOut = true;
			bShowCountDown = false;

			for (int i = 0; i < 100; i++)
			{
				int nBombX = rand() % nMapWidth;
				int nBombY = rand() % (nMapHeight / 2);
				listObjects.push_back(std::unique_ptr<cMissile>(new cMissile(nBombX, nBombY, 0.0f, 0.5f)));
			}

			nNextState = GS_GAME_OVER2;
		}
		break;

		case GS_GAME_OVER2:		// Stay here and wait for chaos to settle
		{
			bEnableComputerControl = false;
			bEnablePlayerControl = false;
			// No exit from this state!
		}
		break;
		}

		if (bEnableComputerControl)		// AI State Machine
		{
			switch (nAIState)
			{
			case AI_ASSESS_ENVIRONMENT:
			{

				int nAction = rand() % 3;
				if (nAction == 0)		// Plays defensively; Moves away from team
				{
					// Finds nearest ally, then walks away from them
					float fNearestAllyDistance = INFINITY;
					float fDirection = 0;
					cWorm* origin = (cWorm*)pObjectUnderControl;

					for (auto w : vecTeams[nCurrentTeam].vecMembers)
					{
						if (w != pObjectUnderControl)
						{
							if (fabs(w->px - origin->px) < fNearestAllyDistance)
							{
								fNearestAllyDistance = fabs(w->px - origin->px);
								fDirection = (w->px - origin->px) < 0.0f ? 1.0f : -1.0f;
							}
						}
					}

					if (fNearestAllyDistance < 50.0f)
						fAISafePosition = origin->px + fDirection * 80.0f;
					else
						fAISafePosition = origin->px;
				}

				if (nAction == 1)		// Plays aggresively; Moves towards middle
				{
					cWorm* origin = (cWorm*)pObjectUnderControl;
					float fDirection = ((float)(nMapWidth / 2.0f) - origin->px) < 0.0f ? -1.0f : 1.0f;
					fAISafePosition = origin->px + fDirection * 200.0f;
				}

				if (nAction == 2)		// Plays dumb; Doesn't move
				{
					cWorm* origin = (cWorm*)pObjectUnderControl;
					fAISafePosition = origin->px;
				}

				// Clamps so they don't walk off of the map
				if (fAISafePosition <= 20.0f) fAISafePosition = 20.0f;
				if (fAISafePosition >= nMapWidth - 20.0f) fAISafePosition = nMapWidth - 20.0f;
				nAINextState = AI_MOVE;
			}
			break;

			case AI_MOVE:		// Moving in this game is performed solely by jumping
			{
				cWorm* origin = (cWorm*)pObjectUnderControl;
				if (fTurnTime >= 8.0f && origin->px != fAISafePosition)		// If not in safe position, move towards it, within 8 seconds
				{
					if (fAISafePosition < origin->px && bGameIsStable)		// Jump towards target until worm is in range
					{
						origin->fShootAngle = -3.14159f * 0.6f;		// Find shooting angle for AI player to angle jump
						bAI_Jump = true;		// Manually presses jump key for AI player
						nAINextState = AI_MOVE;
					}

					if (fAISafePosition > origin->px && bGameIsStable)
					{
						origin->fShootAngle = -3.14159f * 0.4f;
						bAI_Jump = true;
						nAINextState = AI_MOVE;
					}
				}
				else
					nAINextState = AI_CHOOSE_TARGET;
			}
			break;

			case AI_CHOOSE_TARGET:		// Worm has finished moving, so it chooses a target
			{
				bAI_Jump = false;		// Not sending any movement commands, so jumping is diabled

				// Select a team that is not itself
				cWorm* origin = (cWorm*)pObjectUnderControl;
				int nCurrentTeam = origin->nTeam;
				int nTargetTeam = 0;
				do {
					nTargetTeam = rand() % vecTeams.size();
				} while (nTargetTeam == nCurrentTeam || !vecTeams[nTargetTeam].IsTeamAlive());

				// The aggressive strategy is to aim for the opponent unit with the most health
				cWorm* mostHealthyWorm = vecTeams[nTargetTeam].vecMembers[0];
				for (auto w : vecTeams[nTargetTeam].vecMembers)
					if (w->fHealth > mostHealthyWorm->fHealth)
						mostHealthyWorm = w;

				// Once target worm is selected, record its x & y coordinates
				pAITargetWorm = mostHealthyWorm;
				fAITargetX = mostHealthyWorm->px;
				fAITargetY = mostHealthyWorm->py;
				nAINextState = AI_POSITION_FOR_TARGET;
			}
			break;

			case AI_POSITION_FOR_TARGET:		// Calculates trajectory for target, if the worm needs to move, do so
			{
				cWorm* origin = (cWorm*)pObjectUnderControl;
				float dy = -(fAITargetY - origin->py);
				float dx = -(fAITargetX - origin->px);
				float fSpeed = 30.0f;
				float fGravity = 2.0f;

				bAI_Jump = false;

				// Angle distance equation to hit a coordinate
				float a = fSpeed * fSpeed * fSpeed * fSpeed - fGravity * (fGravity * dx * dx + 2.0f * dy * fSpeed * fSpeed);

				if (a < 0)		// Target is out of range
				{
					if (fTurnTime >= 5.0f)		// Will only move if there are more than 5 seconds left on the clock
					{
						if (pAITargetWorm->px < origin->px && bGameIsStable)		// Jump towards target until it is in range
						{
							origin->fShootAngle = -3.14159f * 0.6f;
							bAI_Jump = true;
							nAINextState = AI_POSITION_FOR_TARGET;
						}

						if (pAITargetWorm->px > origin->px && bGameIsStable)
						{
							origin->fShootAngle = -3.14159f * 0.4f;
							bAI_Jump = true;
							nAINextState = AI_POSITION_FOR_TARGET;
						}
					}
					else
					{
						// Worm is stuck, so just fire in the direction of the targeted enemy
						// It's dangerous to itself, but may clear a blockage
						fAITargetAngle = origin->fShootAngle;
						fAITargetEnergy = 0.75f;
						nAINextState = AI_AIM;
					}
				}
				else
				{
					// Worm is close enough, calculate trajectory
					float b1 = fSpeed * fSpeed + sqrtf(a);
					float b2 = fSpeed * fSpeed - sqrtf(a);

					float fTheta1 = atanf(b1 / (fGravity * dx));		// Max Height
					float fTheta2 = atanf(b2 / (fGravity * dx));		// Min Height

					// Uses max, as it has a greater chance of avoiding obstacles
					fAITargetAngle = fTheta1 - (dx > 0 ? 3.14159f : 0.0f);
					float fFireX = cosf(fAITargetAngle);
					float fFireY = sinf(fAITargetAngle);

					// AI is clamped to 3/4 power, to make things fair against human players
					fAITargetEnergy = 0.75f;
					nAINextState = AI_AIM;
				}
			}
			break;

			case AI_AIM:		// Lines up aiming cursor
			{
				cWorm* worm = (cWorm*)pObjectUnderControl;

				bAI_AimLeft = false;
				bAI_AimRight = false;
				bAI_Jump = false;

				if (worm->fShootAngle < fAITargetAngle)
					bAI_AimRight = true;
				else
					bAI_AimLeft = true;

				// Once the cursors are aligned, fire missile
				// Some noise could be added to the floating point value to give the AI varying accuracy, to manage game difficulty
				if (fabs(worm->fShootAngle - fAITargetAngle) <= 0.001f)
				{
					bAI_AimLeft = false;
					bAI_AimRight = false;
					fEnergyLevel = 0.0f;
					nAINextState = AI_FIRE;
				}
				else
					nAINextState = AI_AIM;
			}
			break;

			case AI_FIRE:
			{
				bAI_Energise = true;
				bFireWeapon = false;
				bEnergising = true;		// Energize amount will always be fixed to 0.75, but it's nice to still show the animation to the player

				if (fEnergyLevel >= fAITargetEnergy)
				{
					bFireWeapon = true;
					bAI_Energise = false;
					bEnergising = false;
					bEnableComputerControl = false;
					nAINextState = AI_ASSESS_ENVIRONMENT;
				}
			}
			break;
			}
		}

		
		fTurnTime -= fElapsedTime;			// Decreases turn time

		if (pObjectUnderControl != nullptr)		// If not null, then pointing to a worm
		{
			pObjectUnderControl->ax = 0.0f;

			if (pObjectUnderControl->bStable)	// Ensures user input applies only when object is stable
			{
				// When 'Z' is pressed, worm jumps in the aimed direction, if player is in control; If computer is in control, AI jumps
				if ((bEnablePlayerControl && GetKey(olc::Key::Z).bPressed) || (bEnableComputerControl && bAI_Jump))
				{
					float a = ((cWorm*)pObjectUnderControl)->fShootAngle;

					pObjectUnderControl->vx = 4.0f * cosf(a);
					pObjectUnderControl->vy = 8.0f * sinf(a);
					pObjectUnderControl->bStable = false;

					bAI_Jump = false;
				}

				// When 'A' is held, cursor turns counter-clockwise if player is in control; If computer is in control, AI aims left
				if ((bEnablePlayerControl && GetKey(olc::Key::A).bHeld) || (bEnableComputerControl && bAI_AimLeft))
				{
					cWorm* worm = (cWorm*)pObjectUnderControl;
					worm->fShootAngle -= 1.0f * fElapsedTime;

					if (worm->fShootAngle < -3.14159f)		// If below -pi, wraps around back to pi
						worm->fShootAngle += 3.14159f * 2.0f;
				}

				// When 'S' is held, cursor turns clockwise if player is in control; If computer is in control, AI aims right
				if ((bEnablePlayerControl && GetKey(olc::Key::S).bHeld) || (bEnableComputerControl && bAI_AimRight))
				{
					cWorm* worm = (cWorm*)pObjectUnderControl;
					worm->fShootAngle += 1.0f * fElapsedTime;

					if (worm->fShootAngle > 3.14159f)		// If above pi, wraps around back to -pi
						worm->fShootAngle -= 3.14159f * 2.0f;
				}

				if ((bEnablePlayerControl && GetKey(olc::Key::SPACE).bPressed))		// When spacebar is pressed, start charging weapon, if player is in control
				{
					bEnergising = true;
					bFireWeapon = false;
					fEnergyLevel = 0.0f;
				}

				// When spacebar is being held down, increse weapon charge if player is in control; If computer is in control, AI charges weapon
				if ((bEnablePlayerControl && GetKey(olc::Key::SPACE).bHeld) || (bEnableComputerControl && bAI_Energise))		
				{
					if (bEnergising)
					{
						fEnergyLevel += 0.75f * fElapsedTime;
						if (fEnergyLevel >= 1.0f)		// If energy level reaches max, fire weapon
						{
							fEnergyLevel = 1.0f;
							bFireWeapon = true;
						}
					}
				}

				if ((bEnablePlayerControl && GetKey(olc::Key::SPACE).bReleased))		// When spacebar is released, fire weapon if player is in control
				{
					if (bEnergising)		// While being charged up, as soon as released, weapon fires
						bFireWeapon = true;

					bEnergising = true;
				}
			}

			if (bFireWeapon)
			{
				cWorm* worm = (cWorm*)pObjectUnderControl;

				// Gets weapon origin
				float ox = worm->px;
				float oy = worm->py;

				// Gets weapon direction
				float dx = cosf(worm->fShootAngle);
				float dy = sinf(worm->fShootAngle);

				// Creates weapon object and adds it to the object list
				cMissile* m = new cMissile(ox, oy, dx * 40.0f * fEnergyLevel, dy * 40.0f * fEnergyLevel);
				listObjects.push_back(unique_ptr<cMissile>(m));
				pCameraTrackingObject = m;		// Makes camera track missile


				// Resets all weapon states
				bFireWeapon = false;
				fEnergyLevel = 0.0f;
				bEnergising = false;
				bPlayerHasFired = true;

				if (rand() % 100 >= 50)
					bZoomOut = true;
			}
		}

		if (pCameraTrackingObject != nullptr)		// Move camera automatically if tracking object isn't null
		{
			// Makes camera's current position slowly inerpolate between current and target position
			fCameraPosXTarget = pCameraTrackingObject->px - ScreenWidth() / 2;
			fCameraPosYTarget = pCameraTrackingObject->py - ScreenHeight() / 2;
			fCameraPosX += (fCameraPosXTarget - fCameraPosX) * 15.0f * fElapsedTime;
			fCameraPosY += (fCameraPosYTarget - fCameraPosY) * 15.0f * fElapsedTime;
		}

		// Clamp map boundaries to keep camera in bounds
		if (fCameraPosX < 0)
			fCameraPosX = 0;
		if (fCameraPosX >= nMapWidth - ScreenWidth())
			fCameraPosX = nMapWidth - ScreenWidth();
		if (fCameraPosY < 0)
			fCameraPosY = 0;
		if (fCameraPosY >= nMapHeight - ScreenHeight())
			fCameraPosY = nMapHeight - ScreenHeight();

		for (int z = 0; z < 10; z++)		// Does 10 physics iterations/frame for accurate, controllable calculations
		{
			for (auto& p : listObjects)		// Updates physics of all physical objects
			{
				// Applies gravity
				p->ay += 2.0f;

				// Updates velocity
				p->vx += p->ax * fElapsedTime;
				p->vy += p->ay * fElapsedTime;

				// Updates potential future position
				float fPotentialX = p->px + p->vx * fElapsedTime;
				float fPotentialY = p->py + p->vy * fElapsedTime;

				// Resets acceleration and stability
				p->ax = 0.0f;
				p->ay = 0.0f;
				p->bStable = false;

				// Checks colision with the map 
				float fAngle = atan2f(p->vy, p->vx);		
				float fResponseX = 0;
				float fResponseY = 0;
				bool bCollision = false;

				// Iterates though a semicircle of an object's radius that's rotated towards the direction of travel
				for (float r = fAngle - 3.14159f / 2.0f; r < fAngle + 3.14159f / 2.0f; r += 3.14159f / 4.0f)
				{
					// Calculates the test point on circumference of circle
					float fTestPosX = (p->radius) * cosf(r) + fPotentialX;
					float fTestPosY = (p->radius) * sinf(r) + fPotentialY;

					// Constrains to test within the map's boundary
					if (fTestPosX >= nMapWidth) fTestPosX = nMapWidth - 1;
					if (fTestPosY >= nMapHeight) fTestPosX = nMapHeight - 1;
					if (fTestPosX < 0) fTestPosX = 0;
					if (fTestPosY < 0) fTestPosY = 0;

					// Tests if any of the points on an object's semicircle intersects with the terrain
					if (map[(int)fTestPosY * nMapWidth + (int)fTestPosX] > 0)
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

				if (p->px < 0 || p->px > nMapWidth || p->py <0 || p->py > nMapHeight)
					p->bDead = true;

				// Finds angle of collision
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

						if (p->bDead)		// Action upon an objects death; If greater than 0, creates an explosion
						{
							int nResponse = p->BounceDeathAction();
							if (nResponse > 0)
							{
								Boom(p->px, p->py, nResponse);
								pCameraTrackingObject = nullptr;		// After debris settles, camera goes back to player
							}
						}

					}
				}
				else		// Else allow it to use the new potential positions
				{
					// Updates objects position with potential (x,y) coordinates
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
		if (!bZoomOut)
		{
			for (int x = 0; x < ScreenWidth(); x++)		// Iterate through all pixels on screen
				for (int y = 0; y < ScreenHeight(); y++)
				{
					switch (map[(y + (int)fCameraPosY) * nMapWidth + (x + (int)fCameraPosX)])		// Finds location in map & colors in for sky or terrain
					{
					// Sky radiants
					case -8: Draw(x, y, olc::VERY_DARK_CYAN); break;		
					case -7: Draw(x, y, olc::DARK_CYAN); break;
					case -6: Draw(x, y, olc::DARK_CYAN); break;
					case -5: Draw(x, y, olc::BLUE); break;
					case -4: Draw(x, y, olc::DARK_BLUE); break;
					case -3: Draw(x, y, olc::DARK_BLUE); break;
					case -2: Draw(x, y, olc::VERY_DARK_BLUE); break;
					case -1: Draw(x, y, olc::VERY_DARK_BLUE); break;
						
					// Simple sky
					case  0: Draw(x, y, olc::CYAN); break;
						
					// Land
					case  1: Draw(x, y, olc::DARK_GREEN); break;			
					}
				}

			for (auto& p : listObjects)		// Draws Objects
			{
				p->Draw(this, fCameraPosX, fCameraPosY);
				cWorm* worm = (cWorm*)pObjectUnderControl;

				if (p.get() == worm)		// If object is current worm under control, draws cursor
				{
					// Finds centerpoint of crosshair
					float cx = worm->px + 8.0f * cosf(worm->fShootAngle) - fCameraPosX;
					float cy = worm->py + 8.0f * sinf(worm->fShootAngle) - fCameraPosY;

					// Draws a '+' symbol for the cursor
					Draw(cx, cy, olc::BLACK);
					Draw(cx + 1, cy, olc::BLACK);
					Draw(cx - 1, cy, olc::BLACK);
					Draw(cx, cy + 1, olc::BLACK);
					Draw(cx, cy - 1, olc::BLACK);

					for (int i = 0; i < 11 * fEnergyLevel; i++)		// Draws an energy bar, indicating how much energy the weapon will be fired with
					{
						Draw(worm->px - 5 + i - fCameraPosX, worm->py - 12 - fCameraPosY, olc::GREEN);
						Draw(worm->px - 5 + i - fCameraPosX, worm->py - 11 - fCameraPosY, olc::RED);
					}
				}
			}
		}
		else
		{
			for (int x = 0; x < ScreenWidth(); x++)
				for (int y = 0; y < ScreenHeight(); y++)
				{
					float fx = (float)x / (float)ScreenWidth() * (float)nMapWidth;
					float fy = (float)y / (float)ScreenHeight() * (float)nMapHeight;

					switch (map[(int)fy * nMapWidth + (int)fx])
					{
					// Sky radiants
					case -8: Draw(x, y, olc::VERY_DARK_CYAN); break;
					case -7: Draw(x, y, olc::DARK_CYAN); break;
					case -6: Draw(x, y, olc::DARK_CYAN); break;
					case -5: Draw(x, y, olc::BLUE); break;
					case -4: Draw(x, y, olc::DARK_BLUE); break;
					case -3: Draw(x, y, olc::DARK_BLUE); break;
					case -2: Draw(x, y, olc::VERY_DARK_BLUE); break;
					case -1: Draw(x, y, olc::VERY_DARK_BLUE); break;
						
					// Simple sky	
					case  0: Draw(x, y, olc::CYAN); break;

					// Land
					case  1: Draw(x, y, olc::DARK_GREEN); break;
					}
				}

			for (auto& p : listObjects)
				p->Draw(this, p->px - (p->px / (float)nMapWidth) * (float)ScreenWidth(),
					p->py - (p->py / (float)nMapHeight) * (float)ScreenHeight(), true);
		}

		// Checks for game state stability
		bGameIsStable = true;
		for(auto &p : listObjects)		// Iterates through all objects and checks if stable
			if (!p->bStable)
			{
				bGameIsStable = false;
				break;
			}

		/* Marker for debugging purposes		
		if (bGameIsStable)
			FillRect(2, 2, 4, 4, olc::RED);
		*/
		
		for (size_t t = 0; t < vecTeams.size(); t++)		// Draws team health bars
		{
			float fTotalHealth = 0.0f;
			float fMaxHealth = (float)vecTeams[t].nTeamSize;
			for (auto w : vecTeams[t].vecMembers)		// Accumulates team health
				fTotalHealth += w->fHealth;

			olc::Pixel cols[] = { olc::RED, olc::BLUE, olc::MAGENTA, olc::GREEN };
			FillRect(4, 4 + t * 4, (fTotalHealth / fMaxHealth) * (float)(ScreenWidth() - 8), 3, cols[t]);
		}

		// Counts down using 7 segment display
		if (bShowCountDown) {
			int tx = 4;
			int ty = vecTeams.size() * 4 + 8;
			int nCountDown = round(fTurnTime);
			if (nCountDown < 10) {
				SevenSegmentDisplay(tx, ty, nCountDown, olc::DARK_GREY, 2);
			}
		}

		nGameState = nNextState;
		nAIState = nAINextState;

		return true;
	}

	void SevenSegmentDisplay(int x, int y, int digit, olc::Pixel col = olc::WHITE, int scale = 1)
	{
		// Encodes which segment is active per digit
		char segmentCode[10] = {
			// For digit '0' the segments 0, 1, 2, 4, 5, 6 are active
			0b01110111,
			0b00100100,
			0b01011101,
			0b01101101,
			0b00101110,
			0b01101011,
			0b01111011,
			0b00100101,
			0b01111111,
			0b01101111,
		};

		// Checks if the bit at cByte[nIndex] is set
		auto is_bit_active = [=](char cByte, int nIndex) -> bool {
			return ((cByte >> nIndex) & 0x01) == 0x01;
			};

		// Draws the segments in a 8x8 grid
		if (is_bit_active(segmentCode[digit], 0)) FillRect(x + (1 * scale), y, 4 * scale, 1 * scale, col);
		if (is_bit_active(segmentCode[digit], 1)) FillRect(x, y + (1 * scale), 1 * scale, 3 * scale, col);
		if (is_bit_active(segmentCode[digit], 2)) FillRect(x + (5 * scale), y + (1 * scale), 1 * scale, 3 * scale, col);
		if (is_bit_active(segmentCode[digit], 3)) FillRect(x + (1 * scale), y + (4 * scale), 4 * scale, 1 * scale, col);
		if (is_bit_active(segmentCode[digit], 4)) FillRect(x, y + (5 * scale), 1 * scale, 3 * scale, col);
		if (is_bit_active(segmentCode[digit], 5)) FillRect(x + (5 * scale), y + (5 * scale), 1 * scale, 3 * scale, col);
		if (is_bit_active(segmentCode[digit], 6)) FillRect(x + (1 * scale), y + (8 * scale), 4 * scale, 1 * scale, col);
	}

	void Boom(float fWorldX, float fWorldY, float fRadius)		// Launches debris
	{
		auto CircleBresenham = [&](int xc, int yc, int r)	// Bresenham's midpoint circle algorithm sourced from Wikipedia
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

		int bx = (int)fWorldX;
		int by = (int)fWorldY;

		CircleBresenham(fWorldX, fWorldY, fRadius);		// Erases terrain to form a crater

		for (auto& p : listObjects)		// Knocks back other objects in range using Pythagorean Theorem
		{
			float dx = p->px - fWorldX;
			float dy = p->py - fWorldY;
			float fDist = sqrt(dx * dx + dy * dy);

			if (fDist < 0.0001f) fDist = 0.0001f;		// Prevents possible division by zero

			if (fDist < fRadius)		// Closer objects to explosion get bigger boost
			{
				p->vx = (dx / fDist) * fRadius;
				p->vy = (dy / fDist) * fRadius;
				p->Damage(((fRadius - fDist) / fRadius) * 0.8f);
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
				{
					// Shade the sky according to altitude - we only do top 1/3 of map, as the Boom() function will just paint in 0 (cyan)
					if ((float)y < (float)nMapHeight / 3.0f)
						map[y * nMapWidth + x] = (-8.0f * ((float)y / (nMapHeight / 3.0f))) - 1.0f;
					else
						map[y * nMapWidth + x] = 0;
				}
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
	if (game.Construct(640, 400, 2, 2))
		game.Start();

	return 0;
}
