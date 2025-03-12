#include "Controller.h"

bool Controller::InputMovement()
{
	if (!enabled) return false;

	deltaInput = { 0, 0 };

	// A key
	if (raylibOverlay::IsKeyDown(65))
	{
		deltaInput.x--;
	}

	// S key
	if (raylibOverlay::IsKeyDown(68))
	{
		deltaInput.x++;
	}

	// W key
	if (raylibOverlay::IsKeyDown(87))
	{
		deltaInput.y--;
	}

	// S key
	if (raylibOverlay::IsKeyDown(83))
	{
		deltaInput.y++;
	}

	if (deltaInput.x != 0 || deltaInput.y != 0) return true;
	else return false;
}

raylibOverlay::Vector2 Controller::GetInput()
{
	return deltaInput;
}

bool Controller::InputAttack()
{
	if (!enabled) return false;
	if (raylibOverlay::IsButtonDown(0))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Controller::InputMouseMove()
{
	raylibOverlay::Vector2 mouseDelta = raylibOverlay::GetMouseDelta();
	if (mouseDelta.x != 0 || mouseDelta.y != 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Controller::SetEnabled(bool enabled_)
{
	this->enabled = enabled_;
}
