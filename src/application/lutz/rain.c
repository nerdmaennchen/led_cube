/*
 * surface.c
 *
 *  Created on: May 18, 2013
 *      Author: lutz
 */


#include <interfaces/cubeRendering.h>
#include <interfaces/cubeConfig.h>
#include <interfaces/systemTime.h>
#include <interfaces/random.h>
#include <flawless/init/systemInitializer.h>
#include <string.h>


#define MAX_NUM_DROPLETS (CUBE_CONFIG_NUMBER_OF_COLS * CUBE_CONFIG_NUMBER_OF_ROWS)

static uint8_t g_dropletPositions[MAX_NUM_DROPLETS];

#define DROPLET_HEIGHT 1U

#define DROPLET_SPAWN_INTERVAL_US 60000ULL

#define DROPLET_FALL_INTERVALL_US 50000ULL


static void rain_renderingFunction(cubeFrameBuf *buf);
CUBE_RENDER_FUNCTION(rain_renderingFunction, NULL)
static void rain_renderingFunction(cubeFrameBuf *buf)
{
	uint32_t randomNr = getRandomNumber();
	uint8_t x, y;
	uint8_t choice = randomNr % MAX_NUM_DROPLETS;
	static systemTime_t lastUpdateTime = 0U;
	static systemTime_t lastSpawnTime = 0U;
	systemTime_t curTime = getSystemTimeUS();

	memset(buf, 0, sizeof(*buf));

	if (0 == lastUpdateTime)
	{
		lastUpdateTime = getSystemTimeUS();
		lastSpawnTime = lastUpdateTime;
	}

	if (lastUpdateTime + DROPLET_FALL_INTERVALL_US < curTime)
	{
		lastUpdateTime = curTime;
		/* let the rain fall! */
		for (x = 0U; x < MAX_NUM_DROPLETS; ++x)
		{
			if (0xff != g_dropletPositions[x])
			{
				g_dropletPositions[x] -= 1;
			}
		}
	}

	if (lastSpawnTime + DROPLET_SPAWN_INTERVAL_US < curTime)
	{
		lastSpawnTime = curTime;

		if (0xff == g_dropletPositions[choice]) /* this one can be spawned */
		{
			g_dropletPositions[choice] = CUBE_CONFIG_NUMBER_OF_LAYERS - 1;
		}
	}


	for (x = 0U; x < CUBE_CONFIG_NUMBER_OF_COLS; ++x)
	{
		for (y = 0U; y < CUBE_CONFIG_NUMBER_OF_ROWS; ++y)
		{
			const uint8_t dropletNr = x * CUBE_CONFIG_NUMBER_OF_ROWS + y;
			if (0xff != g_dropletPositions[dropletNr])
			{
				uint8_t j;

				for (j = 0U; j < DROPLET_HEIGHT; ++j)
				{
					if (g_dropletPositions[dropletNr] + j < CUBE_CONFIG_NUMBER_OF_LAYERS)
					{
						(*buf)[g_dropletPositions[dropletNr] + j][x][y] = 1;
					}
				}
			}
		}
	}

}

static void rain_init(void);
MODULE_INIT_FUNCTION(rain, 9, rain_init)
static void rain_init(void)
{
	uint8_t i;
	for (i = 0U; i < MAX_NUM_DROPLETS; ++i)
	{
		g_dropletPositions[i] = 0xff;
	}
}

