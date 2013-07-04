/*
 * cubeRendering.h
 *
 *  Created on: May 18, 2013
 *      Author: lutz
 */

#ifndef CUBERENDERING_H_
#define CUBERENDERING_H_

#include <interfaces/cubeConfig.h>
#include <stdint.h>

typedef uint8_t cubeFrameBuf[CUBE_CONFIG_NUMBER_OF_LAYERS][CUBE_CONFIG_NUMBER_OF_COLS][CUBE_CONFIG_NUMBER_OF_ROWS];

typedef void (*cubeRenderingFunction)(cubeFrameBuf*);
typedef void (*cubeRenderingInit)(cubeFrameBuf*);

typedef struct tag_cubeRenderingHandle
{
	cubeRenderingFunction renderFunction;
	cubeRenderingInit     initFunction;
} cubeRenderingHandle_t;

#define CUBE_RENDER_FUNCTION(renderFunctionName, initFuncitonName) \
		__attribute__((unused))\
		__attribute__ ((section(".cubeRenderHandles"))) \
		const cubeRenderingHandle_t renderFunctionName ## handle = {renderFunctionName, initFuncitonName}; \



#endif /* CUBERENDERING_H_ */
