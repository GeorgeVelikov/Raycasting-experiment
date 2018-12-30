#include "stdafx.h"

#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>

#include <math.h>>
#include <Windows.h>

#include "Definitions.h"


int main() {
	// game time
	auto timeStamp = std::chrono::system_clock::now();
	auto timeStampDynamic = std::chrono::system_clock::now();

	// buffer
	wchar_t *screen = new wchar_t[SCREEN_PIXEL_COUNT];
	HANDLE consoleBuffer = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, NULL, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(consoleBuffer);
	DWORD bytesWritten = 0;

	// map
	std::wstring map;
	map += L"################################";
	map += L"#...............##.............#";
	map += L"#..........##.....#....#.#.#...#";
	map += L"#..........##......#...........#";
	map += L"#........########...#...#.#....#";
	map += L"#...##....######...............#";
	map += L"#...##......#........#####.....#";
	map += L"#........##.....##.............#";
	map += L"#..#......##.....##............#";
	map += L"#..........##.....##.......##..#";
	map += L"#...#.......##....##......#....#";
	map += L"#...#........##..##.......#.#..#";
	map += L"#...#.........###....##...#....#";
	map += L"#...#####..........#....###..#.#";
	map += L"#.............##...............#";
	map += L"################################";

	// player location and local orientation
	float playerX = 1.5f;
	float playerY = 3.24f;
	float playerYaw = 0.0f;
	bool running = true;

	while (running) {

		timeStampDynamic = std::chrono::system_clock::now();
		std::chrono::duration<float> frameTime = timeStampDynamic - timeStamp; // get the time it takes for a tick
		timeStamp = timeStampDynamic; // reset current time and prepare to measure new frametime on upcoming tick
		float fpsNormalise = frameTime.count(); // allows for consistency in movement independent of FPS

		// movement
		if (GetAsyncKeyState('A')) playerYaw -= float(ROTATIONAL_STEP * fpsNormalise);
		if (GetAsyncKeyState('D')) playerYaw += float(ROTATIONAL_STEP * fpsNormalise);
		if (GetAsyncKeyState('W')) {
			playerX += float(sinf(playerYaw) * MOVEMENT_STEP * fpsNormalise);
			playerY += float(cosf(playerYaw) * MOVEMENT_STEP * fpsNormalise);
			if (map[int(playerY) * MAP_WIDTH + int(playerX)] == '#') {
				playerX -= float(sinf(playerYaw) * MOVEMENT_STEP * fpsNormalise);
				playerY -= float(cosf(playerYaw) * MOVEMENT_STEP * fpsNormalise);
			}
		}
		if (GetAsyncKeyState('S')) {
			playerX -= float(sinf(playerYaw) * MOVEMENT_STEP * fpsNormalise);
			playerY -= float(cosf(playerYaw) * MOVEMENT_STEP * fpsNormalise);
			if (map[int(playerY) * MAP_WIDTH + int(playerX)] == '#') {
				playerX += float(sinf(playerYaw) * MOVEMENT_STEP * fpsNormalise);
				playerY += float(cosf(playerYaw) * MOVEMENT_STEP * fpsNormalise);
			}
		}

		// ray casting
		for (int col = 0; col < SCREEN_WIDTH; col++) {
			float rayAngle = (playerYaw - FOV / 2.f) + (float(col) / float(SCREEN_WIDTH)) * FOV;

			float distanceToWall = 0;
			bool touchWall = false;
			// is a char part of the block's borders
			bool blockBoundary = false;
			
			// setting the eye from which we shoot out our rays
			float eyeX = sinf(rayAngle);
			float eyeY = cosf(rayAngle);

			// incrementing to see if a wall is hit by our rays
			while (!touchWall && distanceToWall < MAX_DEPTH) {
				distanceToWall += RAY_STEP;
				int testX = int(playerX + eyeX * distanceToWall);
				int testY = int(playerY + eyeY * distanceToWall);

				// ray is out of bounds
				if (testX < 0 || testX >= MAP_WIDTH || testY < 0 || testY >= MAP_HEIGHT) {
					touchWall = true;
					distanceToWall = MAX_DEPTH;
				}
				else
					if (map[testY*MAP_WIDTH + testX] == '#') {
						// adds for block border shading, this can be seen in the intended use at 1 block thick diagonal lines on the map
						touchWall = true;
						std::vector<std::pair<float, float>> blockCorners;
						for (int x = 0; x < 2; x++) 
							for (int y = 0; y < 2; y++) {
								float vectorX = float(testX) + x - playerX;
								float vectorY = float(testY) + y - playerY;
								float distance = sqrt(pow(vectorX, 2) + pow(vectorY, 2)); // eucledian distance
								float dot = (eyeX * vectorX / distance) + (eyeY * vectorY / distance);
								blockCorners.push_back(std::make_pair(distance, dot));
							}
						// sort pairs
						std::sort(blockCorners.begin(), blockCorners.end(), [](const std::pair<float, float> &left, const std::pair<float, float> &right) {
							return left.first < right.first;
						});
						for (int i = 0; i < WALL_BOUNDARY_AMOUNT; i++) if (acos(blockCorners.at(i).second) < WALL_BOUNDARY_THRESHOLD) blockBoundary = true;
					}		
			}

			int ceiling = float(SCREEN_HEIGHT / 2.) - SCREEN_HEIGHT / float(distanceToWall);
			int floor = SCREEN_HEIGHT - ceiling;
			short wallShade;
			short floorShade;

			// close walls become darker and those outside of the MAX_DEPTH reach are not rendered, wall shader
			if (distanceToWall <= MAX_DEPTH / 6.f)      wallShade = WALL_BLACK_SHADE;
			else if (distanceToWall < MAX_DEPTH / 4.f)  wallShade = WALL_DARK_SHADE;
			else if (distanceToWall < MAX_DEPTH / 2.f)  wallShade = WALL_MEDIUM_SHADE;
			else if (distanceToWall < MAX_DEPTH)        wallShade = WALL_LIGHT_SHADE;
			else                                        wallShade = CLEAR_SHADE;
			if (blockBoundary)                          wallShade = CLEAR_SHADE;

			for (int row = 0; row < SCREEN_HEIGHT; row++) {
				if (row < ceiling)
					screen[row*SCREEN_WIDTH + col] = CLEAR_SHADE; // sky
				else if(row >= ceiling && row < floor)
					screen[row*SCREEN_WIDTH + col] = wallShade; // wall
				else {
					screen[row*SCREEN_WIDTH + col] = FLOOR_SHADE; // floor
				}
			}
		}

		// set last char to end of str 
		screen[SCREEN_PIXEL_COUNT] = '\0';
		// draw the screen array to the buffer
		WriteConsoleOutputCharacter(consoleBuffer, screen, SCREEN_PIXEL_COUNT, SCREEN_DRAW_START, &bytesWritten);
	}
	return 0;
}