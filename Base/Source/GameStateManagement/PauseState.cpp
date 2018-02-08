#include <iostream>
using namespace std;

#include "PauseState.h"
#include "GL\glew.h"
#include "../Application.h"
#include "LoadTGA.h"
#include "GraphicsManager.h"
#include "MeshBuilder.h"
#include "../TextEntity.h"
#include "RenderHelper.h"
#include "../SpriteEntity.h"
#include "../EntityManager.h"

#include "KeyboardController.h"
#include "SceneManager.h"

#include "../PlayerInfo/PlayerInfo.h"

CPauseState::CPauseState()
{

}
CPauseState::~CPauseState()
{

}

void CPauseState::Init()
{
	// Create and attach the camera to the scene
	camera.Init(Vector3(0, 0, 10), Vector3(0, 0, 0), Vector3(0, 1, 0));
	GraphicsManager::GetInstance()->AttachCamera(&camera);

	// Load all the meshes
	MeshBuilder::GetInstance()->GenerateQuad("PAUSESTATE_BKGROUND", Color(1, 1, 1), 1.f);
	MeshBuilder::GetInstance()->GetMesh("PAUSESTATE_BKGROUND")->textureID = LoadTGA("Image//PauseState.tga");
	float halfWindowWidth = Application::GetInstance().GetWindowWidth() / 2.0f;
	float halfWindowHeight = Application::GetInstance().GetWindowHeight() / 2.0f;
	PauseStateBackground = Create::Sprite2DObject("PAUSESTATE_BKGROUND", 
													Vector3(halfWindowWidth, halfWindowHeight, 0.0f), 
													Vector3(800.0f, 600.0f, 0.0f));

	cout << "CPauseState loaded\n" << endl;
}
void CPauseState::Update(double dt)
{
	if (KeyboardController::GetInstance()->IsKeyReleased(0x50))
	{
		cout << "Loading GameState" << endl;
		SceneManager::GetInstance()->SetActiveScene("GameState");
	}
}
void CPauseState::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//GraphicsManager::GetInstance()->UpdateLightUniforms();

	// Setup 3D pipeline then render 3D
	GraphicsManager::GetInstance()->SetPerspectiveProjection(45.0f, 4.0f / 3.0f, 0.1f, 10000.0f);
	GraphicsManager::GetInstance()->AttachCamera(&camera);

	if (CPlayerInfo::GetInstance()->cameraAttached())
	{
		camera.SetCameraPos(CPlayerInfo::GetInstance()->GetPos());
		camera.SetCameraTarget(CPlayerInfo::GetInstance()->GetTarget());
		camera.SetCameraUp(CPlayerInfo::GetInstance()->GetUp());
	}

	// Render the required entities
	EntityManager::GetInstance()->Render();

	// Setup 2D pipeline then render 2D
	GraphicsManager::GetInstance()->SetOrthographicProjection(0, 
															  Application::GetInstance().GetWindowWidth(), 
															  0, 
															  Application::GetInstance().GetWindowHeight(), 
															  -10, 10);
	
	GraphicsManager::GetInstance()->DetachCamera();

	// Render the required entities
	EntityManager::GetInstance()->RenderUI();
}
void CPauseState::Exit()
{
	// Remove the entity from EntityManager
	EntityManager::GetInstance()->RemoveEntity(PauseStateBackground);

	// Remove the meshes which are specific to CPauseState
	MeshBuilder::GetInstance()->RemoveMesh("PAUSESTATE_BKGROUND");

	// Detach camera from other entities
	GraphicsManager::GetInstance()->DetachCamera();
}