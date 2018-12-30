#include "stdafx.h"
#include <iostream>
#include <Windows.h>

#define PI 3.1415927

#define SCREEN_WIDTH 120
#define SCREEN_HEIGHT 40
#define SCREEN_PIXEL_COUNT (SCREEN_WIDTH*SCREEN_HEIGHT)
#define SCREEN_DRAW_START {0,0}

#define FOV (PI/4.0)
#define MAX_DEPTH 16

#define MAP_HEIGHT 16
#define MAP_WIDTH 16

// player location and local orientation
float playerX = 0.0f;
float playerY = 0.0f;
float playerYaw = 0.0f;
bool running = true;


int main()
{
	// buffer
	wchar_t *screen = new wchar_t[SCREEN_PIXEL_COUNT];
	HANDLE consoleBuffer = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(consoleBuffer);
	DWORD bytesWritten = 0;

	std::wstring map;
	map += L"################";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"################";

	while (running)
	{
		for (int col = 0; col < SCREEN_WIDTH; col++)
		{
			float rayAngle = (playerYaw - FOV / 2.0f) + ((float)col / float(SCREEN_WIDTH)) * FOV;
			float distanceToWall = 0;
			bool touchWall = false;
			
			float eyeX = sinf(rayAngle);
			float eyeY = cosf(rayAngle);

			while (!touchWall && distanceToWall < MAX_DEPTH)
			{
				distanceToWall += .1f;
				int testX = (int)(playerX + eyeX * distanceToWall);
				int testY = (int)(playerY + eyeY * distanceToWall);

				// ray is out of bounds
				if (testX < 0 || testX > MAP_WIDTH || testY < 0 || testY > MAP_HEIGHT)
				{
					touchWall = true;
					distanceToWall = MAX_DEPTH;
				}
				else
				{
					if (map[testY*MAP_WIDTH + testX] == '#')
						touchWall = true;
				}
			}
		}
		// set last char to end of str
		screen[SCREEN_PIXEL_COUNT - 1] = '\0';
		WriteConsoleOutputCharacter(consoleBuffer, screen, SCREEN_PIXEL_COUNT, SCREEN_DRAW_START, &bytesWritten);
	}
	return 0;
}