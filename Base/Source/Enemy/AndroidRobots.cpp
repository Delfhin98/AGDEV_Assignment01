#include "AndroidRobots.h"
#include "../EntityManager.h"
#include "GraphicsManager.h"
#include "RenderHelper.h"

CAndroidRobot::CAndroidRobot(void)
	: GenericEntity(NULL)
	, defaultPosition(Vector3(0.0f, -10.0f, 0.0f))
	, defaultTarget(Vector3(0.0f, -10.0f, 0.0f))
	, defaultUp(Vector3(0.0f, 0.0f, 0.0f))
	, target(Vector3(0.0f, 0.0f, 0.0f))
	, up(Vector3(0.0f, 0.0f, 0.0f))
	, maxBoundary(Vector3(0.0f, 0.0f, 0.0f))
	, minBoundary(Vector3(0.0f, 0.0f, 0.0f))
	, m_pTerrain(NULL)
{
}

CAndroidRobot::~CAndroidRobot()
{
}

void CAndroidRobot::Init(void)
{
	//// Set the default values
	//defaultPosition.Set(50.0f, -10.0f, -80.0f);
	//defaultTarget.Set(50.0f, -10, 0);
	//defaultUp.Set(0, 1, 0);

	//// Set the current values
	//position.Set(50.0f, -10.0f, -80.0f);
	//target.Set(50.0f, -10, 450);
	//scale.Set(20.f, 10.f, 20.f);
	//up.Set(0.0f, 1.0f, 0.0f);

	// Set Boundary
	maxBoundary.Set(1, 1, 1);
	minBoundary.Set(-1, -1, -1);

	// Set speed
	m_dSpeed = 10.0;

	// Initialise the LOD meshes
	//InitLOD("RobotBase_HighDef", "RobotBase_MidDef", "RobotBase_LowDef");

	// LOD - Non-Movable Robot //
	float m_fPosY_Offset = -10.f;
	// Head - Source
	GenericEntity* RobotHead = Create::Entity("RobotHead_HighDef", Vector3(50.f, m_fPosY_Offset - 2.f, -80.0f));
	RobotHead->SetScale(Vector3(15.f, 15.f, 15.f));
	RobotHead->InitLOD("RobotHead_HighDef", "RobotHead_MidDef", "RobotHead_LowDef");

	// Body 
	GenericEntity* RobotBody = Create::Entity("RobotBody_HighDef", Vector3(50.f, m_fPosY_Offset - 7.f, -80.0f));
	RobotBody->SetScale(Vector3(15.f, 20.f, 15.f));
	RobotBody->InitLOD("RobotBody_HighDef", "RobotBody_MidDef", "RobotBody_LowDef");

	// Hands
	GenericEntity* RobotHands = Create::Entity("RobotHands_HighDef", Vector3(50.f, m_fPosY_Offset, -75.0f));
	RobotHands->SetScale(Vector3(10.f, 10.f, 10.f));
	RobotHands->InitLOD("RobotHands_HighDef", "RobotHands_MidDef", "RobotHands_LowDef");

	// Base
	GenericEntity* RobotBase = Create::Entity("RobotBase_HighDef", Vector3(50.f, m_fPosY_Offset, -80.0f));
	RobotBase->SetScale(Vector3(20.f, 10.f, 20.f));
	RobotBase->SetCollider(true);
	RobotBase->SetAABB(Vector3(RobotBase->GetScale().x * 0.5f, RobotBase->GetScale().y * 0.5f, RobotBase->GetScale().z * 0.5f),
		Vector3(RobotBase->GetScale().x * -0.5f, RobotBase->GetScale().y * -0.5f, RobotBase->GetScale().z * -0.5f));
	RobotBase->InitLOD("RobotBase_HighDef", "RobotBase_MidDef", "RobotBase_LowDef");


	//// Initialise the Collider
	//this->SetCollider(true);
	//this->SetAABB(Vector3(this->GetScale().x * 0.5f, this->GetScale().y * 0.5f, this->GetScale().z * 0.5f), 
	//	Vector3(this->GetScale().x * -0.5f, this->GetScale().y * -0.5f, this->GetScale().z * -0.5f));

	//// Add to EntityManager
	//EntityManager::GetInstance()->AddEntity(this, true);
}

void CAndroidRobot::Reset(void)
{
	// Set the current values to default values
	position = defaultPosition;
	target = defaultTarget;
	up = defaultUp;
}

void CAndroidRobot::SetPos(const Vector3 & pos)
{
	position = pos;
}

void CAndroidRobot::SetTarget(const Vector3 & target)
{
	this->target = target;
}

void CAndroidRobot::SetUp(const Vector3 & up)
{
	this->up = up;
}

void CAndroidRobot::SetBoundary(Vector3 max, Vector3 min)
{
	maxBoundary = max;
	minBoundary = min;
}

void CAndroidRobot::SetTerrain(GroundEntity * m_pTerrain)
{
	if (m_pTerrain != NULL)
	{
		this->m_pTerrain = m_pTerrain;

		SetBoundary(this->m_pTerrain->GetMaxBoundary(), this->m_pTerrain->GetMinBoundary());
	}
}

Vector3 CAndroidRobot::GetPos(void) const
{
	return position;
}

Vector3 CAndroidRobot::GetTarget(void) const
{
	return target;
}

Vector3 CAndroidRobot::GetUp(void) const
{
	return up;
}

GroundEntity * CAndroidRobot::GetTerrain(void)
{
	return m_pTerrain;
}

void CAndroidRobot::Update(double dt)
{
	Vector3 viewVector = (target - position).Normalized();
	position += viewVector * (float)m_dSpeed * (float)dt;

	Constrain();

	// Update the target
	if (position.z > 100.0f)
	{
		target.z = position.z * -1;
		target.x = 50.0f;
	}
	else if (position.z < -100.0f)
	{
		target.z = position.z * -1;
		target.x = 50.0f;
	}
}

void CAndroidRobot::Constrain(void)
{
	if (position.x > maxBoundary.x - 1.0f)
		position.x = maxBoundary.x - 1.0f;
	if (position.z > maxBoundary.z - 1.0f)
		position.z = maxBoundary.z - 1.0f;
	if (position.x < minBoundary.x + 1.0f)
		position.x = minBoundary.x + 1.0f;
	if (position.z < minBoundary.z + 1.0f)
		position.z = minBoundary.z + 1.0f;

	//if (position.y != m_pTerrain->GetTerrainHeight(position))
	//	position.y = m_pTerrain->GetTerrainHeight(position);
}

void CAndroidRobot::Render(void)
{
	MS& modelStack = GraphicsManager::GetInstance()->GetModelStack();
	modelStack.PushMatrix();
	modelStack.Translate(position.x, position.y, position.z);
	modelStack.Scale(scale.x, scale.y, scale.z);

	if (GetLODStatus() == true)
	{
		if (theDetailLevel != NO_DETAILS)
		{
			RenderHelper::RenderMesh(GetLODMesh());
		}
	}

	modelStack.PopMatrix();
}
