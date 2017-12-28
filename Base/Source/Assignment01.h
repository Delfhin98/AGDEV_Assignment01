#ifndef Assignment01_01_H
#define Assignment01_01_H

#include "Scene.h"
#include "Mtx44.h"
#include "PlayerInfo/PlayerInfo.h"
#include "GroundEntity.h"
#include "FPSCamera.h"
#include "Mesh.h"
#include "MatrixStack.h"
#include "GenericEntity.h"
#include "SceneGraph/UpdateTransformation.h"
#include "Enemy\AndroidRobots.h"

class ShaderProgram;
class SceneManager;
class TextEntity;
class Light;
class Assignment01 : public Scene
{	
public:
	Assignment01();
	~Assignment01();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

private:
	Assignment01(SceneManager* _sceneMgr); // This is used to register to SceneManager

	ShaderProgram* currProg;
	CPlayerInfo* playerInfo;
	GroundEntity* groundEntity;
	FPSCamera camera;
	TextEntity* textObj[3];
	Light* lights[2];

	GenericEntity* theCube;
	CAndroidRobot* theRobot;

	static Assignment01* sInstance; // The pointer to the object that gets registered
};

#endif