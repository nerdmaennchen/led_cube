/*
 * surface.c
 *
 *  Created on: May 18, 2013
 *      Author: lutz
 */


#include <interfaces/cubeRendering.h>
#include <interfaces/cubeConfig.h>
#include <interfaces/systemTime.h>
#include <string.h>

static void simpleRunning_renderingFunction(cubeFrameBuf *buf);
CUBE_RENDER_FUNCTION(simpleRunning_renderingFunction, NULL)
static void simpleRunning_renderingFunction(cubeFrameBuf *buf)
{
	static systemTime_t lastTickTime = 0U;
	static uint8_t i,j,k;

	if (0U == lastTickTime)
	{
		lastTickTime = getSystemTimeUS();
		i = 0U;
		j = 0U;
		k = 0U;
	}
	systemTime_t curTime = getSystemTimeUS();

	memset(buf, 0, sizeof(*buf));

	(*buf)[i][j][k] = 1;

	if (lastTickTime + 500000 < curTime)
	{
		lastTickTime = getSystemTimeUS();
		++i;
		if (8 == i)
		{
			i = 0U;
//			++j;
//			if (8 == j)
//			{
//				j = 0U;
//				++k;
//				if (8 == k)
//				{
//					k = 0U;
//				}
//			}
		}
	}

}

