#pragma once
#include "raylibOverlay.h"
#include <vector>

struct PositionInTime
{
	double time;
	raylibOverlay::Vector2 vector2;
};

struct RotationInTime
{
	float rotation;
	double time;
};

class Entity
{
	int id;
	raylibOverlay::Vector2 position;
	raylibOverlay::Color color;
	raylibOverlay::Texture* texture;
	bool isDead;
	float rotation = 0;
	bool enabled = true;

	std::vector<PositionInTime> positionHistory;
	std::vector<struct RotationInTime> rotationHistory;

public:
	void Init(int id_, raylibOverlay::Vector2 position_, raylibOverlay::Color color_, raylibOverlay::Texture* texture_);
	void Update();
	void Draw();

	int GetID();
	void AddPosition(raylibOverlay::Vector2 position_, double periodTime);
	void AddRotation(float rotation_, double periodTime);
	void SetIsDead(bool isDead_);
	bool GetIsDead();
	void SetEnabled(bool enabled_);
};

