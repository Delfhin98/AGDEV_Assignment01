#include "Assignment.h"
#include "GL\glew.h"

#include "shader.hpp"
#include "MeshBuilder.h"
#include "Application.h"
#include "Utility.h"
#include "LoadTGA.h"
#include <sstream>
#include "KeyboardController.h"
#include "MouseController.h"
#include "SceneManager.h"
#include "GraphicsManager.h"
#include "ShaderProgram.h"
#include "EntityManager.h"

#include "GenericEntity.h"
#include "GroundEntity.h"
#include "TextEntity.h"
#include "SpriteEntity.h"
#include "Light.h"
#include "SkyBox/SkyBoxEntity.h"
#include "SceneGraph\SceneGraph.h"
#include "SpatialPartition\SpatialPartition.h"

#include <iostream>
using namespace std;

Assignment* Assignment::sInstance = new Assignment(SceneManager::GetInstance());

Assignment::Assignment()
{
}

Assignment::Assignment(SceneManager* _sceneMgr)
{
	_sceneMgr->AddScene("Assignment", this);
}

Assignment::~Assignment()
{
	CSpatialPartition::GetInstance()->RemoveCamera();
	CSceneGraph::GetInstance()->Destroy();
}

void Assignment::Init()
{
	currProg = GraphicsManager::GetInstance()->LoadShader("default", "Shader//Texture.vertexshader", "Shader//Texture.fragmentshader");
	
	// Tell the shader program to store these uniform locations
	currProg->AddUniform("MVP");
	currProg->AddUniform("MV");
	currProg->AddUniform("MV_inverse_transpose");
	currProg->AddUniform("material.kAmbient");
	currProg->AddUniform("material.kDiffuse");
	currProg->AddUniform("material.kSpecular");
	currProg->AddUniform("material.kShininess");
	currProg->AddUniform("lightEnabled");
	currProg->AddUniform("numLights");
	currProg->AddUniform("lights[0].type");
	currProg->AddUniform("lights[0].position_cameraspace");
	currProg->AddUniform("lights[0].color");
	currProg->AddUniform("lights[0].power");
	currProg->AddUniform("lights[0].kC");
	currProg->AddUniform("lights[0].kL");
	currProg->AddUniform("lights[0].kQ");
	currProg->AddUniform("lights[0].spotDirection");
	currProg->AddUniform("lights[0].cosCutoff");
	currProg->AddUniform("lights[0].cosInner");
	currProg->AddUniform("lights[0].exponent");
	currProg->AddUniform("lights[1].type");
	currProg->AddUniform("lights[1].position_cameraspace");
	currProg->AddUniform("lights[1].color");
	currProg->AddUniform("lights[1].power");
	currProg->AddUniform("lights[1].kC");
	currProg->AddUniform("lights[1].kL");
	currProg->AddUniform("lights[1].kQ");
	currProg->AddUniform("lights[1].spotDirection");
	currProg->AddUniform("lights[1].cosCutoff");
	currProg->AddUniform("lights[1].cosInner");
	currProg->AddUniform("lights[1].exponent");
	currProg->AddUniform("colorTextureEnabled");
	currProg->AddUniform("colorTexture");
	currProg->AddUniform("textEnabled");
	currProg->AddUniform("textColor");
	
	// Tell the graphics manager to use the shader we just loaded
	GraphicsManager::GetInstance()->SetActiveShader("default");

	lights[0] = new Light();
	GraphicsManager::GetInstance()->AddLight("lights[0]", lights[0]);
	lights[0]->type = Light::LIGHT_DIRECTIONAL;
	lights[0]->position.Set(0, 20, 0);
	lights[0]->color.Set(1, 1, 1);
	lights[0]->power = 1;
	lights[0]->kC = 1.f;
	lights[0]->kL = 0.01f;
	lights[0]->kQ = 0.001f;
	lights[0]->cosCutoff = cos(Math::DegreeToRadian(45));
	lights[0]->cosInner = cos(Math::DegreeToRadian(30));
	lights[0]->exponent = 3.f;
	lights[0]->spotDirection.Set(0.f, 1.f, 0.f);
	lights[0]->name = "lights[0]";

	lights[1] = new Light();
	GraphicsManager::GetInstance()->AddLight("lights[1]", lights[1]);
	lights[1]->type = Light::LIGHT_DIRECTIONAL;
	lights[1]->position.Set(1, 1, 0);
	lights[1]->color.Set(1, 1, 0.5f);
	lights[1]->power = 0.4f;
	lights[1]->name = "lights[1]";

	currProg->UpdateInt("numLights", 1);
	currProg->UpdateInt("textEnabled", 0);
	
	// Create the playerinfo instance, which manages all information about the player
	playerInfo = CPlayerInfo::GetInstance();
	playerInfo->Init();

	// Create and attach the camera to the scene
	//camera.Init(Vector3(0, 0, 10), Vector3(0, 0, 0), Vector3(0, 1, 0));
	camera.Init(playerInfo->GetPos(), playerInfo->GetTarget(), playerInfo->GetUp());
	playerInfo->AttachCamera(&camera);
	GraphicsManager::GetInstance()->AttachCamera(&camera);

	// Load all the meshes
	MeshBuilder::GetInstance()->GenerateAxes("reference");
	MeshBuilder::GetInstance()->GenerateCrossHair("crosshair");
	MeshBuilder::GetInstance()->GenerateQuad("quad", Color(1, 1, 1), 1.f);
	MeshBuilder::GetInstance()->GetMesh("quad")->textureID = LoadTGA("Image//calibri.tga");
	MeshBuilder::GetInstance()->GenerateText("text", 16, 16);
	MeshBuilder::GetInstance()->GetMesh("text")->textureID = LoadTGA("Image//calibri.tga");
	MeshBuilder::GetInstance()->GetMesh("text")->material.kAmbient.Set(1, 0, 0);
	MeshBuilder::GetInstance()->GenerateOBJ("Chair", "OBJ//chair.obj");
	MeshBuilder::GetInstance()->GetMesh("Chair")->textureID = LoadTGA("Image//chair.tga");
	MeshBuilder::GetInstance()->GenerateRing("ring", Color(1, 0, 1), 36, 1, 0.5f);
	MeshBuilder::GetInstance()->GenerateSphere("lightball", Color(1, 1, 1), 18, 36, 1.f);
	MeshBuilder::GetInstance()->GenerateSphere("sphere", Color(1, 0, 0), 18, 36, 0.5f);
	MeshBuilder::GetInstance()->GenerateCone("cone", Color(0.5f, 1, 0.3f), 36, 10.f, 10.f);
	MeshBuilder::GetInstance()->GenerateCube("cube", Color(1.0f, 1.0f, 0.0f), 1.0f);
	MeshBuilder::GetInstance()->GetMesh("cone")->material.kDiffuse.Set(0.99f, 0.99f, 0.99f);
	MeshBuilder::GetInstance()->GetMesh("cone")->material.kSpecular.Set(0.f, 0.f, 0.f);
	MeshBuilder::GetInstance()->GenerateQuad("GRASS_DARKGREEN", Color(1, 1, 1), 1.f);
	MeshBuilder::GetInstance()->GetMesh("GRASS_DARKGREEN")->textureID = LoadTGA("Image//grass_darkgreen.tga");
	MeshBuilder::GetInstance()->GenerateQuad("GEO_GRASS_LIGHTGREEN", Color(1, 1, 1), 1.f);
	MeshBuilder::GetInstance()->GetMesh("GEO_GRASS_LIGHTGREEN")->textureID = LoadTGA("Image//grass_lightgreen.tga");
	MeshBuilder::GetInstance()->GenerateCube("cubeSG", Color(1.0f, 0.64f, 0.0f), 1.0f);

	MeshBuilder::GetInstance()->GenerateQuad("SKYBOX_FRONT", Color(1, 1, 1), 1.f);
	MeshBuilder::GetInstance()->GenerateQuad("SKYBOX_BACK", Color(1, 1, 1), 1.f);
	MeshBuilder::GetInstance()->GenerateQuad("SKYBOX_LEFT", Color(1, 1, 1), 1.f);
	MeshBuilder::GetInstance()->GenerateQuad("SKYBOX_RIGHT", Color(1, 1, 1), 1.f);
	MeshBuilder::GetInstance()->GenerateQuad("SKYBOX_TOP", Color(1, 1, 1), 1.f);
	MeshBuilder::GetInstance()->GenerateQuad("SKYBOX_BOTTOM", Color(1, 1, 1), 1.f);
	MeshBuilder::GetInstance()->GetMesh("SKYBOX_FRONT")->textureID = LoadTGA("Image//SkyBox//skybox_front.tga");
	MeshBuilder::GetInstance()->GetMesh("SKYBOX_BACK")->textureID = LoadTGA("Image//SkyBox//skybox_back.tga");
	MeshBuilder::GetInstance()->GetMesh("SKYBOX_LEFT")->textureID = LoadTGA("Image//SkyBox//skybox_left.tga");
	MeshBuilder::GetInstance()->GetMesh("SKYBOX_RIGHT")->textureID = LoadTGA("Image//SkyBox//skybox_right.tga");
	MeshBuilder::GetInstance()->GetMesh("SKYBOX_TOP")->textureID = LoadTGA("Image//SkyBox//skybox_top.tga");
	MeshBuilder::GetInstance()->GetMesh("SKYBOX_BOTTOM")->textureID = LoadTGA("Image//SkyBox//skybox_bottom.tga");
	MeshBuilder::GetInstance()->GenerateRay("laser", 10.0f);
	MeshBuilder::GetInstance()->GenerateQuad("GRIDMESH", Color(1, 1, 1), 10.f);

	// Load up Assignment Meshes //
	// Walls //
	MeshBuilder::GetInstance()->GenerateOBJ("Wall", "OBJ//Assignment//Wall_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("Wall")->textureID = LoadTGA("Image//Assignment//Wall_UV_TGA.tga");
	// Turrets //
	// Base
	MeshBuilder::GetInstance()->GenerateOBJ("TurretBase_HighDef", "OBJ//Assignment//Turret_Base_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("TurretBase_HighDef")->textureID = LoadTGA("Image//Assignment//Turret_Base_UV_Texture_HighDef.tga");
	MeshBuilder::GetInstance()->GenerateOBJ("TurretBase_MidDef", "OBJ//Assignment//Turret_Base_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("TurretBase_MidDef")->textureID = LoadTGA("Image//Assignment//Turret_Base_UV_Texture_MidDef.tga");
	MeshBuilder::GetInstance()->GenerateOBJ("TurretBase_LowDef", "OBJ//Assignment//Turret_Base_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("TurretBase_LowDef")->textureID = LoadTGA("Image//Assignment//Turret_Base_UV_Texture_LowDef.tga");
	// Stand
	MeshBuilder::GetInstance()->GenerateOBJ("TurretStand_HighDef", "OBJ//Assignment//Turret_Stand_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("TurretStand_HighDef")->textureID = LoadTGA("Image//Assignment//Turret_Stand_UV_Texture_HighDef.tga");
	MeshBuilder::GetInstance()->GenerateOBJ("TurretStand_MidDef", "OBJ//Assignment//Turret_Stand_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("TurretStand_MidDef")->textureID = LoadTGA("Image//Assignment//Turret_Stand_UV_Texture_MidDef.tga");
	MeshBuilder::GetInstance()->GenerateOBJ("TurretStand_LowDef", "OBJ//Assignment//Turret_Stand_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("TurretStand_LowDef")->textureID = LoadTGA("Image//Assignment//Turret_Stand_UV_Texture_LowDef.tga");
	// Source
	MeshBuilder::GetInstance()->GenerateOBJ("TurretSource_HighDef", "OBJ//Assignment//Turret_Source_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("TurretSource_HighDef")->textureID = LoadTGA("Image//Assignment//Turret_Source_UV_Texture_HighDef.tga");
	MeshBuilder::GetInstance()->GenerateOBJ("TurretSource_MidDef", "OBJ//Assignment//Turret_Source_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("TurretSource_MidDef")->textureID = LoadTGA("Image//Assignment//Turret_Source_UV_Texture_MidDef.tga");
	MeshBuilder::GetInstance()->GenerateOBJ("TurretSource_LowDef", "OBJ//Assignment//Turret_Source_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("TurretSource_LowDef")->textureID = LoadTGA("Image//Assignment//Turret_Source_UV_Texture_LowDef.tga");
	// Target Cursor
	MeshBuilder::GetInstance()->GenerateOBJ("TurretCursor_HighDef", "OBJ//Assignment//Turret_TargetCursor_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("TurretCursor_HighDef")->textureID = LoadTGA("Image//Assignment//Turret_TargetCursor_UV_Texture_HighDef.tga");
	MeshBuilder::GetInstance()->GenerateOBJ("TurretCursor_MidDef", "OBJ//Assignment//Turret_TargetCursor_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("TurretCursor_MidDef")->textureID = LoadTGA("Image//Assignment//Turret_TargetCursor_UV_Texture_MidDef.tga");
	MeshBuilder::GetInstance()->GenerateOBJ("TurretCursor_LowDef", "OBJ//Assignment//Turret_TargetCursor_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("TurretCursor_LowDef")->textureID = LoadTGA("Image//Assignment//Turret_TargetCursor_UV_Texture_LowDef.tga");
	// Android Robots //
	// Base
	MeshBuilder::GetInstance()->GenerateOBJ("RobotBase_HighDef", "OBJ//Assignment//AndroidBase_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("RobotBase_HighDef")->textureID = LoadTGA("Image//Assignment//AndroidBase_UV_Texture_HighDef.tga");
	MeshBuilder::GetInstance()->GenerateOBJ("RobotBase_MidDef", "OBJ//Assignment//AndroidBase_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("RobotBase_MidDef")->textureID = LoadTGA("Image//Assignment//AndroidBase_UV_Texture_MidDef.tga");
	MeshBuilder::GetInstance()->GenerateOBJ("RobotBase_LowDef", "OBJ//Assignment//AndroidBase_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("RobotBase_LowDef")->textureID = LoadTGA("Image//Assignment//AndroidBase_UV_Texture_LowDef.tga");
	// Body
	MeshBuilder::GetInstance()->GenerateOBJ("RobotBody_HighDef", "OBJ//Assignment//AndroidBody_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("RobotBody_HighDef")->textureID = LoadTGA("Image//Assignment//AndroidBody_UV_Texture_HighDef.tga");
	MeshBuilder::GetInstance()->GenerateOBJ("RobotBody_MidDef", "OBJ//Assignment//AndroidBody_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("RobotBody_MidDef")->textureID = LoadTGA("Image//Assignment//AndroidBody_UV_Texture_MidDef.tga");
	MeshBuilder::GetInstance()->GenerateOBJ("RobotBody_LowDef", "OBJ//Assignment//AndroidBody_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("RobotBody_LowDef")->textureID = LoadTGA("Image//Assignment//AndroidBody_UV_Texture_LowDef.tga");
	// Hands
	MeshBuilder::GetInstance()->GenerateOBJ("RobotHands_HighDef", "OBJ//Assignment//AndroidHands_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("RobotHands_HighDef")->textureID = LoadTGA("Image//Assignment//AndroidHands_UV_Texture_HighDef.tga");
	MeshBuilder::GetInstance()->GenerateOBJ("RobotHands_MidDef", "OBJ//Assignment//AndroidHands_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("RobotHands_MidDef")->textureID = LoadTGA("Image//Assignment//AndroidHands_UV_Texture_MidDef.tga");
	MeshBuilder::GetInstance()->GenerateOBJ("RobotHands_LowDef", "OBJ//Assignment//AndroidHands_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("RobotHands_LowDef")->textureID = LoadTGA("Image//Assignment//AndroidHands_UV_Texture_LowDef.tga");
	// Head
	MeshBuilder::GetInstance()->GenerateOBJ("RobotHead_HighDef", "OBJ//Assignment//AndroidHead_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("RobotHead_HighDef")->textureID = LoadTGA("Image//Assignment//AndroidHead_UV_Texture_HighDef.tga");
	MeshBuilder::GetInstance()->GenerateOBJ("RobotHead_MidDef", "OBJ//Assignment//AndroidHead_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("RobotHead_MidDef")->textureID = LoadTGA("Image//Assignment//AndroidHead_UV_Texture_MidDef.tga");
	MeshBuilder::GetInstance()->GenerateOBJ("RobotHead_LowDef", "OBJ//Assignment//AndroidHead_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("RobotHead_LowDef")->textureID = LoadTGA("Image//Assignment//AndroidHead_UV_Texture_LowDef.tga");

	// Set up the Spatial Partition and pass it to the EntityManager to manage
	CSpatialPartition::GetInstance()->Init(100, 100, 10, 10);
	CSpatialPartition::GetInstance()->SetMesh("GRIDMESH");
	CSpatialPartition::GetInstance()->SetCamera(&camera);
	CSpatialPartition::GetInstance()->SetLevelOfDetails(10000.0f, 20000.0f);
	EntityManager::GetInstance()->SetSpatialPartition(CSpatialPartition::GetInstance());

	RenderWalls();
	RenderTurrets();
	RenderRobots();

	// Create entities into the scene
	Create::Entity("reference", Vector3(0.0f, 0.0f, 0.0f)); // Reference
	Create::Entity("lightball", Vector3(lights[0]->position.x, lights[0]->position.y, lights[0]->position.z)); // Lightball

	GenericEntity* aCube = Create::Entity("cube", Vector3(-20.0f, 0.0f, -20.0f));
	aCube->SetCollider(true);
	aCube->SetAABB(Vector3(0.5f, 0.5f, 0.5f), Vector3(-0.5f, -0.5f, -0.5f));
	aCube->InitLOD("cube", "sphere", "cubeSG");

	// Add the pointer to this new entity to the Scene Graph
	CSceneNode* theNode = CSceneGraph::GetInstance()->AddNode(aCube);
	if (theNode == NULL)
	{
		cout << "EntityManager::AddEntity: Unable to add to scene graph!" << endl;
	}

	GenericEntity* anotherCube = Create::Entity("cube", Vector3(-20.0f, 1.1f, -20.0f));
	anotherCube->SetCollider(true);
	anotherCube->SetAABB(Vector3(0.5f, 0.5f, 0.5f), Vector3(-0.5f, -0.5f, -0.5f));
	CSceneNode* anotherNode = theNode->AddChild(anotherCube);
	if (anotherNode == NULL)
	{
		cout << "EntityManager::AddEntity: Unable to add to scene graph!" << endl;
	}

	GenericEntity* baseCube = Create::Asset("cube", Vector3(0.0f, 0.0f, 0.0f));
	CSceneNode* baseNode = CSceneGraph::GetInstance()->AddNode(baseCube);

	CUpdateTransformation* baseMtx = new CUpdateTransformation();
	baseMtx->ApplyUpdate(1.0f, 0.0f, 0.0f, 1.0f);
	baseMtx->SetSteps(-60, 60);
	baseNode->SetUpdateTransformation(baseMtx);

	GenericEntity* childCube = Create::Asset("cubeSG", Vector3(0.0f, 0.0f, 0.0f));
	CSceneNode* childNode = baseNode->AddChild(childCube);
	childNode->ApplyTranslate(0.0f, 1.0f, 0.0f);

	GenericEntity* grandchildCube = Create::Asset("cubeSG", Vector3(0.0f, 0.0f, 0.0f));
	CSceneNode* grandchildNode = childNode->AddChild(grandchildCube);
	grandchildNode->ApplyTranslate(0.0f, 0.0f, 1.0f);
	CUpdateTransformation* aRotateMtx = new CUpdateTransformation();
	aRotateMtx->ApplyUpdate(1.0f, 0.0f, 0.0f, 1.0f);
	aRotateMtx->SetSteps(-120, 60);
	grandchildNode->SetUpdateTransformation(aRotateMtx);

	groundEntity = Create::Ground("GRASS_DARKGREEN", "GEO_GRASS_LIGHTGREEN");
//	Create::Text3DObject("text", Vector3(0.0f, 0.0f, 0.0f), "DM2210", Vector3(10.0f, 10.0f, 10.0f), Color(0, 1, 1));
	Create::Sprite2DObject("crosshair", Vector3(0.0f, 0.0f, 0.0f), Vector3(10.0f, 10.0f, 10.0f));

	SkyBoxEntity* theSkyBox = Create::SkyBox("SKYBOX_FRONT", "SKYBOX_BACK",
											 "SKYBOX_LEFT", "SKYBOX_RIGHT",
											 "SKYBOX_TOP", "SKYBOX_BOTTOM");

	// Customise the ground entity
	groundEntity->SetPosition(Vector3(0, -10, 0));
	groundEntity->SetScale(Vector3(100.0f, 100.0f, 100.0f));
	groundEntity->SetGrids(Vector3(10.0f, 1.0f, 10.0f));
	playerInfo->SetTerrain(groundEntity);

	// Setup the 2D entities
	float halfWindowWidth = Application::GetInstance().GetWindowWidth() / 2.0f;
	float halfWindowHeight = Application::GetInstance().GetWindowHeight() / 2.0f;
	float fontSize = 25.0f;
	float halfFontSize = fontSize / 2.0f;
	for (int i = 0; i < 3; ++i)
	{
		textObj[i] = Create::Text2DObject("text", Vector3(-halfWindowWidth, -halfWindowHeight + fontSize*i + halfFontSize, 0.0f), "", Vector3(fontSize, fontSize, fontSize), Color(0.0f,1.0f,0.0f));
	}
	textObj[0]->SetText("HELLO WORLD");
}

void Assignment::RenderWalls()
{
	float m_fPosY_Offset = -10.f;

	// Left Wall
	GenericEntity* LeftWall = Create::Entity("Wall", Vector3(0.0f, m_fPosY_Offset, -200.0f));
	LeftWall->SetScale(Vector3(100.f, 50.f, 10.f));

	// Right Wall
	GenericEntity* RightWall = Create::Entity("Wall", Vector3(0.0f, m_fPosY_Offset, 200.0f));
	RightWall->SetScale(Vector3(100.f, 50.f, 10.f));

	// Back Wall
	GenericEntity* BackWall = Create::Entity("Wall", Vector3(200.0f, m_fPosY_Offset, 0.0f), Vector3(0.f, 1.f, 0.f), 90.f, Vector3(100.f, 50.f, 10.f));

	// Front Wall
	GenericEntity* FrontWall = Create::Entity("Wall", Vector3(-200.0f, m_fPosY_Offset, 0.0f), Vector3(0.f, 1.f, 0.f), 90.f, Vector3(100.f, 50.f, 10.f));

	// Ceiling
	GenericEntity* Ceiling = Create::Entity("Wall", Vector3(0.0f, 90.f, -200.0f), Vector3(1.f, 0.f, 0.f), 90.f, Vector3(100.f, 200.f, 10.f));
}

void Assignment::RenderTurrets()
{
	float m_fPosY_Offset = -10.f;

	// Base
	GenericEntity* TurretBase = Create::Entity("TurretBase_HighDef", Vector3(100.0f, m_fPosY_Offset, -100.0f));
	TurretBase->SetScale(Vector3(20.f, 10.f, 20.f));
	TurretBase->InitLOD("TurretBase_HighDef", "TurretBase_MidDef", "TurretBase_LowDef");
	
	// Stand 
	GenericEntity* TurretStand = Create::Entity("TurretStand_HighDef", Vector3(100.0f, m_fPosY_Offset - 2.f, -100.0f));
	TurretStand->SetScale(Vector3(10.f, 20.f, 10.f));
	TurretStand->InitLOD("TurretStand_HighDef", "TurretStand_MidDef", "TurretStand_LowDef");

	// Source
	GenericEntity* TurretSource = Create::Entity("TurretSource_HighDef", Vector3(100.0f, m_fPosY_Offset + 3.5f, -100.0f));
	TurretSource->SetScale(Vector3(10.f, 10.f, 10.f));
	TurretSource->InitLOD("TurretSource_HighDef", "TurretSource_MidDef", "TurretSource_LowDef");

	// Cursor
	GenericEntity* TurretCursor = Create::Entity("TurretCursor_HighDef", Vector3(100.0f, m_fPosY_Offset, -100.0f));
	TurretCursor->SetScale(Vector3(15.f, 15.f, 15.f));
	TurretCursor->InitLOD("TurretCursor_HighDef", "TurretCursor_MidDef", "TurretCursor_LowDef");
	
}

void Assignment::RenderRobots()
{
	float m_fPosY_Offset = -10.f;

	// Base
	GenericEntity* RobotBase = Create::Entity("RobotBase_HighDef", Vector3(-100.0f, m_fPosY_Offset, -100.0f));
	RobotBase->SetScale(Vector3(20.f, 10.f, 20.f));
	RobotBase->InitLOD("RobotBase_HighDef", "RobotBase_MidDef", "RobotBase_LowDef");

	// Body 
	GenericEntity* RobotBody = Create::Entity("RobotBody_HighDef", Vector3(-100.0f, m_fPosY_Offset - 7.f, -100.0f));
	RobotBody->SetScale(Vector3(15.f, 20.f, 15.f));
	RobotBody->InitLOD("RobotBody_HighDef", "RobotBody_MidDef", "RobotBody_LowDef");

	// Hand
	GenericEntity* RobotHands = Create::Entity("RobotHands_HighDef", Vector3(-100.0f, m_fPosY_Offset, -95.0f));
	RobotHands->SetScale(Vector3(10.f, 10.f, 10.f));
	RobotHands->InitLOD("RobotHands_HighDef", "RobotHands_MidDef", "RobotHands_LowDef");

	// Head - Source
	GenericEntity* RobotHead = Create::Entity("RobotHead_HighDef", Vector3(-100.0f, m_fPosY_Offset - 2.f, -100.0f));
	RobotHead->SetScale(Vector3(15.f, 15.f, 15.f));
	RobotHead->InitLOD("RobotHead_HighDef", "RobotHead_MidDef", "RobotHead_LowDef");
}

void Assignment::Update(double dt)
{
	// Update our entities
	EntityManager::GetInstance()->Update(dt);

	// THIS WHOLE CHUNK TILL <THERE> CAN REMOVE INTO ENTITIES LOGIC! Or maybe into a scene function to keep the update clean
	if(KeyboardController::GetInstance()->IsKeyDown('1'))
		glEnable(GL_CULL_FACE);
	if(KeyboardController::GetInstance()->IsKeyDown('2'))
		glDisable(GL_CULL_FACE);
	if(KeyboardController::GetInstance()->IsKeyDown('3'))
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if(KeyboardController::GetInstance()->IsKeyDown('4'))
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	if(KeyboardController::GetInstance()->IsKeyDown('5'))
	{
		lights[0]->type = Light::LIGHT_POINT;
	}
	else if(KeyboardController::GetInstance()->IsKeyDown('6'))
	{
		lights[0]->type = Light::LIGHT_DIRECTIONAL;
	}
	else if(KeyboardController::GetInstance()->IsKeyDown('7'))
	{
		lights[0]->type = Light::LIGHT_SPOT;
	}

	if(KeyboardController::GetInstance()->IsKeyDown('I'))
		lights[0]->position.z -= (float)(10.f * dt);
	if(KeyboardController::GetInstance()->IsKeyDown('K'))
		lights[0]->position.z += (float)(10.f * dt);
	if(KeyboardController::GetInstance()->IsKeyDown('J'))
		lights[0]->position.x -= (float)(10.f * dt);
	if(KeyboardController::GetInstance()->IsKeyDown('L'))
		lights[0]->position.x += (float)(10.f * dt);
	if(KeyboardController::GetInstance()->IsKeyDown('O'))
		lights[0]->position.y -= (float)(10.f * dt);
	if(KeyboardController::GetInstance()->IsKeyDown('P'))
		lights[0]->position.y += (float)(10.f * dt);

	if (KeyboardController::GetInstance()->IsKeyReleased('M'))
	{
		CSceneNode* theNode = CSceneGraph::GetInstance()->GetNode(1);
		Vector3 pos = theNode->GetEntity()->GetPosition();
		theNode->GetEntity()->SetPosition(Vector3(pos.x + 50.0f, pos.y, pos.z + 50.0f));
	}
	if (KeyboardController::GetInstance()->IsKeyReleased('N'))
	{
		CSpatialPartition::GetInstance()->PrintSelf();
	}

	// if the left mouse button was released
	if (MouseController::GetInstance()->IsButtonReleased(MouseController::LMB))
	{
		cout << "Left Mouse Button was released!" << endl;
	}
	if (MouseController::GetInstance()->IsButtonReleased(MouseController::RMB))
	{
		cout << "Right Mouse Button was released!" << endl;
	}
	if (MouseController::GetInstance()->IsButtonReleased(MouseController::MMB))
	{
		cout << "Middle Mouse Button was released!" << endl;
	}
	if (MouseController::GetInstance()->GetMouseScrollStatus(MouseController::SCROLL_TYPE_XOFFSET) != 0.0)
	{
		cout << "Mouse Wheel has offset in X-axis of " << MouseController::GetInstance()->GetMouseScrollStatus(MouseController::SCROLL_TYPE_XOFFSET) << endl;
	}
	if (MouseController::GetInstance()->GetMouseScrollStatus(MouseController::SCROLL_TYPE_YOFFSET) != 0.0)
	{
		cout << "Mouse Wheel has offset in Y-axis of " << MouseController::GetInstance()->GetMouseScrollStatus(MouseController::SCROLL_TYPE_YOFFSET) << endl;
	}
	// <THERE>

	// Update the player position and other details based on keyboard and mouse inputs
	playerInfo->Update(dt);

	//camera.Update(dt); // Can put the camera into an entity rather than here (Then we don't have to write this)

	GraphicsManager::GetInstance()->UpdateLights(dt);

	// Update the 2 text object values. NOTE: Can do this in their own class but i'm lazy to do it now :P
	// Eg. FPSRenderEntity or inside RenderUI for LightEntity
	std::ostringstream ss;
	ss.precision(5);
	float fps = (float)(1.f / dt);
	ss << "FPS: " << fps;
	textObj[1]->SetText(ss.str());

	std::ostringstream ss1;
	ss1.precision(4);
	ss1 << "Player:" << playerInfo->GetPos();
	textObj[2]->SetText(ss1.str());
}

void Assignment::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GraphicsManager::GetInstance()->UpdateLightUniforms();

	// Setup 3D pipeline then render 3D
	GraphicsManager::GetInstance()->SetPerspectiveProjection(45.0f, 4.0f / 3.0f, 0.1f, 10000.0f);
	GraphicsManager::GetInstance()->AttachCamera(&camera);
	EntityManager::GetInstance()->Render();

	// Setup 2D pipeline then render 2D
	int halfWindowWidth = Application::GetInstance().GetWindowWidth() / 2;
	int halfWindowHeight = Application::GetInstance().GetWindowHeight() / 2;
	GraphicsManager::GetInstance()->SetOrthographicProjection(-halfWindowWidth, halfWindowWidth, -halfWindowHeight, halfWindowHeight, -10, 10);
	GraphicsManager::GetInstance()->DetachCamera();
	EntityManager::GetInstance()->RenderUI();
}

void Assignment::Exit()
{
	// Detach camera from other entities
	GraphicsManager::GetInstance()->DetachCamera();
	playerInfo->DetachCamera();

	if (playerInfo->DropInstance() == false)
	{
#if _DEBUGMODE==1
		cout << "Unable to drop PlayerInfo class" << endl;
#endif
	}

	// Delete the lights
	delete lights[0];
	delete lights[1];
}
