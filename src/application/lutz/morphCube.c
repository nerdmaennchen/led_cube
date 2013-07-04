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

#define MORPH_CUBE_RENDER_UPDATE_INTERVAL_US 50000ULL


static uint8_t g_size;


static void drawCube(cubeFrameBuf *buf);
static void drawCube(cubeFrameBuf *buf) {
	memset(buf, 0, sizeof(*buf));

	uint8_t i, j, k;

	for (i = g_size; i < CUBE_CONFIG_NUMBER_OF_ROWS - g_size; ++i)
	{
		for (j = g_size; j < CUBE_CONFIG_NUMBER_OF_ROWS - g_size; ++j)
		{
			for (k = g_size; k < CUBE_CONFIG_NUMBER_OF_ROWS - g_size; ++k)
			{
				(*buf)[i][j][k] = 1;
			}
		}
	}
}

static void morphCube_renderingInitFunction(cubeFrameBuf *buf);
static void morphCube_renderingInitFunction(cubeFrameBuf *buf)
{
	g_size = 0U;
	drawCube(buf);
}

static void morphCube_renderingFunction(cubeFrameBuf *buf);
static void morphCube_renderingFunction(cubeFrameBuf *buf) {
	static systemTime_t lastTickTime = 0U;

	if (0U == lastTickTime) {
		lastTickTime = getSystemTimeUS();
	}
	systemTime_t curTime = getSystemTimeUS();

	if (lastTickTime + MORPH_CUBE_RENDER_UPDATE_INTERVAL_US < curTime) {
		static bool increasing = false;

		lastTickTime = curTime;

		if (false == increasing)
		{
			++g_size;
			if (g_size >= 4)
			{
				g_size = 4;
				increasing = true;
			}
		} else
		{
			--g_size;
			if (0 == g_size)
			{
				increasing = false;
			}
		}

		drawCube(buf);
	}
}

CUBE_RENDER_FUNCTION(morphCube_renderingFunction,
		morphCube_renderingInitFunction)
