#include "raylibOverlay.h"
#include "raylib.h"
#include "raymath.h"
#include <string>
namespace raylibOverlay
{
	void InitWindow(int width, int height, const char* name)
	{
		RAYLIB_H::InitWindow(width, height, name);
	}

	void SetFPS(int fps)
	{
		RAYLIB_H::SetTargetFPS(fps);
	}

	void DrawFPS()
	{
		RAYLIB_H::DrawFPS(10, 10);
	}

	bool WindowShouldClose()
	{
		if (RAYLIB_H::WindowShouldClose())
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	void BeginDrawing()
	{
		RAYLIB_H::BeginDrawing();
	}

	void ClearBackground(Color color)
	{
		RAYLIB_H::Color rColor;
		rColor.r = color.r;
		rColor.g = color.g;
		rColor.b = color.b;
		rColor.a = color.a;

		RAYLIB_H::ClearBackground(rColor);
	}

	void DrawCircle(Vector2 position, int radius, Color color)
	{
		RAYLIB_H::Color rColor;
		rColor.r = color.r;
		rColor.g = color.g;
		rColor.b = color.b;
		rColor.a = color.a;

		RAYLIB_H::DrawCircleV({ position.x, position.y }, (float)radius, rColor);
	}

	void DrawTextV(const char* text, raylibOverlay::Vector2 position, int fontSize, raylibOverlay::Color color)
	{
		RAYLIB_H::Color rColor;
		rColor.r = color.r;
		rColor.g = color.g;
		rColor.b = color.b;
		rColor.a = color.a;

		RAYLIB_H::DrawText(text, (int)position.x, (int)position.y, fontSize, rColor);
	}

	void DrawTexture(Texture texture, Vector2 position, Vector2 origin, float rotation, Color tint)
	{
		RAYLIB_H::Texture rTexture;

		rTexture.id = texture.id;
		rTexture.width = texture.width;
		rTexture.height = texture.height;
		rTexture.mipmaps = texture.mipmaps;
		rTexture.format = texture.format;

		RAYLIB_H::Rectangle src = { 0, 0, (float)rTexture.width, (float)rTexture.height };
		RAYLIB_H::Rectangle dest = { position.x, position.y, 100, 77 };

		RAYLIB_H::Vector2 rOrigin = { origin.x, origin.y };

		RAYLIB_H::Color rTint = { tint.r, tint.g, tint.b, tint.a };

		RAYLIB_H::DrawTexturePro(rTexture, src, dest, rOrigin, rotation, rTint);
	}

	void EndDrawing()
	{
		RAYLIB_H::EndDrawing();
	}

	bool IsKeyPressed(int key)
	{
		if (RAYLIB_H::IsKeyPressed(key))
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool IsKeyDown(int key)
	{
		if (RAYLIB_H::IsKeyDown(key))
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool IsButtonDown(int button)
	{
		if (RAYLIB_H::IsMouseButtonDown(button))
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	raylibOverlay::Vector2 GetMousePosition()
	{
		RAYLIB_H::Vector2 mousePosition = RAYLIB_H::GetMousePosition();
		return { mousePosition.x, mousePosition.y };
	}

	raylibOverlay::Vector2 GetMouseDelta()
	{
		RAYLIB_H::Vector2 rVec = RAYLIB_H::GetMouseDelta();

		return { rVec.x, rVec.y };
	}

	raylibOverlay::Vector2 Vector2Subtract(raylibOverlay::Vector2 vec1, raylibOverlay::Vector2 vec2)
	{
		RAYLIB_H::Vector2 rVec1 = { vec1.x, vec1.y };
		RAYLIB_H::Vector2 rVec2 = { vec2.x, vec2.y };

		RAYLIB_H::Vector2 result = RAYMATH_H::Vector2Subtract(rVec1, rVec2);

		return { result.x, result.y };
	}

	raylibOverlay::Vector2 Vector2Normalize(raylibOverlay::Vector2 vector)
	{
		RAYLIB_H::Vector2 rVec = { vector.x, vector.y };

		RAYLIB_H::Vector2 result = Vector2Normalize(rVec);

		return { result.x, result.y };
	}

	float atan2f(float y, float x)
	{
		return RAYMATH_H::atan2f(y, x);
	}

	float RadToDeg(float rad)
	{
		return rad * RAD2DEG;
	}

	bool CheckCollisionCircles(Vector2 center1, float radius1, Vector2 center2, float radius2)
	{
		RAYLIB_H::Vector2 rCenter1 = { center1.x, center1.y };
		RAYLIB_H::Vector2 rCenter2 = { center2.x, center2.y };

		return RAYLIB_H::CheckCollisionCircles(rCenter1, radius1, rCenter2, radius2);
	}

	Texture LoadTexture(const char* fileName)
	{
		RAYLIB_H::Texture newTexture = RAYLIB_H::LoadTexture(fileName);

		raylibOverlay::Texture convertedTexture;
		convertedTexture.id = newTexture.id;
		convertedTexture.width = newTexture.width;
		convertedTexture.height = newTexture.height;
		convertedTexture.mipmaps = newTexture.mipmaps;
		convertedTexture.format = newTexture.format;

		return convertedTexture;
	}

	void UnloadTexture(Texture texture)
	{
		RAYLIB_H::Texture textureToUnload;

		textureToUnload.id = texture.id;
		textureToUnload.width = texture.width;
		textureToUnload.height = texture.height;
		textureToUnload.mipmaps = texture.mipmaps;
		textureToUnload.format = texture.format;

		RAYLIB_H::UnloadTexture(textureToUnload);
	}

	double GetTime()
	{
		return RAYLIB_H::GetTime();
	}

	double SecondsPassedSinceTime(double time)
	{
		return RAYLIB_H::GetTime() - time;
	}

	int GetScreenWidth()
	{
		return RAYLIB_H::GetScreenWidth();
	}

	int GetScreenHeight()
	{
		return RAYLIB_H::GetScreenHeight();
	}

	void CloseWindow()
	{
		RAYLIB_H::CloseWindow();
	}

	Color::Color()
	{
	}

	Color::Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}
}