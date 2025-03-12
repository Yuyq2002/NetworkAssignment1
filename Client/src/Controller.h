#pragma once
#include "raylibOverlay.h"

class Controller
{
	bool enabled = false;
	raylibOverlay::Vector2 deltaInput;

public:
	bool InputMovement();
	raylibOverlay::Vector2 GetInput();

	bool InputAttack();

	bool InputMouseMove();

	void SetEnabled(bool enabled_);
};

