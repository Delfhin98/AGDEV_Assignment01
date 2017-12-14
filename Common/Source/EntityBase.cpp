#include "EntityBase.h"

EntityBase::EntityBase()
	: position(0.0f, 0.0f, 0.0f)
	, scale(1.0f, 1.0f, 1.0f)
	, isDone(false)
	, m_bCollider(false)
	, bLaser(false)
	, isProjectile(false)
	, isImmortal(false)
	, angle(0.f)
	, HP(1)
	, rotate(0.f, 1.f, 0.f)
{
}

EntityBase::~EntityBase()
{
}

void EntityBase::Update(double _dt)
{
}

void EntityBase::Render()
{
}

void EntityBase::RenderUI()
{
}

bool EntityBase::IsDone()
{
	return isDone;
}

void EntityBase::SetIsDone(bool _value)
{
	isDone = _value;
}

// Check if this entity has a collider class parent
bool EntityBase::HasCollider(void) const
{
	return m_bCollider;
}

// Set the flag to indicate if this entity has a collider class parent
void EntityBase::SetCollider(const bool _value)
{
	m_bCollider = _value;
}

// Set the flag, bLaser
void EntityBase::SetIsLaser(const bool bLaser)
{
	this->bLaser = bLaser;
}

// Get the flag, bLaser
bool EntityBase::GetIsLaser(void) const
{
	return bLaser;
}

// Set if is projectile
void EntityBase::SetIsProjectile(const bool isProjectile)
{
	this->isProjectile = isProjectile;
}

// Get if is projectile
bool EntityBase::GetIsProjectile(void) const
{
	return isProjectile;
}

// Set if is immortal
void EntityBase::SetIsImmortal(const bool isImmortal)
{
	this->isImmortal = isImmortal;
}

// Get if is immortal
bool EntityBase::GetIsImmortal(void) const
{
	return isImmortal;
}

//Set HP
void  EntityBase::SetHP(int HP)
{
	this->HP = HP;
}
//Get HP
int EntityBase::GetHP(void)
{
	return HP;
}
//Add HP
void EntityBase::AddHP(int HP)
{
	this->HP += HP;
}