/*
 * surface.c
 *
 *  Created on: May 18, 2013
 *      Author: lutz
 */

#include <interfaces/cubeRendering.h>
#include <interfaces/cubeConfig.h>
#include <interfaces/systemTime.h>
#include <flawless/stdtypes.h>

#include <string.h>

#define WOBBEL_CUBE_RENDER_UPDATE_INTERVAL_US 50000ULL

static uint8_t x1, y1, z1;
static uint8_t x2, y2, z2;

static void drawCube(cubeFrameBuf *buf);
static void drawCube(cubeFrameBuf *buf) {
	uint8_t i;

	/* top and bottomn x part */
	for (i = x1; i < x2; ++i) {
		(*buf)[i][y1][z1] = 1;
		(*buf)[i][y2][z1] = 1;
		(*buf)[i][y1][z2] = 1;
		(*buf)[i][y2][z2] = 1;
	}

	for (i = y1; i < y2; ++i) {
		(*buf)[x1][i][z1] = 1;
		(*buf)[x2][i][z1] = 1;
		(*buf)[x1][i][z2] = 1;
		(*buf)[x2][i][z2] = 1;
	}

	for (i = z1; i < z2; ++i) {
		(*buf)[x1][y1][i] = 1;
		(*buf)[x2][y1][i] = 1;
		(*buf)[x1][y2][i] = 1;
		(*buf)[x2][y2][i] = 1;
	}
}

static void wobbelCube_renderingInitFunction(cubeFrameBuf *buf);
static void wobbelCube_renderingInitFunction(cubeFrameBuf *buf) {
	memset(buf, 0, sizeof(*buf));
	x1 = 0U;
	y1 = 0U;
	z1 = 0U;

	x2 = CUBE_CONFIG_NUMBER_OF_ROWS - 1;
	y2 = CUBE_CONFIG_NUMBER_OF_COLS - 1;
	z2 = CUBE_CONFIG_NUMBER_OF_LAYERS - 1;

	drawCube(buf);
}

static void wobbelCube_renderingFunction(cubeFrameBuf *buf);
static void wobbelCube_renderingFunction(cubeFrameBuf *buf) {
	static systemTime_t lastTickTime = 0U;

	if (0U == lastTickTime) {
		lastTickTime = getSystemTimeUS();
	}
	systemTime_t curTime = getSystemTimeUS();

	if (lastTickTime + WOBBEL_CUBE_RENDER_UPDATE_INTERVAL_US < curTime) {
		lastTickTime = curTime;

		uint8_t direction = getRandomNumber();
		direction = direction % 12;

		switch (direction) {
		case 0:
			if (x1 < CUBE_CONFIG_NUMBER_OF_ROWS - 1) {
				++x1;
			}
			break;
		case 1:
			if (y1 < CUBE_CONFIG_NUMBER_OF_ROWS - 1) {
				++y1;
			}
			break;
		case 2:
			if (z1 < CUBE_CONFIG_NUMBER_OF_ROWS - 1) {
				++z1;
			}
			break;
		case 3:
			if (x1 > 0) {
				--x1;
			}
			break;
		case 4:
			if (y1 > 0) {
				--y1;
			}
			break;
		case 5:
			if (z1 > 0) {
				--z1;
			}
			break;
		case 6:
			if (x2 < CUBE_CONFIG_NUMBER_OF_ROWS - 1) {
				++x2;
			}
			break;
		case 7:
			if (y2 < CUBE_CONFIG_NUMBER_OF_ROWS - 1) {
				++y2;
			}
			break;
		case 8:
			if (z2 < CUBE_CONFIG_NUMBER_OF_ROWS - 1) {
				++z2;
			}
			break;
		case 9:
			if (x2 > 0) {
				--x2;
			}
			break;
		case 10:
			if (y2 > 0) {
				--y2;
			}
			break;
		case 11:
			if (z2 > 0) {
				--z2;
			}
			break;
		default:
			break;
		}

		if (x1 > x2) {
			const uint8_t tmp = x1;
			x1 = x2;
			x2 = tmp;
		}
		if (y1 > y2) {
			const uint8_t tmp = y1;
			y1 = y2;
			y2 = tmp;
		}
		if (z1 > z2) {
			const uint8_t tmp = z1;
			z1 = z2;
			z2 = tmp;
		}

		memset(buf, 0, sizeof(*buf));
		drawCube(buf);
	}
}

CUBE_RENDER_FUNCTION(wobbelCube_renderingFunction,
		wobbelCube_renderingInitFunction)
