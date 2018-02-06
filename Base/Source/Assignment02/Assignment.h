#ifndef Assignment_H
#define Assignment_H

#include "Scene.h"
#include "Mtx44.h"
#include "../PlayerInfo/PlayerInfo.h"
#include "../GroundEntity.h"
#include "../FPSCamera.h"
#include "Mesh.h"
#include "MatrixStack.h"
#include "../GenericEntity.h"
#include "../SceneGraph/UpdateTransformation.h"
#include "../Enemy\AndroidRobots.h"
#include "../SpatialPartition\SpatialPartition.h"
#include "../Waypoint/WaypointManager.h"
#include "../Lua/LuaInterface.h"
#include "../Assignment02/Enemy.h"

class ShaderProgram;
class SceneManager;
class TextEntity;
class Light;

class Assignment : public Scene
{	
public:
	Assignment();
	~Assignment();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

private:
	//Assignment(SceneManager* _sceneMgr); // This is used to register to SceneManager

	ShaderProgram* currProg;
	CPlayerInfo* playerInfo;
	GroundEntity* groundEntity;
	FPSCamera camera;
	TextEntity* textObj[3];
	Light* lights[2];

	GenericEntity* theCube;
	CAndroidRobot* theRobot;
	CEnemy* theEnemy;

	//static Assignment* sInstance; // The pointer to the object that gets registered
};

#endif