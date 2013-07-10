/*
 * topDownSwap.c
 *
 *  Created on: Jul 3, 2013
 *      Author: lutz
 */

#include <interfaces/cubeRendering.h>
#include <interfaces/cubeConfig.h>
#include <interfaces/systemTime.h>
#include <interfaces/random.h>

#include <string.h>

#define TOP_DOWN_MOVE_UPDATE_INTERVAL_US 20000

static void initTopDownRendering(cubeFrameBuf *buf);
static void initTopDownRendering(cubeFrameBuf *buf)
{
	uint8_t i,j;

	memset(buf, 0, sizeof(*buf));
	for (i = 0U; i < CUBE_CONFIG_NUMBER_OF_COLS; ++i)
	{
		for (j = 0U; j < CUBE_CONFIG_NUMBER_OF_ROWS; ++j)
		{
			uint32_t randomNr = getRandomNumber();
			if (1 == (randomNr & 1))
			{
				(*buf)[0][i][j] = 1;
			} else
			{
				(*buf)[CUBE_CONFIG_NUMBER_OF_LAYERS - 1][i][j] = 1;
			}
		}
	}
}

static void topDownSwap_renderingFunction(cubeFrameBuf *buf);
static void topDownSwap_renderingFunction(cubeFrameBuf *buf)
{
	/* at first */
	static uint32_t currentSelection = 0xffffffff;
	static bool movingUp = false;

	uint8_t row = currentSelection % CUBE_CONFIG_NUMBER_OF_ROWS;
	uint8_t col = (currentSelection >> 8) % CUBE_CONFIG_NUMBER_OF_COLS;

	if (currentSelection == 0xffffffff)
	{
		currentSelection = getRandomNumber();
		row = currentSelection % CUBE_CONFIG_NUMBER_OF_ROWS;
		col = (currentSelection >> 8) % CUBE_CONFIG_NUMBER_OF_COLS;

		if (0 != (*buf)[0][row][col])
		{
			movingUp = true;
		} else
		{
			movingUp = false;
		}
	} else
	{
		static systemTime_t lastUpdateTime = 0U;

		if (0U == lastUpdateTime)
		{
			lastUpdateTime = getSystemTimeUS();
		}

		if (lastUpdateTime + TOP_DOWN_MOVE_UPDATE_INTERVAL_US < getSystemTimeUS())
		{
			lastUpdateTime = getSystemTimeUS();

			uint8_t i;
			if (false != movingUp)
			{
				for (i = 0U; i < CUBE_CONFIG_NUMBER_OF_LAYERS; ++i)
				{
					if (0U != (*buf)[i][row][col])
					{
						(*buf)[i][row][col] = 0U;
						if (i < CUBE_CONFIG_NUMBER_OF_LAYERS - 1U)
						{
							(*buf)[i + 1][row][col] = 1U;
						} else
						{
							(*buf)[CUBE_CONFIG_NUMBER_OF_LAYERS - 1][row][col] = 1;
							currentSelection = 0xffffffff; /* go for the next one */
						}
						break;
					}
				}
			} else
			{
				for (i = 0U; i < CUBE_CONFIG_NUMBER_OF_LAYERS; ++i)
				{
					if (0U != (*buf)[i][row][col])
					{
						(*buf)[i][row][col] = 0U;
						if (i > 0)
						{
							(*buf)[i - 1][row][col] = 1U;
						} else
						{
							(*buf)[0][row][col] = 1;
							currentSelection = 0xffffffff; /* go for the next one */
						}
						break;
					}
				}
			}
		}
	}
}


CUBE_RENDER_FUNCTION(topDownSwap_renderingFunction, initTopDownRendering, 20000000ULL);

