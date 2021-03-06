#include "AndroidRobots.h"
#include "../EntityManager.h"
#include "GraphicsManager.h"
#include "RenderHelper.h"
#include "../Waypoint//WaypointManager.h"

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
	,m_iWayPointIndex(-1)
{
	listOfWaypoints.clear();
}

CAndroidRobot::~CAndroidRobot()
{
}

void CAndroidRobot::Init(void)
{
	// Set the default values
	defaultPosition.Set(-10.0f, -10.0f, -80.0f);
	defaultTarget.Set(-10.0f, -10, 0);
	defaultUp.Set(0, 1, 0);

	//Set up the waypoints
	listOfWaypoints.push_back(0);
	listOfWaypoints.push_back(1);
	listOfWaypoints.push_back(2);

	// Set the current values
	position.Set(-10.0f, -10.0f, -80.0f);
	target.Set(-10.0f, -10, 450);
	scale.Set(20.f, 10.f, 20.f);
	up.Set(0.0f, 1.0f, 0.0f);

	CWaypoint* nextWaypoint = GetNextWaypoint();
	if (nextWaypoint)
		target = nextWaypoint->GetPosition();
	else
		target = Vector3(0, 0, 0);
	cout << "Next target: " << target << endl;

	// Set Boundary
	maxBoundary.Set(1, 1, 1);
	minBoundary.Set(-1, -1, -1);

	// Set speed
	m_dSpeed = 10.0;

	// Initialise the LOD meshes
	InitLOD("Robot_HighDef", "Robot_MidDef", "Robot_LowDef");

	// Initialise the Collider
	this->SetCollider(true);
	this->SetAABB(Vector3(this->GetScale().x * 0.5f, this->GetScale().y * 0.5f, this->GetScale().z * 0.5f), 
		Vector3(this->GetScale().x * -0.5f, this->GetScale().y * -0.5f, this->GetScale().z * -0.5f));

	// Add to EntityManager
	EntityManager::GetInstance()->AddEntity(this, true);

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

CWaypoint* CAndroidRobot::GetNextWaypoint(void)
{
	if ((int)listOfWaypoints.size() > 0)
	{
		m_iWayPointIndex++;
		if (m_iWayPointIndex >= (int)listOfWaypoints.size())
			m_iWayPointIndex = 0;
		return CWaypointManager::GetInstance()->GetWaypoint(listOfWaypoints[m_iWayPointIndex]);
	}
	else
		return NULL;
}

void CAndroidRobot::Update(double dt)
{
	Vector3 viewVector = (target - position).Normalized();
	position += viewVector * (float)m_dSpeed * (float)dt;

	Constrain();
	
	// Update the target
	/*if (position.z > 100.0f)
	{
		target.z = position.z * -1;
		target.x = defaultTarget.x;
	}
	else if (position.z < -100.0f)
	{
		target.z = position.z * -1;
		target.x = defaultTarget.x;
	}*/

	if ((target - position).LengthSquared() < 25.0f)
	{
		CWaypoint* nextWayPoint = GetNextWaypoint();
		if (nextWayPoint)
			target = nextWayPoint->GetPosition();
		else
			target = Vector3(0, 0, 0);
		cout << "Next target: " << target << endl;
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
