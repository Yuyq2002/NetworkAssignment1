#include "Entity.h"
#include "Entity.h"

void Entity::Init(int id_, raylibOverlay::Vector2 position_, raylibOverlay::Color color_, raylibOverlay::Texture* texture_)
{
	this->id = id_;
	this->position = position_;
	this->color = color_;
	this->texture = texture_;
}

void Entity::Update()
{
	for (auto& point : positionHistory)
	{
		if (point.time < raylibOverlay::GetTime())
		{
			position = point.vector2;
			break;
		}
	}

	if (!positionHistory.empty()) positionHistory.erase(positionHistory.begin());

	for (auto& point : rotationHistory)
	{
		if (point.time < raylibOverlay::GetTime())
		{
			rotation = point.rotation;
			break;
		}
	}

	if (!rotationHistory.empty()) rotationHistory.erase(rotationHistory.begin());
}

void Entity::Draw()
{
	if (!enabled) return;

	if (texture == nullptr)
	{
		raylibOverlay::DrawCircle(position, 10, { 0, 0, 0, 255 });
	}
	else
	{
		raylibOverlay::DrawTexture(*texture, position, { 45, 38 }, rotation, color);
	}
}

int Entity::GetID()
{
	return id;
}

void Entity::AddPosition(raylibOverlay::Vector2 position_, double periodTime)
{
	PositionInTime newPoint;
	newPoint.vector2 = position_;

	positionHistory.push_back(newPoint);

	int index = 0;
	for (auto& point : positionHistory)
	{
		point.time = (periodTime / positionHistory.size()) * index;
		index++;
	}
}

void Entity::AddRotation(float rotation_, double periodTime)
{
	RotationInTime newPoint;
	newPoint.rotation = rotation_;

	rotationHistory.push_back(newPoint);

	int index = 0;
	for (auto& point : rotationHistory)
	{
		point.time = (periodTime / rotationHistory.size())* index;
		index++;
	}
}

void Entity::SetIsDead(bool isDead_)
{
	this->isDead = isDead_;
}

bool Entity::GetIsDead()
{
	return isDead;
}

void Entity::SetEnabled(bool enabled_)
{
	this->enabled = enabled_;
}
