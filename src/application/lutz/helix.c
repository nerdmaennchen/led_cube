/*
 * helix.c
 *
 *  Created on: Jul 4, 2013
 *      Author: lutz
 */




#include <interfaces/cubeRendering.h>
#include <interfaces/cubeConfig.h>
#include <interfaces/systemTime.h>
#include <interfaces/random.h>
#include <flawless/init/systemInitializer.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#define M_PI		3.14159265358979323846f

#define HELIX_UPDATE_INTERVAL_US 100000

#define HELIX_ROUND_TIME_US 1500000.f

#define CLIP_VAL(x,min,max) (((x) > max) ? max : ((x) < min) ? min : (x))

static float g_phase = 0.f;

static void line(cubeFrameBuf *buf, uint8_t z, int8_t x0, int8_t y0, int8_t x1, int8_t y1)
{
  int dx =  abs(x1-x0), sx = x0<x1 ? 1 : -1;
  int dy = -abs(y1-y0), sy = y0<y1 ? 1 : -1;
  int err = dx+dy, e2; /* error value e_xy */

  for(;;){  /* loop */
    (*buf)[z][x0][y0] = 1;
    if (x0==x1 && y0==y1) break;
    e2 = 2*err;
    if (e2 > dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
    if (e2 < dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
  }
}


static void helixDraw(cubeFrameBuf *buf);
static void helixDraw(cubeFrameBuf *buf)
{
	memset(buf, 0, sizeof(*buf));

	uint8_t i;
	for (i = 0U; i < CUBE_CONFIG_NUMBER_OF_ROWS; ++i)
	{
		const float curPhase = g_phase + M_PI * ((float)i / (float)(CUBE_CONFIG_NUMBER_OF_LAYERS));

		int8_t x1 = (uint8_t)(roundf( sinf(curPhase) * ((CUBE_CONFIG_NUMBER_OF_COLS - 1) / 2.f) + ((CUBE_CONFIG_NUMBER_OF_COLS - 1) / 2.f)));
		int8_t y1 = (uint8_t)(roundf( cosf(curPhase) * ((CUBE_CONFIG_NUMBER_OF_COLS - 1) / 2.f) + ((CUBE_CONFIG_NUMBER_OF_COLS - 1) / 2.f)));

		int8_t x2 = (uint8_t)(roundf(-sinf(curPhase) * ((CUBE_CONFIG_NUMBER_OF_COLS - 1) / 2.f) + ((CUBE_CONFIG_NUMBER_OF_COLS - 1) / 2.f)));
		int8_t y2 = (uint8_t)(roundf(-cosf(curPhase) * ((CUBE_CONFIG_NUMBER_OF_COLS - 1) / 2.f) + ((CUBE_CONFIG_NUMBER_OF_COLS - 1) / 2.f)));

		x1 = CLIP_VAL(x1, 0, CUBE_CONFIG_NUMBER_OF_COLS);
		x2 = CLIP_VAL(x2, 0, CUBE_CONFIG_NUMBER_OF_COLS);

		y1 = CLIP_VAL(y1, 0, CUBE_CONFIG_NUMBER_OF_COLS);
		y2 = CLIP_VAL(y2, 0, CUBE_CONFIG_NUMBER_OF_COLS);

		(*buf)[i][x1][y1] = 1;
		(*buf)[i][x2][y2] = 1;

		if (0 == (i % 3))
		{
			line(buf, i, x1, y1, x2, y2);
		}
	}
}


static void helix_initFunction(cubeFrameBuf *buf)
{
	g_phase = 0.f;
	helixDraw(buf);
}

static void helix_renderingFunction(cubeFrameBuf *buf)
{
	static systemTime_t lastUpdateTime = 0U;

	if (0 == lastUpdateTime)
	{
		lastUpdateTime = getSystemTimeUS();
	}

	if (lastUpdateTime + HELIX_UPDATE_INTERVAL_US < getSystemTimeUS())
	{
		lastUpdateTime = getSystemTimeUS();

		g_phase += 2 * M_PI / (HELIX_ROUND_TIME_US / HELIX_UPDATE_INTERVAL_US);

		helixDraw(buf);
	}
}

CUBE_RENDER_FUNCTION(helix_renderingFunction, helix_initFunction, 15000000ULL)
