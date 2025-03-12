#pragma once

namespace raylibOverlay
{
	struct Color
	{
		unsigned char r = 0, g = 0, b = 0, a = 0;

		Color();
		Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
	};

	struct Vector2
	{
		float x = 0, y = 0;
	};

	struct Texture {
		unsigned int id;
		int width;
		int height;
		int mipmaps;
		int format;
	};

	//Initializer
	void InitWindow(int width, int height, const char* name);

	//FPS
	void SetFPS(int fps);
	void DrawFPS();

	//Checks
	bool WindowShouldClose();

	//Draw
	void BeginDrawing();
	void ClearBackground(Color color);
	void DrawCircle(Vector2 position, int radius, Color color);
	void DrawTextV(const char* text, raylibOverlay::Vector2 position, int fontSize, raylibOverlay::Color color);
	void DrawTexture(Texture texture, Vector2 position, Vector2 origin, float rotation, Color tint);
	void EndDrawing();

	//Input
	bool IsKeyPressed(int key);
	bool IsKeyDown(int key);
	bool IsButtonDown(int button);

	//Get
	raylibOverlay::Vector2 GetMousePosition();
	raylibOverlay::Vector2 GetMouseDelta();
	int GetScreenWidth();
	int GetScreenHeight();

	//Calculation
	raylibOverlay::Vector2 Vector2Subtract(raylibOverlay::Vector2 vec1, raylibOverlay::Vector2 vec2);
	raylibOverlay::Vector2 Vector2Normalize(raylibOverlay::Vector2 vector);
	float atan2f(float y, float x);
	float RadToDeg(float rad);

	//Collision check
	bool CheckCollisionCircles(Vector2 center1, float radius1, Vector2 center2, float radius2);

	//Texture
	Texture LoadTexture(const char* fileName);
	void UnloadTexture(Texture texture);

	//Time
	double GetTime();
	double SecondsPassedSinceTime(double time);

	//Clear
	void CloseWindow();
}