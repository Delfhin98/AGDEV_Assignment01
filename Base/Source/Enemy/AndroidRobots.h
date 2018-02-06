#pragma once
#include "../GenericEntity.h"
#include "../GroundEntity.h"

#include "../Waypoint/Waypoint.h"

#include <vector>
using namespace std;

class Mesh;

class CAndroidRobot : public GenericEntity
{
public:
	CAndroidRobot(void);
	virtual ~CAndroidRobot();

	void Init(void);
	// Reset this AndroidRobot to default
	void Reset(void);
	// Set Position
	void SetPos(const Vector3& pos);
	// Set Target
	void SetTarget(const Vector3& target);
	// Set Up
	void SetUp(const Vector3& up);
	// Set the boundary for android info
	void SetBoundary(Vector3 max, Vector3 min);
	// Set the Terrain for the android info
	void SetTerrain(GroundEntity* m_pTerrain);

	// Get Position
	Vector3 GetPos(void) const;
	// Get Target
	Vector3 GetTarget(void) const;
	// Get Up
	Vector3 GetUp(void) const;
	// Get the terrain for the android info
	GroundEntity* GetTerrain(void);
	//Get next Waypoint for this CEnemy
	CWaypoint* GetNextWaypoint(void);

	// Update
	void Update(double dt = 0.0333f);
	
	// Constrain the position within the borders
	void Constrain(void);
	// Render
	void Render(void);

protected:
	Vector3 defaultPosition, defaultTarget, defaultUp;
	Vector3	target, up;
	Vector3 maxBoundary, minBoundary;
	GroundEntity* m_pTerrain;

	//Vector containing IDs of Waypoints
	vector<int> listOfWaypoints;
	//Current ID of Waypoint
	int m_iWayPointIndex;

	double m_dSpeed;
	double m_dAcceleratino;
};