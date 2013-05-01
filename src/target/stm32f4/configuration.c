/*
 * configuration.c
 *
 *  Created on: 05.05.2012
 *      Author: danielb
 */

#include <libopencm3/stm32/f4/rcc.h>
#include <flawless/init/systemInitializer.h>

#include <interfaces/configuration.h>

#include <libopencm3/stm32/f4/flash.h>

extern uint16_t _flashConfigROMBegin;
extern uint16_t _flashConfigRAMBegin, _flashConfigRAMEnd;

#define FLASH_CONFIG_SECTOR (11 << 3)
#define FLASH_PROGRAMM_ACCESS_SIZE (1 << 8)

#define FLASH_CONFIG_SECTOR_SIZE (128 * 1024)
#define FLAS_CONFIG_START_ADDR   (0x080e0000)



static void readCurrentConfigurationFromFlash()
{
	uint16_t *src = &_flashConfigROMBegin;
	uint16_t *dst = &_flashConfigRAMBegin;

	for (;dst < &_flashConfigRAMEnd; ++src, ++dst)
	{
		*dst = *src;
	}
}


void config_updateToFlash()
{

	uint16_t *dst = &_flashConfigROMBegin;
	uint16_t *src = &_flashConfigRAMBegin;

	bool canUpdateFlash = false;
	for (; src < &_flashConfigRAMEnd; ++src, ++dst)
	{
		if (*dst != *src)
		{
			canUpdateFlash = true;
			break;
		}
	}


	if (false != canUpdateFlash)
	{
		dst = &_flashConfigROMBegin;
		src = &_flashConfigRAMBegin;
		(void) flash_unlock();
		flash_erase_sector(FLASH_CONFIG_SECTOR, FLASH_PROGRAMM_ACCESS_SIZE);
		for (; src < &_flashConfigRAMEnd; ++src, ++dst)
		{
			flash_program_half_word((uint32_t)dst, *src, FLASH_PROGRAMM_ACCESS_SIZE);
		}
		(void) flash_lock();
	}
}

static void configuration_init(void);
MODULE_INIT_FUNCTION(configuration, 3, configuration_init)
static void configuration_init(void)
{
	/* enable flash interface  clock. */
	 (void) readCurrentConfigurationFromFlash();
	 config_updateToFlash();

}

