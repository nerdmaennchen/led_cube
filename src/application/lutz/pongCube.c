/*
 * pongCube.c
 *
 *  Created on: Jul 3, 2013
 *      Author: lutz
 */


#include <interfaces/cubeRendering.h>
#include <interfaces/cubeConfig.h>
#include <interfaces/systemTime.h>
#include <flawless/stdtypes.h>
#include <interfaces/random.h>

#include <string.h>

#define PONG_CUBE_RENDER_UPDATE_INTERVAL_US 25000ULL

#define PONG_CUBE_SIZE 3U

static uint8_t x,y,z;

static void drawCube(cubeFrameBuf *buf);
static void drawCube(cubeFrameBuf *buf)
{
	memset(buf, 0, sizeof(*buf));

	uint8_t i, j, k;

	for (i = x; i < x + PONG_CUBE_SIZE; ++i)
	{
		for (j = y; j < y + PONG_CUBE_SIZE; ++j)
		{
			for (k = z; k < z + PONG_CUBE_SIZE; ++k)
			{
				(*buf)[i][j][k] = 1;
			}
		}
	}
}


static void pongCube_renderingInitFunction(cubeFrameBuf *buf);
static void pongCube_renderingInitFunction(cubeFrameBuf *buf)
{
	x = 0;
	y = 0;
	z = 0;

	drawCube(buf);
}

static void pongCube_renderingFunction(cubeFrameBuf *buf);
static void pongCube_renderingFunction(cubeFrameBuf *buf)
{
	static systemTime_t lastTickTime = 0U;

	if (0U == lastTickTime) {
		lastTickTime = getSystemTimeUS();
	}
	systemTime_t curTime = getSystemTimeUS();

	if (lastTickTime + PONG_CUBE_RENDER_UPDATE_INTERVAL_US < curTime) {
		lastTickTime = curTime;

		/* move the cube */

		uint8_t direction = getRandomNumber();
		direction = direction % 12;

		switch (direction) {
			case 0:
				if (x < CUBE_CONFIG_NUMBER_OF_ROWS - PONG_CUBE_SIZE - 1) {
					++x;
				}
				break;
			case 1:
				if (y < CUBE_CONFIG_NUMBER_OF_ROWS - PONG_CUBE_SIZE - 1) {
					++y;
				}
				break;
			case 2:
				if (z < CUBE_CONFIG_NUMBER_OF_ROWS - PONG_CUBE_SIZE - 1) {
					++z;
				}
				break;
			case 3:
				if (x > 0) {
					--x;
				}
				break;
			case 4:
				if (y > 0) {
					--y;
				}
				break;
			case 5:
				if (z > 0) {
					--z;
				}
				break;
		}

		drawCube(buf);
	}
}



CUBE_RENDER_FUNCTION(pongCube_renderingFunction, pongCube_renderingInitFunction)
