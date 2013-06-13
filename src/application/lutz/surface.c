/*
 * surface.c
 *
 *  Created on: May 18, 2013
 *      Author: lutz
 */


#include <interfaces/cubeRendering.h>
#include <interfaces/cubeConfig.h>
#include <interfaces/systemTime.h>

static void surface_renderingFunction(cubeFrameBuf *buf);
CUBE_RENDER_FUNCTION(surface_renderingFunction)
static void surface_renderingFunction(cubeFrameBuf *buf)
{
	static uint8_t layer = 0U;
	static systemTime_t lastTickTime = 0U;
	uint8_t i,j,k;

	if (0U == lastTickTime)
	{
		lastTickTime = getSystemTimeUS();
	}
	systemTime_t curTime = getSystemTimeUS();

	if (lastTickTime + 10000 < curTime)
	{
		layer = (layer + 1) % CUBE_CONFIG_NUMBER_OF_LAYERS;
		lastTickTime = getSystemTimeUS();
	}

	for (i = 0U; i < CUBE_CONFIG_NUMBER_OF_LAYERS; ++i)
	{
		for (j = 0U; j < CUBE_CONFIG_NUMBER_OF_ROWS; ++j)
		{
			for (k = 0U; k < CUBE_CONFIG_NUMBER_OF_COLS; ++k)
			{
				if (layer == k)
				{
					(*buf)[k][i][j] = 1;
				} else
				{
					(*buf)[k][i][j] = 0;
				}
			}
		}
	}
}

