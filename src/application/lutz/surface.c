/*
 * surface.c
 *
 *  Created on: May 18, 2013
 *      Author: lutz
 */


#include <interfaces/cubeRendering.h>
#include <interfaces/cubeConfig.h>

static void surface_renderingFunction(cubeFrameBuf *buf);
CUBE_RENDER_FUNCTION(surface_renderingFunction)
static void surface_renderingFunction(cubeFrameBuf *buf)
{
	uint8_t i,j;
	for (i = 0U; i < CUBE_CONFIG_NUMBER_OF_ROWS; ++i)
	{
		for (j = 0U; j < CUBE_CONFIG_NUMBER_OF_ROWS; ++j)
		{
			(*buf)[0][i][j] = 1;
		}

	}
}

