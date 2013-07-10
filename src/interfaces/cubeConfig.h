/*
 * cubeConfig.h
 *
 *  Created on: May 17, 2013
 *      Author: lutz
 */

#ifndef CUBECONFIG_H_
#define CUBECONFIG_H_


#define CUBE_CONFIG_NUMBER_OF_LAYERS 8
#define CUBE_CONFIG_NUMBER_OF_ROWS   8
#define CUBE_CONFIG_NUMBER_OF_COLS   8



/*
 * 20 updates per second (seems fair enough)
 */
#define CUBE_RENDER_NEW_FRAME_INTERVAL_US 50000UL

/*
 * change to next rendering function each ... us
 */
#define CUBE_NEXT_RENDERING_FUNCTION_INTERVAL_US 15000000ULL


/*
 * the duration a layer is visible
 * 4ms = 31.25 fps
 */
#define CUBE_LAYER_FRAME_INTERVAL_US 1000UL

#endif /* CUBECONFIG_H_ */
