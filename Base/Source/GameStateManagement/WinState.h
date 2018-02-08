#pragma once

#include "Scene.h"
#include "Mtx44.h"
#include "Mesh.h"
#include "MatrixStack.h"
#include "../FPSCamera.h"
#include "../SpriteEntity.h"

class SceneManager;
//class TextEntity;
class CWinState : public Scene
{
public:
	CWinState();
	~CWinState();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

private:
	FPSCamera camera;
	SpriteEntity* WinStateBackground;

	//float timer;
	//	TextEntity* textObj[3];
};
