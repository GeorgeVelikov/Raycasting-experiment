#include "stdafx.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>
#include <math.h>

#include <Windows.h>

#include "headers/UNICODEDefinitions.h"
#include "headers/UNICODEFunctions.h"

int main2() {
    player.location = { 1.5f, 3.2f };
    player.cameraYaw = { 0.f, 0.f };
    player.touchWall = false;
    consoleName = GetForegroundWindow();
    setWindowCentered();

	// buffer
	screen = new wchar_t[SCREEN_PIXEL_COUNT];
	consoleBuffer = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, NULL, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleScreenBufferSize(consoleBuffer, COORD {short(SCREEN_WIDTH), short(SCREEN_HEIGHT)} );
    SetConsoleActiveScreenBuffer(consoleBuffer);
	DWORD bytesWritten = 0;

    cfi.cbSize = sizeof(cfi);
    cfi.nFont = 0;
    cfi.dwFontSize = { PIXEL_SIZE_WIDTH, PIXEL_SIZE_HEIGHT };
    wcscpy_s(cfi.FaceName, L"Raster Fonts");
    SetCurrentConsoleFontEx(consoleBuffer, true, &cfi);
    
    // map
    map += L"################################";
    map += L"#.......#.......##.............#";
    map += L"#.......#..##.....#....#.#.#...#";
    map += L"#..#############...#...........#";
    map += L"#....#..............#...#.#....#";
    map += L"#....#....######...............#";
    map += L"#....#...##..........#####.....#";
    map += L"####.######.....##.............#";
    map += L"#....#....##.....##...#####....#";
    map += L"#..........##.....##.......##..#";
    map += L"#...#.......##....##......#....#";
    map += L"#...#........##..##.......#.#..#";
    map += L"#...#.........###....##...#....#";
    map += L"#...#####..........#....###..#.#";
    map += L"#.............##...............#";
    map += L"################################";
   
	while (running) {
        // pause game state if window not focused
        while (consoleName != GetForegroundWindow()) continue;

        // sets cursor to the middle of the screen
        setCursorMidScreen();

        // start game clock
		timeStampDynamic = std::chrono::system_clock::now();
		frameTime = timeStampDynamic - timeStamp; // get the time it takes for a tick
		timeStamp = timeStampDynamic; // reset current time and prepare to measure new frametime on upcoming tick
		fpsNormalise = frameTime.count(); // allows for consistency in movement independent of FPS

		// controls
        float moveAngle;
        bool moving = false;
        if (GetAsyncKeyState('W') && GetAsyncKeyState('D')) { moveAngle = -1*PI/4; moving = true; }
        else if (GetAsyncKeyState('W') && GetAsyncKeyState('A')) { moveAngle = 1*PI/4; moving = true; }
        else if (GetAsyncKeyState('W')) { moveAngle = 2*PI; moving = true; }
        else if (GetAsyncKeyState('S')) { moveAngle = PI; moving = true; }
        else if (GetAsyncKeyState('A')) { moveAngle = PI/2; moving = true; }
        else if (GetAsyncKeyState('D')) { moveAngle = -PI/2; moving = true; }
        if (moving) playerMove(moveAngle);

        if (GetAsyncKeyState(VK_ESCAPE)) running = false;

		// ray casting
		for (int col = 0; col < SCREEN_WIDTH; col++) {
			ray = (player.cameraYaw.x - FOV / 2.f) + (float(col) / float(SCREEN_WIDTH)) * FOV;
            eyeX = sinf(ray); eyeY = cosf(ray);
            distanceToWall = 0;
            blockBoundary = false;
			player.touchWall = false;

			// incrementing to see if a wall is hit by our rays
			while (!player.touchWall && distanceToWall < MAX_DEPTH) {
				distanceToWall += RAY_STEP;
				rayHitX = int(player.location.x + eyeX * distanceToWall);
			    rayHitY = int(player.location.y + eyeY * distanceToWall);

				// object is out of bounds
				if (rayHitX < 0 || rayHitX >= MAP_WIDTH || rayHitY < 0 || rayHitY >= MAP_HEIGHT) 
                    distanceToWall = MAX_DEPTH+1; // object is out of our render distance
				else
					if (map[rayHitY*MAP_WIDTH + rayHitX] == '#') {
						player.touchWall = true;
						std::vector<std::pair<float, float>> blockCorners;
						for (int x = 0; x < WALL_BOUNDARY_AMOUNT; x++) 
							for (int y = 0; y < WALL_BOUNDARY_AMOUNT; y++) {   
								float vectorX = rayHitX + x - player.location.x;
								float vectorY = rayHitY + y - player.location.y;
								float distance = sqrt(pow(vectorX, 2) + pow(vectorY, 2)); // eucledian distance
								float area = (eyeX * vectorX / distance) + (eyeY * vectorY / distance);
								blockCorners.push_back(std::make_pair(distance, area));
							}
						// sort pairs of block corners
						std::sort(blockCorners.begin(), blockCorners.end(), [](const std::pair<float, float> &left, const std::pair<float, float> &right) 
                        { return left.first < right.first; });

						for (int i = 0; i < WALL_BOUNDARY_AMOUNT; i++) 
                            if (acos(blockCorners.at(i).second) < WALL_BOUNDARY_THRESHOLD) 
                                blockBoundary = true;
					}		
			}
            shadeWall();

            // draw world 
            worldCeiling = float(SCREEN_HEIGHT / 2.) - SCREEN_HEIGHT / float(distanceToWall) - player.cameraYaw.y;
            worldFloor = SCREEN_HEIGHT - worldCeiling - 2 * player.cameraYaw.y;
			for (int row = 0; row < SCREEN_HEIGHT; row++) {
                if (row < worldCeiling) screen[row*SCREEN_WIDTH + col] = CLEAR_SHADE; // sky
                else if (row >= worldCeiling && row < worldFloor) screen[row*SCREEN_WIDTH + col] = wallShade; // wall
				else screen[row*SCREEN_WIDTH + col] = FLOOR_SHADE; // floor
			}
		}

        // check how much the mouse has moved (not optimal place to put it as it greatly depends on frame time
        rotatationCheck();

        // frame counter
        swprintf_s(screen, 10, L" FPS %3.0f", 1.f / fpsNormalise);

		// draw the screen array to the buffer
		WriteConsoleOutputCharacter(consoleBuffer, screen, SCREEN_PIXEL_COUNT, SCREEN_DRAW_START, &bytesWritten);
	}

    return 0;
}