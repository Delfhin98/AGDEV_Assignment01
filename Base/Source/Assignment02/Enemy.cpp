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
	, _Timer(0.0f)
	, _playerInfo(NULL)
{
	listOfWaypoints.clear();
}


CEnemy::~CEnemy()
{
}

void CEnemy::Init(void)
{
	// Set the default values
	defaultPosition.Set(0.0f, 0.0f, 0.0f);
	defaultTarget.Set(0.0f, 0.0f, 0.0f);
	defaultUp.Set(0, 1, 0);

	// Set up the waypoints
	listOfWaypoints.push_back(0);
	listOfWaypoints.push_back(1);
	listOfWaypoints.push_back(2);

	m_iWayPointIndex = 0;

	// Set the current values
	position.Set(10.0f, 0.0f, 10.0f);
	target.Set(10.0f, 0.0f, 10.0f);
	up.Set(0.0f, 1.0f, 0.0f);

	CWaypoint* nextWaypoint = GetNextWaypoint();
	if (nextWaypoint)
		target = nextWaypoint->GetPosition();
	else
		target = Vector3(0, 0, 0);

	// Set Boundary
	maxBoundary.Set(1, 1, 1);
	minBoundary.Set(-1, -1, -1);

	// Set speed
	m_dSpeed = 10.0;

	// Set State
	_CurrState = IDLE;

	// Set PlayerInfo Pos and Target
	_playerInfo = CPlayerInfo::GetInstance();
	_playerInfo->Init();

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
	// Constrain the position
	Constrain();

	// Timer to be used to change state.
	_Timer += (float)m_dSpeed * (float)dt * 0.1f;

	// Update on Enemy State(s)
	switch (_CurrState)
	{
	case IDLE:
	{
		// 1. Enemy will not move.
		// 2. Enemy will move to Original Position or to a Waypoint.
		if (position != defaultPosition)
		{
			Vector3 viewVector = (target - position).Normalized();
			position += viewVector * (float)m_dSpeed * (float)dt;

			target = defaultTarget;
		}

		// 3. Enemy State will change to CHASE when Player is within its range.
		float _dist = (_playerInfo->GetPos() - position).Length();

		if (_dist < 20.0f)
		{
			_CurrState = CHASE;
			cout << "Changing to CHASE STATE" << endl;
		}
		// 4. Otherwise if Player is not in range, Enemy State will change to PATROL based on a timer.
		else if (_dist > 20.0f && _Timer > 20.f)
		{
			_CurrState = PATROL;
			cout << "Changing to PATROL STATE" << endl;
		}

		break;
	}
	case PATROL:
	{
		// 1. Enemy can move.
		m_dSpeed = 10.0f;
		Vector3 viewVector = (target - position).Normalized();
		position += viewVector * (float)m_dSpeed * (float)dt;

		// 2. Enemy will move around from Waypoint(s) to Waypoint(s).
		if ((target - position).LengthSquared() < 25.0f)
		{
			CWaypoint* nextWaypoint = GetNextWaypoint();
			if (nextWaypoint)
				target = nextWaypoint->GetPosition();
			else
				target = defaultTarget;
		}

		// 3. Enemy State will change to IDLE based on a timer.
		if (_Timer > 50.f)
		{
			_CurrState = IDLE;
			_Timer = 0.0f;
			cout << "Changing to IDLE STATE" << endl;
		}

		// 4. Enemy State will change to CHASE if Player is within its range.
		float _dist = (_playerInfo->GetPos() - position).Length();

		if (_dist < 20.0f)
		{
			_CurrState = CHASE;
			cout << "Changing to CHASE STATE" << endl;
		}

		break;
	}
	case CHASE:
	{
		// 1. Enemy can move.
		m_dSpeed = 10.0f;
		Vector3 viewVector = (target - position).Normalized();
		position += viewVector * (float)m_dSpeed * (float)dt;

		// 2. Enemy will chase the Player.
		target = _playerInfo->GetPos() - Vector3(2.f, 2.f, 2.f);
		
		// 3. Enemy State will change to IDLE when Player is out of its range.
		float _dist = (_playerInfo->GetPos() - position).Length();

		if (_dist > 20.0f)
		{
			_CurrState = IDLE;
			_Timer = 0.0f;
			cout << "Changing to IDLE STATE" << endl;
		}

		break;
	}
	default:
		cout << "Error! Invalid State." << endl;
	}
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
