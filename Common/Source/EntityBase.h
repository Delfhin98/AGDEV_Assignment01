#ifndef ENTITY_BASE_H
#define ENTITY_BASE_H

#include "Vector3.h"

class EntityBase
{
public:
	EntityBase();
	virtual ~EntityBase();

	virtual void Update(double _dt);
	virtual void Render();
	virtual void RenderUI();

	inline void SetPosition(const Vector3& _value){ position = _value; };
	inline Vector3 GetPosition(){ return position; };

	inline void SetScale(const Vector3& _value){ scale = _value; };
	inline Vector3 GetScale(){ return scale; };

	inline void SetAngle(const float& _value) { angle = _value; };
	inline float GetAngle() { return angle; };

	inline void SetRotate(const Vector3& _value) { rotate = _value; };
	inline Vector3 GetRotate() { return rotate; }

	bool IsDone();
	void SetIsDone(const bool _value);

	// Check if this entity has a collider class parent
	virtual bool HasCollider(void) const;
	// Set the flag to indicate if this entity has a collider class parent
	virtual void SetCollider(const bool _value);
	// Set the flag, bLaser
	virtual void SetIsLaser(const bool bLaser);
	// Get the flag, bLaser
	virtual bool GetIsLaser(void) const;

	// Set if is projectile
	virtual void SetIsProjectile(const bool isProjectile);
	// Get if is projectile
	virtual bool GetIsProjectile(void) const;

	// Set if is immortal
	virtual void SetIsImmortal(const bool isImmortal);
	// Get if is immortal
	virtual bool GetIsImmortal(void) const;

	//Set HP
	virtual void SetHP(int HP);
	//Get HP
	virtual int GetHP(void);
	//Add HP
	virtual void AddHP(int HP);

protected:
	Vector3 position;
	Vector3 scale;
	Vector3 rotate;

	float angle;

	int HP;

	bool isDone;
	bool m_bCollider;
	bool bLaser;
	bool isProjectile;
	bool isImmortal;
};

#endif // ENTITY_BASE_H