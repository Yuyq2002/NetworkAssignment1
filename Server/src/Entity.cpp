#include "Entity.h"

void Player::Init(int id_, raylibOverlay::Vector2 position_, raylibOverlay::Color color_)
{
	this->id = id_;
	this->position = position_;
	this->color = color_;
}

void Player::ProcessMovementInput(raylibOverlay::Vector2 input)
{
	if (!enabled) return;

	position.x += input.x * speed;
	position.y += input.y * speed;
}

void Player::ProcessRotationInput(raylibOverlay::Vector2 input)
{
	if (!enabled) return;

	raylibOverlay::Vector2 directionVector = raylibOverlay::Vector2Subtract(input, position);

	rotation = raylibOverlay::RadToDeg(raylibOverlay::atan2f(directionVector.y, directionVector.x));
}

bool Player::ProcessAttackInput()
{
	if (!enabled) return false;

	if (raylibOverlay::SecondsPassedSinceTime(LastShootingTime) >= PLAYER_ATTACK_COOLDOWN)
	{
		LastShootingTime = raylibOverlay::GetTime();
		return true;
	}

	return false;
}

void Player::TakeDamage()
{
	health -= 10;
}

bool Player::CheckHealth()
{
	if (!enabled) return false;

	if (health > 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Player::ResetHealth()
{
	health = PLAYER_HEALTH;
}

void Player::SetEnabled(bool enabled_)
{
	this->enabled = enabled_;
}

bool Player::GetEnabled()
{
	return enabled;
}

void Player::Update()
{
}

void Player::Draw()
{
	if (!enabled) return;
	raylibOverlay::DrawCircle(position, 50, color);
}

float Player::GetRotation()
{
	return rotation;
}

int Entity::GetID()
{
	return id;
}

raylibOverlay::Vector2 Entity::GetPosition()
{
	return position;
}

raylibOverlay::Color Entity::GetColor()
{
	return color;
}

void Entity::SetIsDead(bool isDead_)
{
	this->isDead = isDead_;
}

bool Entity::GetIsDead()
{
	return isDead;
}

void Bullet::Init(int id_, raylibOverlay::Vector2 position_, raylibOverlay::Vector2 direction_, raylibOverlay::Color color_)
{
	this->id = id_;
	this->position = position_;
	this->direction = direction_;
	this->color = color_;
}

void Bullet::Update()
{
	position.x += direction.x * speed;
	position.y += direction.y * speed;
}

void Bullet::Draw()
{
	raylibOverlay::DrawCircle(position, 10, color);
}
