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

#define SURFACE_RENDER_UPDATE_INTERVAL_US 50000ULL

static void surface_renderingInitFunction(cubeFrameBuf *buf);
static void surface_renderingInitFunction(cubeFrameBuf *buf)
{
	memset(buf, 0, sizeof(*buf));
	uint8_t i,j;

	for (i = 0U; i < CUBE_CONFIG_NUMBER_OF_COLS; ++i)
	{
		for (j = 0U; j < CUBE_CONFIG_NUMBER_OF_COLS; ++j)
		{
			(*buf)[0][i][j] = 1;
		}
	}
}

static void surface_renderingFunction(cubeFrameBuf *buf);
static void surface_renderingFunction(cubeFrameBuf *buf)
{
	static systemTime_t lastTickTime = 0U;
	uint8_t tick = 0U;
	uint8_t i,j,k;

	if (0U == lastTickTime)
	{
		lastTickTime = getSystemTimeUS();
	}
	systemTime_t curTime = getSystemTimeUS();

	if (lastTickTime + SURFACE_RENDER_UPDATE_INTERVAL_US < curTime)
	{
		lastTickTime = curTime;

	}
}

CUBE_RENDER_FUNCTION(surface_renderingFunction, surface_renderingInitFunction)
