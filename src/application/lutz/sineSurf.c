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
#include <stdlib.h>

#define M_PI		3.14159265358979323846f

#define SINE_SURF_WAVE_HEIGHT 1.5

#define SINE_SURF_UPDATE_INTERVAL_US 50000
#define SINE_SURF_ROUND_TIME_US 1000000.f

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

static void sineSurf_draw(cubeFrameBuf *buf);
static void sineSurf_draw(cubeFrameBuf *buf)
{
	memset(buf, 0, sizeof(*buf));

	uint8_t i;
	for (i = 0U; i < CUBE_CONFIG_NUMBER_OF_ROWS; ++i)
	{
		const float curPhase = g_phase + M_PI * ((float)i / (float)(CUBE_CONFIG_NUMBER_OF_LAYERS));
		int8_t z = (uint8_t)(roundf( sinf(curPhase) * SINE_SURF_WAVE_HEIGHT + ((CUBE_CONFIG_NUMBER_OF_COLS - 1) / 2.f)));

		z = CLIP_VAL(z, 0, CUBE_CONFIG_NUMBER_OF_COLS);
		line(buf, z, i,0,i,7);
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

			g_phase += 2.f * M_PI / (SINE_SURF_ROUND_TIME_US / SINE_SURF_UPDATE_INTERVAL_US);

			sineSurf_draw(buf);
		}
}


CUBE_RENDER_FUNCTION(sineSurf_renderingFunction, sineSurf_initFunction, 10000000ULL)
