/*
 * wobbelSurf.c
 *
 *  Created on: Jun 16, 2013
 *      Author: lutz
 */



#include <interfaces/cubeRendering.h>
#include <interfaces/cubeConfig.h>
#include <interfaces/systemTime.h>
#include <interfaces/random.h>
#include <flawless/init/systemInitializer.h>
#include <string.h>

#define WOBBEL_SURF_UPDATE_INTERVAL_US 10000ULL

static bool g_directionUp = false;
static int8_t g_step = 0U;

static void wobbelSurf_draw(cubeFrameBuf *buf);
static void wobbelSurf_draw(cubeFrameBuf *buf)
{
	memset(buf, 0, sizeof(*buf));
	uint8_t i;
	for (i = 0U; i < CUBE_CONFIG_NUMBER_OF_LAYERS / 2; ++i)
	{
		uint8_t z = 0;
		if (false == g_directionUp)
		{
			z = MIN(CUBE_CONFIG_NUMBER_OF_LAYERS - 1, MAX(g_step - i,0));
		} else
		{
			z = MIN(CUBE_CONFIG_NUMBER_OF_LAYERS - 1, MAX(g_step + i,0));
		}

		uint8_t j;
		for (j = i; j < CUBE_CONFIG_NUMBER_OF_LAYERS - i; ++j)
		{
			(*buf)[z][j][i] = 1;
			(*buf)[z][i][j] = 1;

			(*buf)[z][j][CUBE_CONFIG_NUMBER_OF_LAYERS - 1 - i] = 1;
			(*buf)[z][CUBE_CONFIG_NUMBER_OF_LAYERS - 1 - i][j] = 1;
		}
	}
}

static void wobbelSurf_initFunction(cubeFrameBuf *buf)
{
	g_directionUp = true;
	g_step = - (CUBE_CONFIG_NUMBER_OF_LAYERS / 2);
	wobbelSurf_draw(buf);
}

static void wobbelSurf_renderingFunction(cubeFrameBuf *buf)
{
	static systemTime_t lastUpdateTime = 0U;
	systemTime_t curtime = getSystemTimeUS();

		if (0 == lastUpdateTime)
		{
			lastUpdateTime = curtime;
		}

		if (lastUpdateTime + WOBBEL_SURF_UPDATE_INTERVAL_US < curtime)
		{
			lastUpdateTime = getCurrentTime();
			if (false == g_directionUp)
			{
				--g_step;
				if (g_step <= - CUBE_CONFIG_NUMBER_OF_LAYERS / 2)
				{
					g_directionUp = true;
				}
			} else
			{
				++g_step;
				if (g_step >= CUBE_CONFIG_NUMBER_OF_LAYERS + CUBE_CONFIG_NUMBER_OF_LAYERS / 2)
				{
					g_directionUp = false;
				}
			}
			wobbelSurf_draw(buf);
		}
}


CUBE_RENDER_FUNCTION(wobbelSurf_renderingFunction, wobbelSurf_initFunction, 10000000ULL)
