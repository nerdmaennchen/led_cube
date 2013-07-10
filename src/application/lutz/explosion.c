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
#include <flawless/stdtypes.h>
#include <string.h>

#define EXPLOSION_LAST_INTERVAL 3

#define EXPLOSION_UPDATE_INTERVAL_US 50000

#define CLIP_VAL(x,min,max) (((x) > max) ? max : ((x) < min) ? min : (x))

static uint8_t x = 0U;
static uint8_t y = 0U;
static uint8_t z = 0U;
static uint8_t trigger_height;

static void explosion_initFunction(cubeFrameBuf *buf);
static void explosion_initFunction(cubeFrameBuf *buf)
{
	memset(buf, 0, sizeof(*buf));
	x = getRandomNumber() % CUBE_CONFIG_NUMBER_OF_COLS;
	y = getRandomNumber() % CUBE_CONFIG_NUMBER_OF_ROWS;
	trigger_height = getRandomNumber() % CUBE_CONFIG_NUMBER_OF_LAYERS;

	x = CLIP_VAL(x, 1, CUBE_CONFIG_NUMBER_OF_COLS - 2);
	y = CLIP_VAL(y, 1, CUBE_CONFIG_NUMBER_OF_ROWS - 2);
	trigger_height = CLIP_VAL(trigger_height, 2, CUBE_CONFIG_NUMBER_OF_LAYERS - 2);
	z = 0;
}

static void explosion_renderingFunction(cubeFrameBuf *buf);
static void explosion_renderingFunction(cubeFrameBuf *buf)
{
	static systemTime_t lastTickTime = 0U;
	static bool isExploded = false;

	if (0U == lastTickTime)
	{
		lastTickTime = getSystemTimeUS();
	}
	systemTime_t curTime = getSystemTimeUS();

	if (lastTickTime + EXPLOSION_UPDATE_INTERVAL_US < curTime)
	{
		lastTickTime = getSystemTimeUS();
		if (false == isExploded)
		{
			memset(buf, 0, sizeof(*buf));
			(*buf)[z][y][x] = 1;
			++z;
			if (z >= trigger_height)
			{
				(*buf)[z][y][x] = EXPLOSION_LAST_INTERVAL;
				isExploded = true;
				x = 0U;
			}
		} else
		{
			uint8_t i, j, k;
			bool needNoReset = false;
			for (i = 0U; i < CUBE_CONFIG_NUMBER_OF_LAYERS; ++i)
			{
				for (j = 0U; j < CUBE_CONFIG_NUMBER_OF_COLS; ++j)
				{
					for (k = 0U; k < CUBE_CONFIG_NUMBER_OF_ROWS; ++k)
					{
						/* spawn if there is a neighbor alive */
						if ((0 == (*buf)[i][j][k]) &&
							(((i > 0) && (EXPLOSION_LAST_INTERVAL == (*buf)[i - 1][j][k])) ||
							((i < 7) && (EXPLOSION_LAST_INTERVAL == (*buf)[i + 1][j][k])) ||
							((j > 0) && (EXPLOSION_LAST_INTERVAL == (*buf)[i][j - 1][k])) ||
							((j < 7) && (EXPLOSION_LAST_INTERVAL == (*buf)[i][j + 1][k])) ||
							((k > 0) && (EXPLOSION_LAST_INTERVAL == (*buf)[i][j][k - 1])) ||
							((k < 7) && (EXPLOSION_LAST_INTERVAL == (*buf)[i][j][k + 1]))))
						{
							(*buf)[i][j][k] = EXPLOSION_LAST_INTERVAL + 1;
						} else
						{
							/* and die */
							if (0 < (*buf)[i][j][k])
							{
								--(*buf)[i][j][k];
								if (0 == (*buf)[i][j][k])
								{
									(*buf)[i][j][k] = -1;
								}
							}
						}

						if (0 < (*buf)[i][j][k])
						{
							needNoReset |= true;
						}
					}
				}
			}
			if (false == needNoReset) /* we need a reset */
			{
				isExploded = false;
				x = getRandomNumber() % CUBE_CONFIG_NUMBER_OF_COLS;
				y = getRandomNumber() % CUBE_CONFIG_NUMBER_OF_ROWS;

				trigger_height = getRandomNumber() % CUBE_CONFIG_NUMBER_OF_LAYERS;

				x = CLIP_VAL(x, EXPLOSION_LAST_INTERVAL, CUBE_CONFIG_NUMBER_OF_COLS - EXPLOSION_LAST_INTERVAL);
				y = CLIP_VAL(y, EXPLOSION_LAST_INTERVAL, CUBE_CONFIG_NUMBER_OF_ROWS - EXPLOSION_LAST_INTERVAL);
				trigger_height = CLIP_VAL(trigger_height, EXPLOSION_LAST_INTERVAL, CUBE_CONFIG_NUMBER_OF_LAYERS - EXPLOSION_LAST_INTERVAL);

				z = 0;
			}
		}
	}
}

CUBE_RENDER_FUNCTION(explosion_renderingFunction, explosion_initFunction, 20000000ULL)

