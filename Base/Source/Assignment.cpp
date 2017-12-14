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
	MeshBuilder::GetInstance()->GenerateOBJ("Wall2", "OBJ//Assignment//Wall2_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("Wall2")->textureID = LoadTGA("Image//Assignment//Wall_UV_TGA.tga");
	// Turrets //
	// Base
	MeshBuilder::GetInstance()->GenerateOBJ("TurretBase_HighDef", "OBJ//Assignment//Turret_Base2_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("TurretBase_HighDef")->textureID = LoadTGA("Image//Assignment//Turret_Base_UV_Texture_HighDef.tga");
	MeshBuilder::GetInstance()->GenerateOBJ("TurretBase_MidDef", "OBJ//Assignment//Turret_Base2_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("TurretBase_MidDef")->textureID = LoadTGA("Image//Assignment//Turret_Base_UV_Texture_MidDef.tga");
	MeshBuilder::GetInstance()->GenerateOBJ("TurretBase_LowDef", "OBJ//Assignment//Turret_Base2_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("TurretBase_LowDef")->textureID = LoadTGA("Image//Assignment//Turret_Base_UV_Texture_LowDef.tga");
	// Stand
	MeshBuilder::GetInstance()->GenerateOBJ("TurretStand_HighDef", "OBJ//Assignment//Turret_Stand2_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("TurretStand_HighDef")->textureID = LoadTGA("Image//Assignment//Turret_Stand_UV_Texture_HighDef.tga");
	MeshBuilder::GetInstance()->GenerateOBJ("TurretStand_MidDef", "OBJ//Assignment//Turret_Stand2_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("TurretStand_MidDef")->textureID = LoadTGA("Image//Assignment//Turret_Stand_UV_Texture_MidDef.tga");
	MeshBuilder::GetInstance()->GenerateOBJ("TurretStand_LowDef", "OBJ//Assignment//Turret_Stand2_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("TurretStand_LowDef")->textureID = LoadTGA("Image//Assignment//Turret_Stand_UV_Texture_LowDef.tga");
	// Source
	MeshBuilder::GetInstance()->GenerateOBJ("TurretSource_HighDef", "OBJ//Assignment//Turret_Source2_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("TurretSource_HighDef")->textureID = LoadTGA("Image//Assignment//Turret_Source_UV_Texture_HighDef.tga");
	MeshBuilder::GetInstance()->GenerateOBJ("TurretSource_MidDef", "OBJ//Assignment//Turret_Source2_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("TurretSource_MidDef")->textureID = LoadTGA("Image//Assignment//Turret_Source_UV_Texture_MidDef.tga");
	MeshBuilder::GetInstance()->GenerateOBJ("TurretSource_LowDef", "OBJ//Assignment//Turret_Source2_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("TurretSource_LowDef")->textureID = LoadTGA("Image//Assignment//Turret_Source_UV_Texture_LowDef.tga");
	// Target Cursor
	MeshBuilder::GetInstance()->GenerateOBJ("TurretCursor_HighDef", "OBJ//Assignment//Turret_TargetCursor2_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("TurretCursor_HighDef")->textureID = LoadTGA("Image//Assignment//Turret_TargetCursor_UV_Texture_HighDef.tga");
	MeshBuilder::GetInstance()->GenerateOBJ("TurretCursor_MidDef", "OBJ//Assignment//Turret_TargetCursor2_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("TurretCursor_MidDef")->textureID = LoadTGA("Image//Assignment//Turret_TargetCursor_UV_Texture_MidDef.tga");
	MeshBuilder::GetInstance()->GenerateOBJ("TurretCursor_LowDef", "OBJ//Assignment//Turret_TargetCursor2_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("TurretCursor_LowDef")->textureID = LoadTGA("Image//Assignment//Turret_TargetCursor_UV_Texture_LowDef.tga");
	// Android Robots //
	// Base
	MeshBuilder::GetInstance()->GenerateOBJ("RobotBase_HighDef", "OBJ//Assignment//AndroidBase2_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("RobotBase_HighDef")->textureID = LoadTGA("Image//Assignment//AndroidBase_UV_Texture_HighDef.tga");
	MeshBuilder::GetInstance()->GenerateOBJ("RobotBase_MidDef", "OBJ//Assignment//AndroidBase2_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("RobotBase_MidDef")->textureID = LoadTGA("Image//Assignment//AndroidBase_UV_Texture_MidDef.tga");
	MeshBuilder::GetInstance()->GenerateOBJ("RobotBase_LowDef", "OBJ//Assignment//AndroidBase2_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("RobotBase_LowDef")->textureID = LoadTGA("Image//Assignment//AndroidBase_UV_Texture_LowDef.tga");
	// Body
	MeshBuilder::GetInstance()->GenerateOBJ("RobotBody_HighDef", "OBJ//Assignment//AndroidBody2_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("RobotBody_HighDef")->textureID = LoadTGA("Image//Assignment//AndroidBody_UV_Texture_HighDef.tga");
	MeshBuilder::GetInstance()->GenerateOBJ("RobotBody_MidDef", "OBJ//Assignment//AndroidBody2_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("RobotBody_MidDef")->textureID = LoadTGA("Image//Assignment//AndroidBody_UV_Texture_MidDef.tga");
	MeshBuilder::GetInstance()->GenerateOBJ("RobotBody_LowDef", "OBJ//Assignment//AndroidBody2_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("RobotBody_LowDef")->textureID = LoadTGA("Image//Assignment//AndroidBody_UV_Texture_LowDef.tga");
	// Hands
	MeshBuilder::GetInstance()->GenerateOBJ("RobotHands_HighDef", "OBJ//Assignment//AndroidHands2_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("RobotHands_HighDef")->textureID = LoadTGA("Image//Assignment//AndroidHands_UV_Texture_HighDef.tga");
	MeshBuilder::GetInstance()->GenerateOBJ("RobotHands_MidDef", "OBJ//Assignment//AndroidHands2_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("RobotHands_MidDef")->textureID = LoadTGA("Image//Assignment//AndroidHands_UV_Texture_MidDef.tga");
	MeshBuilder::GetInstance()->GenerateOBJ("RobotHands_LowDef", "OBJ//Assignment//AndroidHands2_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("RobotHands_LowDef")->textureID = LoadTGA("Image//Assignment//AndroidHands_UV_Texture_LowDef.tga");
	// Head
	MeshBuilder::GetInstance()->GenerateOBJ("RobotHead_HighDef", "OBJ//Assignment//AndroidHead2_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("RobotHead_HighDef")->textureID = LoadTGA("Image//Assignment//AndroidHead_UV_Texture_HighDef.tga");
	MeshBuilder::GetInstance()->GenerateOBJ("RobotHead_MidDef", "OBJ//Assignment//AndroidHead2_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("RobotHead_MidDef")->textureID = LoadTGA("Image//Assignment//AndroidHead_UV_Texture_MidDef.tga");
	MeshBuilder::GetInstance()->GenerateOBJ("RobotHead_LowDef", "OBJ//Assignment//AndroidHead2_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("RobotHead_LowDef")->textureID = LoadTGA("Image//Assignment//AndroidHead_UV_Texture_LowDef.tga");
	// Android Robot in 1 Piece //
	MeshBuilder::GetInstance()->GenerateOBJ("Robot_HighDef", "OBJ//Assignment//CombinedAndroidRobot_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("Robot_HighDef")->textureID = LoadTGA("Image//Assignment//CombinedAndroidRobot_UV_HighDef.tga");
	MeshBuilder::GetInstance()->GenerateOBJ("Robot_MidDef", "OBJ//Assignment//CombinedAndroidRobot_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("Robot_MidDef")->textureID = LoadTGA("Image//Assignment//CombinedAndroidRobot_UV_MidDef.tga");
	MeshBuilder::GetInstance()->GenerateOBJ("Robot_LowDef", "OBJ//Assignment//CombinedAndroidRobot_OBJ.obj");
	MeshBuilder::GetInstance()->GetMesh("Robot_LowDef")->textureID = LoadTGA("Image//Assignment//CombinedAndroidRobot_UV_LowDef.tga");


	// Set up the Spatial Partition and pass it to the EntityManager to manage
	CSpatialPartition::GetInstance()->Init(100, 100, 10, 10);
	CSpatialPartition::GetInstance()->SetMesh("GRIDMESH");
	CSpatialPartition::GetInstance()->SetCamera(&camera);
	CSpatialPartition::GetInstance()->SetLevelOfDetails(10000.0f, 20000.0f);
	EntityManager::GetInstance()->SetSpatialPartition(CSpatialPartition::GetInstance());

	RenderWalls();
	RenderTurrets();
	RenderRobots();
	RenderSpatialPartitionObjects();

	// Create entities into the scene
	Create::Entity("reference", Vector3(0.0f, 0.0f, 0.0f), 99); // Reference
	Create::Entity("lightball", Vector3(lights[0]->position.x, lights[0]->position.y, lights[0]->position.z), 99); // Lightball

	GenericEntity* aCube = Create::Entity("cube", Vector3(-20.0f, 0.0f, -20.0f),3);
	aCube->SetCollider(true);
	aCube->SetAABB(Vector3(0.5f, 0.5f, 0.5f), Vector3(-0.5f, -0.5f, -0.5f));
	aCube->InitLOD("cube", "sphere", "cubeSG");

	GenericEntity* aCube2 = Create::Entity("cube", Vector3(-10.0f, 0.0f, -20.0f),5);
	aCube2->SetScale(Vector3(2.f, 2.f, 2.f));
	aCube2->SetCollider(true);
	aCube2->SetAABB(Vector3(1.f, 1.0f, 1.0f), Vector3(-1.0f, -1.0f, -1.0f));
	aCube2->InitLOD("cube", "sphere", "cubeSG");

	// Add the pointer to this new entity to the Scene Graph
	CSceneNode* theNode = CSceneGraph::GetInstance()->AddNode(aCube);
	CSceneNode* theNode2 = CSceneGraph::GetInstance()->AddNode(aCube2);
	if (theNode == NULL)
	{
		cout << "EntityManager::AddEntity: Unable to add to scene graph!" << endl;
	}

	theRobot = new CAndroidRobot();
	theRobot->Init();

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
	theRobot->SetTerrain(groundEntity);

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

	// Front Wall
	GenericEntity* FrontWall = Create::Entity("Wall2", Vector3(200.0f, m_fPosY_Offset + 50.f, 0.0f), 99);
	FrontWall->SetScale(Vector3(10.f, 50.f, 100.f));
	FrontWall->SetCollider(true);
	FrontWall->SetIsImmortal(true);
	FrontWall->SetAABB(	Vector3(0.2f * FrontWall->GetScale().x, 1.f * FrontWall->GetScale().y, 2.f * FrontWall->GetScale().z),
						Vector3(-(0.2f * FrontWall->GetScale().x), -(1.f * FrontWall->GetScale().y), -(2.f * FrontWall->GetScale().z)));

	// Back Wall
	GenericEntity* BackWall = Create::Entity("Wall2", Vector3(-200.0f, m_fPosY_Offset + 50.f, 0.0f), 99);
	BackWall->SetScale(Vector3(10.f, 50.f, 100.f));
	BackWall->SetCollider(true);
	BackWall->SetIsImmortal(true);
	BackWall->SetAABB(	Vector3(0.2f * BackWall->GetScale().x, 1.f * BackWall->GetScale().y, 2.f * BackWall->GetScale().z),
						Vector3(-(0.2f * BackWall->GetScale().x), -(1.f * BackWall->GetScale().y), -(2.f * BackWall->GetScale().z)));

	// Right Wall
	GenericEntity* RightWall = Create::Entity("Wall", Vector3(0.0f, m_fPosY_Offset + 50.f, 200.0f), 99);
	RightWall->SetScale(Vector3(100.f, 50.f, 10.f));
	RightWall->SetIsImmortal(true);
	RightWall->SetCollider(true);
	RightWall->SetAABB(	Vector3(2.f * RightWall->GetScale().x, 1.f * RightWall->GetScale().y, 0.2f * RightWall->GetScale().z),
						Vector3(-(2.f * RightWall->GetScale().x), -(1.f * RightWall->GetScale().y), -(0.2f * RightWall->GetScale().z)));

	// Left Wall
	GenericEntity* LeftWall = Create::Entity("Wall", Vector3(0.0f, m_fPosY_Offset + 50.f, -200.0f), 99);
	LeftWall->SetScale(Vector3(100.f, 50.f, 10.f));
	LeftWall->SetIsImmortal(true);
	LeftWall->SetCollider(true);
	LeftWall->SetAABB(	Vector3(2.f * LeftWall->GetScale().x, 1.f * LeftWall->GetScale().y, 0.2f * LeftWall->GetScale().z),
						Vector3(-(2.f * LeftWall->GetScale().x), -(1.f * LeftWall->GetScale().y), -(0.2f * LeftWall->GetScale().z)));

	// Ceiling
	GenericEntity* Ceiling = Create::Entity("Wall", Vector3(0.0f, 90.f, 0.0f), Vector3(1.f, 0.f, 0.f), 90.f, 99, Vector3(100.f, 200.f, 10.f));
	Ceiling->SetIsImmortal(true);
	Ceiling->SetCollider(true);
}


void Assignment::RenderTurrets()
{
	float m_fPosY_Offset = -10.f;

	// LOD - Non-Movable Turret //
	// Source
	GenericEntity* TurretSource = Create::Entity("TurretSource_HighDef", Vector3(20.0f, m_fPosY_Offset + 11.f, -100.0f), 1);
	TurretSource->SetScale(Vector3(10.f, 10.f, 10.f));
	TurretSource->SetCollider(true);
	TurretSource->SetAABB(	Vector3(0.24f * TurretSource->GetScale().x * 0.5f, 0.24f * TurretSource->GetScale().y * 0.5f, 0.24f * TurretSource->GetScale().z * 0.5f),
							Vector3(0.24f * TurretSource->GetScale().x * -0.5f, 0.24f * TurretSource->GetScale().y * -0.5f, 0.24f * TurretSource->GetScale().z * -0.5f));
	TurretSource->InitLOD("TurretSource_HighDef", "TurretSource_MidDef", "TurretSource_LowDef");

	// Base
	GenericEntity* TurretBase = Create::Entity("TurretBase_HighDef", Vector3(20.0f, m_fPosY_Offset + 0.5f, -100.0f), 2);
	TurretBase->SetScale(Vector3(20.f, 10.f, 20.f));
	TurretBase->SetCollider(true);
	TurretBase->SetAABB(	Vector3(0.5f * TurretBase->GetScale().x * 0.5f, 0.15f * TurretBase->GetScale().y * 0.5f, 0.5f * TurretBase->GetScale().z * 0.5f),
							Vector3(0.5f * TurretBase->GetScale().x * -0.5f, 0.15f * TurretBase->GetScale().y * -0.5f, 0.5f * TurretBase->GetScale().z * -0.5f));
	TurretBase->InitLOD("TurretBase_HighDef", "TurretBase_MidDef", "TurretBase_LowDef");

	// Stand 
	GenericEntity* TurretStand = Create::Entity("TurretStand_HighDef", Vector3(20.0f, m_fPosY_Offset + 5.f, -100.0f), 3);
	TurretStand->SetScale(Vector3(10.f, 20.f, 10.f));
	TurretStand->SetCollider(true);
	TurretStand->SetAABB(	Vector3(0.2f * TurretStand->GetScale().x * 0.5f, 0.5f * TurretStand->GetScale().y * 0.5f, 0.2f * TurretStand->GetScale().z * 0.5f),
							Vector3(0.2f * TurretStand->GetScale().x * -0.5f, 0.5f * TurretStand->GetScale().y * -0.5f, 0.2f * TurretStand->GetScale().z * -0.5f));
	TurretStand->InitLOD("TurretStand_HighDef", "TurretStand_MidDef", "TurretStand_LowDef");

	// Cursor
	GenericEntity* TurretCursor = Create::Entity("TurretCursor_HighDef", Vector3(20.0f, m_fPosY_Offset + 11.f, -100.f + 3.f), 1);
	TurretCursor->SetScale(Vector3(15.f, 15.f, 15.f));
	TurretCursor->SetCollider(true);
	TurretCursor->SetAABB(	Vector3(0.3f * TurretCursor->GetScale().x * 0.5f, 0.3f * TurretCursor->GetScale().y * 0.5f, 0.2f *TurretCursor->GetScale().z * 0.5f),
							Vector3(0.3f * TurretCursor->GetScale().x * -0.5f, 0.3f * TurretCursor->GetScale().y * -0.5f, 0.2f * TurretCursor->GetScale().z * -0.5f));
	TurretCursor->InitLOD("TurretCursor_HighDef", "TurretCursor_MidDef", "TurretCursor_LowDef");




	// SceneGraph - Non-Movable Turret, Source (Parent) is moving thus the Cursor (Child) is moving too. //
	// Source
	GenericEntity* TurretSource_02 = Create::Asset("TurretSource_HighDef", Vector3(-120.0f, m_fPosY_Offset + 11.f, -100.0f), 1);
	TurretSource_02->SetScale(Vector3(10.f, 10.f, 10.f));
	TurretSource_02->InitLOD("TurretSource_HighDef", "TurretSource_MidDef", "TurretSource_LowDef");

	CSceneNode* theSource_02 = CSceneGraph::GetInstance()->AddNode(TurretSource_02);
	CUpdateTransformation* baseMtx_02 = new CUpdateTransformation();
	baseMtx_02->ApplyUpdate(0.0f, 0.01f, 0.0f);
	baseMtx_02->SetSteps(-40, 40);
	theSource_02->SetUpdateTransformation(baseMtx_02);

	// Base
	GenericEntity* TurretBase_02 = Create::Entity("TurretBase_HighDef", Vector3(-120.0f, m_fPosY_Offset + 0.5f, -100.0f), 2);
	TurretBase_02->SetScale(Vector3(20.f, 10.f, 20.f));
	TurretBase_02->SetCollider(true);
	TurretBase_02->SetAABB(Vector3(TurretBase_02->GetScale().x * 0.5f, TurretBase_02->GetScale().y * 0.5f, TurretBase_02->GetScale().z * 0.5f),
		Vector3(TurretBase_02->GetScale().x * -0.5f, TurretBase_02->GetScale().y * -0.5f, TurretBase_02->GetScale().z * -0.5f));
	TurretBase_02->InitLOD("TurretBase_HighDef", "TurretBase_MidDef", "TurretBase_LowDef");

	// Stand 
	GenericEntity* TurretStand_02 = Create::Entity("TurretStand_HighDef", Vector3(-120.0f, m_fPosY_Offset + 5.f, -100.0f), 3);
	TurretStand_02->SetScale(Vector3(10.f, 20.f, 10.f));
	TurretStand_02->InitLOD("TurretStand_HighDef", "TurretStand_MidDef", "TurretStand_LowDef");

	// Cursor
	GenericEntity* TurretCursor_02 = Create::Asset("TurretCursor_HighDef", Vector3(-120.0f, m_fPosY_Offset + 11.f, -100.0f + 3.f), 1);
	TurretCursor_02->SetScale(Vector3(15.f, 15.f, 15.f));
	TurretCursor_02->InitLOD("TurretCursor_HighDef", "TurretCursor_MidDef", "TurretCursor_LowDef");

	CSceneNode* theCursor_02 = theSource_02->AddChild(TurretCursor_02);



	// SceneGraph - Non-Movable Turrets, Source (Parent) is moving while Cursor (Child) is moving on its own too. //
	// Source
	GenericEntity* TurretSource_03 = Create::Asset("TurretSource_HighDef", Vector3(-150.0f, m_fPosY_Offset + 11.f, -100.0f), 1);
	TurretSource_03->SetScale(Vector3(10.f, 10.f, 10.f));
	TurretSource_03->InitLOD("TurretSource_HighDef", "TurretSource_MidDef", "TurretSource_LowDef");

	CSceneNode* theSource_03 = CSceneGraph::GetInstance()->AddNode(TurretSource_03);
	CUpdateTransformation* baseMtx_03 = new CUpdateTransformation();
	baseMtx_03->ApplyUpdate(0.0f, 0.01f, 0.0f);
	baseMtx_03->SetSteps(-40, 40);
	theSource_03->SetUpdateTransformation(baseMtx_03);

	// Base
	GenericEntity* TurretBase_03 = Create::Entity("TurretBase_HighDef", Vector3(-150.0f, m_fPosY_Offset + 0.5f, -100.0f), 2);
	TurretBase_03->SetScale(Vector3(20.f, 10.f, 20.f));
	TurretBase_03->SetCollider(true);
	TurretBase_03->SetAABB(Vector3(TurretBase_03->GetScale().x * 0.5f, TurretBase_03->GetScale().y * 0.5f, TurretBase_03->GetScale().z * 0.5f),
		Vector3(TurretBase_03->GetScale().x * -0.5f, TurretBase_03->GetScale().y * -0.5f, TurretBase_03->GetScale().z * -0.5f));
	TurretBase_03->InitLOD("TurretBase_HighDef", "TurretBase_MidDef", "TurretBase_LowDef");

	// Stand 
	GenericEntity* TurretStand_03 = Create::Entity("TurretStand_HighDef", Vector3(-150.0f, m_fPosY_Offset + 5.f, -100.0f), 3);
	TurretStand_03->SetScale(Vector3(10.f, 20.f, 10.f));
	TurretStand_03->InitLOD("TurretStand_HighDef", "TurretStand_MidDef", "TurretStand_LowDef");

	// Cursor
	GenericEntity* TurretCursor_03 = Create::Asset("TurretCursor_HighDef", Vector3(-150.0f, m_fPosY_Offset + 11.f, -100.0f + 3.f), 1);
	TurretCursor_03->SetScale(Vector3(15.f, 15.f, 15.f));
	TurretCursor_03->InitLOD("TurretCursor_HighDef", "TurretCursor_MidDef", "TurretCursor_LowDef");

	CSceneNode* theCursor_03 = theSource_03->AddChild(TurretCursor_03);
	CUpdateTransformation* rotateMtx_03 = new CUpdateTransformation();
	rotateMtx_03->ApplyUpdate(0.01f, 0.05f, 0.05f, 0.05f);
	rotateMtx_03->SetSteps(-60, 60);
	theCursor_03->SetUpdateTransformation(rotateMtx_03);

}

void Assignment::RenderRobots()
{
	//float m_fPosY_Offset = -10.f;

	// LOD - Non-Movable Robot //
	// Head - Source
	GenericEntity* RobotHead = Create::Entity("RobotHead_HighDef", Vector3(-20.0f, 5.f, -98.0f), 1);
	RobotHead->SetScale(Vector3(15.f, 15.f, 15.f));
	RobotHead->InitLOD("RobotHead_HighDef", "RobotHead_MidDef", "RobotHead_LowDef");

	// Body 
	GenericEntity* RobotBody = Create::Entity("RobotBody_HighDef", Vector3(-20.0f, -2.f, -98.0f), 3);
	RobotBody->SetScale(Vector3(15.f, 20.f, 15.f));
	RobotBody->InitLOD("RobotBody_HighDef", "RobotBody_MidDef", "RobotBody_LowDef");

	// Hands
	GenericEntity* RobotHands = Create::Entity("RobotHands_HighDef", Vector3(-20.0f, 1.f, -93.0f), 1);
	RobotHands->SetScale(Vector3(10.f, 10.f, 10.f));
	RobotHands->InitLOD("RobotHands_HighDef", "RobotHands_MidDef", "RobotHands_LowDef");

	// Base
	GenericEntity* RobotBase = Create::Entity("RobotBase_HighDef", Vector3(-20.0f, -10.f, -98.0f), 3);
	RobotBase->SetScale(Vector3(20.f, 10.f, 20.f));
	RobotBase->SetCollider(true);
	RobotBase->SetAABB(Vector3(0.25f * RobotBase->GetScale().x, 0.2f * RobotBase->GetScale().y, 0.25f * RobotBase->GetScale().z),
		Vector3(-(0.25f * RobotBase->GetScale().x), -(0.2f * RobotBase->GetScale().y), -(0.25f * RobotBase->GetScale().z)));
	RobotBase->InitLOD("RobotBase_HighDef", "RobotBase_MidDef", "RobotBase_LowDef");




	// Robot 2 - SceneGraph; The Source (Head) is the parent & the rest are children. Only parent is moving. //
	// Head - Source
	GenericEntity* RobotHead_02 = Create::Asset("RobotHead_HighDef", Vector3(-80.0f, 5.f, -100.0f));
	RobotHead_02->SetScale(Vector3(15.f, 15.f, 15.f));

	CSceneNode* theSource_02 = CSceneGraph::GetInstance()->AddNode(RobotHead_02);

	CUpdateTransformation* theBaseMtx_02 = new CUpdateTransformation();
	theBaseMtx_02->ApplyUpdate(0.0f, 0.0f, 0.1f);
	theBaseMtx_02->SetSteps(0, 200);
	theSource_02->SetUpdateTransformation(theBaseMtx_02);

	// Body 
	GenericEntity* RobotBody_02 = Create::Asset("RobotBody_HighDef", Vector3(-80.0f, -2.f, -100.0f));
	RobotBody_02->SetScale(Vector3(15.f, 20.f, 15.f));

	CSceneNode* theBody_02 = theSource_02->AddChild(RobotBody_02);

	// Hands
	GenericEntity* RobotHands_02 = Create::Asset("RobotHands_HighDef", Vector3(-80.0f, 1.f, -95.0f));
	RobotHands_02->SetScale(Vector3(10.f, 10.f, 10.f));

	CSceneNode* theHands_02 = theSource_02->AddChild(RobotHands_02);

	// Base
	GenericEntity* RobotBase_02 = Create::Asset("RobotBase_HighDef", Vector3(-80.f, -10.f, -100.0f));
	RobotBase_02->SetScale(Vector3(20.f, 10.f, 20.f));

	CSceneNode* theBase_02 = theSource_02->AddChild(RobotBase_02);



	// Robot 3 - SceneGraph; The Source (Head) is the parent & the rest are children. Parent is moving while a child (Hands) is moving too.//
	// Head - Source
	GenericEntity* RobotHead_03 = Create::Asset("RobotHead_HighDef", Vector3(-60.0f, 5.f, -100.0f));
	RobotHead_03->SetScale(Vector3(15.f, 15.f, 15.f));

	CSceneNode* theSource_03 = CSceneGraph::GetInstance()->AddNode(RobotHead_03);
	CUpdateTransformation* theBaseMtx_03 = new CUpdateTransformation();
	theBaseMtx_03->ApplyUpdate(0.0f, 0.0f, 0.1f);
	theBaseMtx_03->SetSteps(0, 200);
	theSource_03->SetUpdateTransformation(theBaseMtx_03);

	// Body 
	GenericEntity* RobotBody_03 = Create::Asset("RobotBody_HighDef", Vector3(-60.0f, -2.f, -100.0f));
	RobotBody_03->SetScale(Vector3(15.f, 20.f, 15.f));

	CSceneNode* theBody_03 = theSource_03->AddChild(RobotBody_03);

	// Hands
	GenericEntity* RobotHands_03 = Create::Asset("RobotHands_HighDef", Vector3(-60.0f, 1.f, -95.0f));
	RobotHands_03->SetScale(Vector3(10.f, 10.f, 10.f));

	CSceneNode* theHands_03 = theSource_03->AddChild(RobotHands_03);
	CUpdateTransformation* theChildMtx_03 = new CUpdateTransformation();
	theChildMtx_03->ApplyUpdate(0.1f, 0.0f, 0.0f);
	theChildMtx_03->SetSteps(-10, 10);
	theHands_03->SetUpdateTransformation(theChildMtx_03);

	// Base
	GenericEntity* RobotBase_03 = Create::Asset("RobotBase_HighDef", Vector3(-60.f, -10.f, -100.0f));
	RobotBase_03->SetScale(Vector3(20.f, 10.f, 20.f));

	CSceneNode* theBase_03 = theSource_03->AddChild(RobotBase_03);
}

void Assignment::RenderSpatialPartitionObjects()
{
	float m_fPosY_Offset = -10.f;

	// Turret 1 - Testing Spatial Partitioning w Grenade //
	// Source
	GenericEntity* TurretSource = Create::Entity("TurretSource_HighDef", Vector3(20.0f, m_fPosY_Offset + 11.f, 120.0f),
		Vector3(0.0f, 1.0f, 0.0f), 180.f, 1);
	TurretSource->SetScale(Vector3(10.f, 10.f, 10.f));
	TurretSource->InitLOD("TurretSource_HighDef", "TurretSource_MidDef", "TurretSource_LowDef");

	CSceneNode* theNode = CSceneGraph::GetInstance()->AddNode(TurretSource);

	// Base
	GenericEntity* TurretBase = Create::Entity("TurretBase_HighDef", Vector3(20.0f, m_fPosY_Offset + 0.5f, 120.0f),
		Vector3(0.0f, 1.0f, 0.0f), 180.f, 2);
	TurretBase->SetScale(Vector3(20.f, 10.f, 20.f));
	TurretBase->SetCollider(true);
	TurretBase->SetAABB(Vector3(TurretBase->GetScale().x * 0.5f, TurretBase->GetScale().y * 0.5f, TurretBase->GetScale().z * 0.5f),
		Vector3(TurretBase->GetScale().x * -0.5f, TurretBase->GetScale().y * -0.5f, TurretBase->GetScale().z * -0.5f));
	TurretBase->InitLOD("TurretBase_HighDef", "TurretBase_MidDef", "TurretBase_LowDef");

	CSceneNode* theChild_01 = theNode->AddChild(TurretBase);

	// Stand 
	GenericEntity* TurretStand = Create::Entity("TurretStand_HighDef", Vector3(20.0f, m_fPosY_Offset + 5.f, 120.0f)
		, Vector3(0.0f, 1.0f, 0.0f), 180.f, 3);
	TurretStand->SetScale(Vector3(10.f, 20.f, 10.f));
	TurretStand->InitLOD("TurretStand_HighDef", "TurretStand_MidDef", "TurretStand_LowDef");

	CSceneNode* theChild_02 = theNode->AddChild(TurretStand);

	// Cursor
	GenericEntity* TurretCursor = Create::Entity("TurretCursor_HighDef", Vector3(20.0f, m_fPosY_Offset + 11.f, 120.0f - 3.f)
		, Vector3(0.0f, 1.0f, 0.0f), 180.f, 1);
	TurretCursor->SetScale(Vector3(15.f, 15.f, 15.f));
	TurretCursor->InitLOD("TurretCursor_HighDef", "TurretCursor_MidDef", "TurretCursor_LowDef");

	CSceneNode* theChild_03 = theNode->AddChild(TurretCursor);



	// Turret 2 - Testing Spatial Partitioning w Grenade //
	// Source
	GenericEntity* TurretSource_02 = Create::Entity("TurretSource_HighDef", Vector3(20.0f, m_fPosY_Offset + 11.f, 80.0f),
		Vector3(0.0f, 1.0f, 0.0f), 180.f, 1);
	TurretSource_02->SetScale(Vector3(10.f, 10.f, 10.f));
	TurretSource_02->InitLOD("TurretSource_HighDef", "TurretSource_MidDef", "TurretSource_LowDef");

	CSceneNode* theNode_02 = CSceneGraph::GetInstance()->AddNode(TurretSource_02);

	// Base
	GenericEntity* TurretBase_02 = Create::Entity("TurretBase_HighDef", Vector3(20.0f, m_fPosY_Offset + 0.5f, 80.0f),
		Vector3(0.0f, 1.0f, 0.0f), 180.f, 2);
	TurretBase_02->SetScale(Vector3(20.f, 10.f, 20.f));
	TurretBase_02->SetCollider(true);
	TurretBase_02->SetAABB(Vector3(TurretBase_02->GetScale().x * 0.5f, TurretBase_02->GetScale().y * 0.5f, TurretBase_02->GetScale().z * 0.5f),
		Vector3(TurretBase_02->GetScale().x * -0.5f, TurretBase_02->GetScale().y * -0.5f, TurretBase_02->GetScale().z * -0.5f));
	TurretBase_02->InitLOD("TurretBase_HighDef", "TurretBase_MidDef", "TurretBase_LowDef");

	CSceneNode* theChild_01_ = theNode_02->AddChild(TurretBase_02);

	// Stand 
	GenericEntity* TurretStand_02 = Create::Entity("TurretStand_HighDef", Vector3(20.0f, m_fPosY_Offset + 5.f, 80.0f)
		, Vector3(0.0f, 1.0f, 0.0f), 180.f, 3);
	TurretStand_02->SetScale(Vector3(10.f, 20.f, 10.f));
	TurretStand_02->InitLOD("TurretStand_HighDef", "TurretStand_MidDef", "TurretStand_LowDef");

	CSceneNode* theChild_02_ = theNode_02->AddChild(TurretStand_02);

	// Cursor
	GenericEntity* TurretCursor_02 = Create::Entity("TurretCursor_HighDef", Vector3(20.0f, m_fPosY_Offset + 11.f, 80.0f - 3.f)
		, Vector3(0.0f, 1.0f, 0.0f), 180.f, 1);
	TurretCursor_02->SetScale(Vector3(15.f, 15.f, 15.f));
	TurretCursor_02->InitLOD("TurretCursor_HighDef", "TurretCursor_MidDef", "TurretCursor_LowDef");

	CSceneNode* theChild_03_ = theNode_02->AddChild(TurretCursor_02);

}

void Assignment::SpawnRobots(float x, float y, float z)
{
	// Head - Source
	GenericEntity* RobotHead = Create::Entity("RobotHead_HighDef", Vector3(0.f + x, 5.f + y, 0.f + z), 1);
	RobotHead->SetScale(Vector3(15.f, 15.f, 15.f));
	RobotHead->SetCollider(true);
	RobotHead->SetAABB(Vector3(0.125f * RobotHead->GetScale().x, 0.125f * RobotHead->GetScale().y, 0.125f * RobotHead->GetScale().z),
		Vector3(-(0.125f * RobotHead->GetScale().x), -(0.125f * RobotHead->GetScale().y), -(0.125f * RobotHead->GetScale().z)));
	RobotHead->InitLOD("RobotHead_HighDef", "RobotHead_MidDef", "RobotHead_LowDef");

	// Body 
	GenericEntity* RobotBody = Create::Entity("RobotBody_HighDef", Vector3(0.f + x, -2.f + y, 0.0f + z), 3);
	RobotBody->SetScale(Vector3(15.f, 20.f, 15.f));
	RobotBody->SetCollider(true);
	RobotBody->SetAABB(Vector3(0.2f * RobotBody->GetScale().x, 0.25f * RobotBody->GetScale().y, 0.125f * RobotBody->GetScale().z),
		Vector3(-(0.2f * RobotBody->GetScale().x), -(0.25 * RobotBody->GetScale().y), -(0.125 * RobotBody->GetScale().z)));
	RobotBody->InitLOD("RobotBody_HighDef", "RobotBody_MidDef", "RobotBody_LowDef");

	// Hands
	GenericEntity* RobotHands = Create::Entity("RobotHands_HighDef", Vector3(0.f + x, 1.0f + y, 3.0f + z), 2);
	RobotHands->SetScale(Vector3(10.f, 10.f, 10.f));
	RobotHands->InitLOD("RobotHands_HighDef", "RobotHands_MidDef", "RobotHands_LowDef");

	// Base
	GenericEntity* RobotBase = Create::Entity("RobotBase_HighDef", Vector3(0.f + x, -10.f + y, 0.0f + z), 3);
	RobotBase->SetScale(Vector3(20.f, 10.f, 20.f));
	RobotBase->SetCollider(true);
	RobotBase->SetAABB(Vector3(0.25f * RobotBase->GetScale().x, 0.2f * RobotBase->GetScale().y, 0.25f * RobotBase->GetScale().z),
		Vector3(-(0.25f * RobotBase->GetScale().x), -(0.2f * RobotBase->GetScale().y), -(0.25f * RobotBase->GetScale().z)));
	RobotBase->InitLOD("RobotBase_HighDef", "RobotBase_MidDef", "RobotBase_LowDef");

	CSceneNode* headNode = CSceneGraph::GetInstance()->AddNode(RobotHead);
	CSceneNode* bodyNode = headNode->AddChild(RobotBody);
	CSceneNode* handNode = bodyNode->AddChild(RobotHands);
	CSceneNode* baseNode = bodyNode->AddChild(RobotBase);
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

	// press z to spawn
	if (playerInfo->getSpawnEnemy())
	{
		float x;
		float z;
		x = Math::RandFloatMinMax(90, 160);
		z = Math::RandFloatMinMax(-170, -50);
		SpawnRobots(x, 0, z);
		playerInfo->setSpawnEnemy(false);
	}

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
