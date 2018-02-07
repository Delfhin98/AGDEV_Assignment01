#include "Enemy.h"
#include "../EntityManager.h"
#include "GraphicsManager.h"
#include "RenderHelper.h"
#include "MeshBuilder.h"
#include "../Waypoint/WaypointManager.h"

CEnemy::CEnemy()
	: GenericEntity(NULL)
	, defaultPosition(Vector3(0.0f, 0.0f, 0.0f))
	, defaultTarget(Vector3(0.0f, 0.0f, 0.0f))
	, defaultUp(Vector3(0.0f, 0.0f, 0.0f))
	, target(Vector3(0.0f, 0.0f, 0.0f))
	, up(Vector3(0.0f, 0.0f, 0.0f))
	, maxBoundary(Vector3(0.0f, 0.0f, 0.0f))
	, minBoundary(Vector3(0.0f, 0.0f, 0.0f))
	, m_pTerrain(NULL)
	, m_iWayPointIndex(-1)
	, _CurrState(IDLE)
{
	listOfWaypoints.clear();
}


CEnemy::~CEnemy()
{
}

void CEnemy::Init(void)
{
	// Set the default values
	defaultPosition.Set(0, 0, 0);
	defaultTarget.Set(0, 0, 0);
	defaultUp.Set(0, 1, 0);

	// Set up the waypoints
	listOfWaypoints.push_back(0);
	listOfWaypoints.push_back(1);
	listOfWaypoints.push_back(2);

	m_iWayPointIndex = 0;

	// Set the current values
	position.Set(0.0f, 0.0f, 0.0f);
	target.Set(0.0f, 0.0f, 0.0f);
	up.Set(0.0f, 1.0f, 0.0f);

	//CWaypoint* nextWaypoint = GetNextWaypoint();
	//if (nextWaypoint)
	//	target = nextWaypoint->GetPosition();
	//else
	//	target = Vector3(0, 0, 0);

	//cout << "Next target: " << target << endl;

	// Set Boundary
	maxBoundary.Set(1, 1, 1);
	minBoundary.Set(-1, -1, -1);

	// Set speed
	m_dSpeed = 10.0;

	// Set State
	_CurrState = IDLE;

	// Initialise the LOD meshes
	InitLOD("cube", "cube", "cube");

	// Initialise the Collider
	this->SetCollider(true);
	this->SetAABB(Vector3(1, 1, 1), Vector3(-1, -1, -1));

	// Add to EntityManager
	EntityManager::GetInstance()->AddEntity(this, true);

}

// Reset this player instance to default
void CEnemy::Reset(void)
{
	// Set the current values to default values
	position = defaultPosition;
	target = defaultTarget;
	up = defaultUp;
}

// Set position
void CEnemy::SetPos(const Vector3& pos)
{
	position = pos;
}

// Set target
void CEnemy::SetTarget(const Vector3& target)
{
	this->target = target;
}
// Set Up
void CEnemy::SetUp(const Vector3& up)
{
	this->up = up;
}
// Set the boundary for the player info
void CEnemy::SetBoundary(Vector3 max, Vector3 min)
{
	maxBoundary = max;
	minBoundary = min;
}
// Set the terrain for the player info
void CEnemy::SetTerrain(GroundEntity* m_pTerrain)
{
	if (m_pTerrain != NULL)
	{
		this->m_pTerrain = m_pTerrain;

		SetBoundary(this->m_pTerrain->GetMaxBoundary(), this->m_pTerrain->GetMinBoundary());
	}
}

// Get position
Vector3 CEnemy::GetPos(void) const
{
	return position;
}

// Get target
Vector3 CEnemy::GetTarget(void) const
{
	return target;
}
// Get Up
Vector3 CEnemy::GetUp(void) const
{
	return up;
}
// Get the terrain for the player info
GroundEntity* CEnemy::GetTerrain(void)
{
	return m_pTerrain;
}

// Get next Waypoint for this CEnemy
CWaypoint* CEnemy::GetNextWaypoint(void)
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

// Update
void CEnemy::Update(double dt)
{
	//Vector3 viewVector = (target - position).Normalized();
	//position += viewVector * (float)m_dSpeed * (float)dt;

	// Constrain the position
	Constrain();

	// Timer to be used to change state.
	float _Timer = 0.0f;
	_Timer += (float)m_dSpeed * (float)dt;

	// Update on Enemy State(s)
	switch (_CurrState)
	{
	case IDLE:
	{
		// 1. Enemy will not move.
		// 2. Enemy will stay at Original Position or at a Waypoint.
		// 3. Enemy State will change to PATROL based on a timer.
		cout << "In IDLE STATE" << endl;
		if (_Timer > 0.4f)
		{
			_CurrState = PATROL;
		}
		break;
	}
	case PATROL:
	{
		cout << "In PATROL STATE" << endl;

		if (_Timer > 0.8f)
		{
			_Timer = 0.0f;
			_CurrState = IDLE;
		}
		break;
	}
	default:
		cout << "Error! Invalid State." << endl;
	}


	//if ((target - position).LengthSquared() < 25.0f)
	//{
	//	CWaypoint* nextWaypoint = GetNextWaypoint();
	//	if (nextWaypoint)
	//		target = nextWaypoint->GetPosition();
	//	else
	//		target = Vector3(0, 0, 0);
	//	cout << "Next target: " << target << endl;
	//}
}

// Constrain the position within the borders
void CEnemy::Constrain(void)
{
	// Constrain player within the boundary
	if (position.x > maxBoundary.x - 1.0f)
		position.x = maxBoundary.x - 1.0f;
	if (position.z > maxBoundary.z - 1.0f)
		position.z = maxBoundary.z - 1.0f;
	if (position.x < minBoundary.x + 1.0f)
		position.x = minBoundary.x + 1.0f;
	if (position.z < minBoundary.z + 1.0f)
		position.z = minBoundary.z + 1.0f;

	// if the y position is not equal to terrain height at that position, 
	// then update y position to the terrain height
	if (position.y != m_pTerrain->GetTerrainHeight(position))
		position.y = m_pTerrain->GetTerrainHeight(position);
}

// Render
void CEnemy::Render(void)
{
	MS& modelStack = GraphicsManager::GetInstance()->GetModelStack();
	modelStack.PushMatrix();
	modelStack.Translate(position.x, position.y, position.z);
	modelStack.Scale(scale.x, scale.y, scale.z);
	if (GetLODStatus() == true)
	{
		if (theDetailLevel != NO_DETAILS)
		{
			//cout << theDetailLevel << endl;
			RenderHelper::RenderMesh(GetLODMesh());
		}
	}
	modelStack.PopMatrix();
}
