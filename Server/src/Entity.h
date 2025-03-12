#pragma once
#include "raylibOverlay.h"

#define PLAYER_ATTACK_COOLDOWN 1
#define PLAYER_HEALTH 100

class Entity
{
protected:
	float speed = 5; 
	int id;
	raylibOverlay::Vector2 position;
	raylibOverlay::Color color;
	bool isDead = false;

public:
	virtual void Update() = 0;
	virtual void Draw() = 0;

	int GetID();
	raylibOverlay::Vector2 GetPosition();
	raylibOverlay::Color GetColor();
	void SetIsDead(bool isDead_);
	bool GetIsDead();
};

class Player : public Entity
{
	float rotation = 0;
	double LastShootingTime = 0;
	int health = PLAYER_HEALTH;
	bool enabled = true;
public:
	void Init(int id_, raylibOverlay::Vector2 position_, raylibOverlay::Color color_);
	void ProcessMovementInput(raylibOverlay::Vector2 input);
	void ProcessRotationInput(raylibOverlay::Vector2 input);
	bool ProcessAttackInput();
	void TakeDamage();
	bool CheckHealth();
	void ResetHealth();
	void SetEnabled(bool enabled_);
	bool GetEnabled();
	void Update() override;
	void Draw() override;

	float GetRotation();
};

class Bullet : public Entity
{
	raylibOverlay::Vector2 direction;
public:
	void Init(int id_, raylibOverlay::Vector2 position_, raylibOverlay::Vector2 direction_, raylibOverlay::Color color_);
	void Update() override;
	void Draw() override;
};