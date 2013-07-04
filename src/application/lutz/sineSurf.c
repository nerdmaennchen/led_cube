/*
 * sineSurf.c
 *
 *  Created on: Jul 3, 2013
 *      Author: lutz
 */


#include <interfaces/cubeRendering.h>
#include <interfaces/cubeConfig.h>
#include <interfaces/systemTime.h>
#include <interfaces/random.h>
#include <flawless/init/systemInitializer.h>
#include <string.h>
#include <math.h>

#define M_PI		3.14159265358979323846f


#define SINE_SURF_UPDATE_INTERVAL_US 100000
#define SINE_SURF_ROUND_TIME 2.f

static float g_phase = 0.f;

static void sineSurf_draw(cubeFrameBuf *buf);
static void sineSurf_draw(cubeFrameBuf *buf)
{
	uint8_t i, j;
	for (i = 0U; i < CUBE_CONFIG_NUMBER_OF_ROWS; ++i)
	{
		for (j = 0U; j < CUBE_CONFIG_NUMBER_OF_COLS; ++j)
		{

		}
	}
}


static void sineSurf_initFunction(cubeFrameBuf *buf)
{
	g_phase = 0.f;
	sineSurf_draw(buf);
}

static void sineSurf_renderingFunction(cubeFrameBuf *buf)
{
	static systemTime_t lastUpdateTime = 0U;
	systemTime_t curtime = getSystemTimeUS();

		if (0 == lastUpdateTime)
		{
			lastUpdateTime = curtime;
		}

		if (lastUpdateTime + SINE_SURF_UPDATE_INTERVAL_US < curtime)
		{
			lastUpdateTime = getCurrentTime();

			const float time = ((float)lastUpdateTime / 1000000.f / SINE_SURF_ROUND_TIME) * M_PI;

			g_phase = asin(time);

			sineSurf_draw(buf);
		}
}


CUBE_RENDER_FUNCTION(sineSurf_renderingFunction, sineSurf_initFunction)
