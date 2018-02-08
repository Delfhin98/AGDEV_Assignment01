#pragma once
#include "../GenericEntity.h"
#include "../GroundEntity.h"
#include "../Waypoint/Waypoint.h"
#include "../PlayerInfo/PlayerInfo.h"

#include <vector>
#include <string>
using namespace std;

class Mesh;

class CEnemy : public GenericEntity
{
	// Enemy State
	enum ENEMY_STATE
	{
		NONE = 0,
		IDLE,
		PATROL,
		CHASE
	};

protected:
	Vector3 defaultPosition, defaultTarget, defaultUp;
	Vector3 target, up;
	Vector3 maxBoundary, minBoundary;
	GroundEntity* m_pTerrain;

	// Vector containing IDs of Waypoints
	vector<int> listOfWaypoints;
	// Current ID of Waypoint
	int m_iWayPointIndex;

	double m_dSpeed;
	double m_dAcceleration;

	float _Timer;
	// PlayerInfo
	CPlayerInfo* _playerInfo;
	// Initial Enemy State
	ENEMY_STATE _CurrState;
	ENEMY_STATE _PrevState;

public:
	CEnemy(void);
	virtual ~CEnemy();

	void Init(void);
	// Reset this player instance to default
	void Reset(void);

	// Set position
	void SetPos(const Vector3& pos);
	// Set target
	void SetTarget(const Vector3& target);
	// Set Up
	void SetUp(const Vector3& up);
	// Set the boundary for the player info
	void SetBoundary(Vector3 max, Vector3 min);
	// Set the terrain for the player info
	void SetTerrain(GroundEntity* m_pTerrain);

	// Get position
	Vector3 GetPos(void) const;
	// Get target
	Vector3 GetTarget(void) const;
	// Get Up
	Vector3 GetUp(void) const;
	// Get the terrain for the player info
	GroundEntity* GetTerrain(void);
	// Get next Waypoint for this CEnemy
	CWaypoint* GetNextWaypoint(void);

	// Update
	void Update(double dt = 0.0333f);

	// Constrain the position within the borders
	void Constrain(void);
	// Render
	void Render(void);
};