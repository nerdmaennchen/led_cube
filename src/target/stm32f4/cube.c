/*
 * cube.c
 *
 *  Created on: May 15, 2013
 *      Author: lutz
 */


#include <flawless/init/systemInitializer.h>
#include <flawless/timer/swTimer.h>
#include <flawless/core/msg_msgPump.h>
#include <flawless/config/msgIDs.h>

#include <libopencm3/stm32/f4/gpio.h>
#include <libopencm3/stm32/f4/rcc.h>
#include <libopencm3/stm32/f4/spi.h>

#include <libopencm3/stm32/f4/dma.h>
#include <libopencm3/stm32/f4/nvic_f4.h>
#include <libopencm3/stm32/f4/nvic.h>

#include <libopencm3/stm32/exti.h>

#include <interfaces/systemTime.h>
#include <interfaces/cubeConfig.h>
#include <interfaces/cubeRendering.h>

#include <string.h>

#define CUBE_SPI                        SPI1

#define CUBE_NEXT_RENDER_FUNCTION_BTN_PORT GPIOA
#define CUBE_NEXT_RENDER_FUNCTION_BTN_PIN  GPIO0

#define CUBE_TX_DMA DMA2
#define CUBE_TX_DMA_STREAM DMA_STREAM_3
#define CUBE_TX_DMA_CHANNEL 3U

#define CUBE_SPI_PORT                   GPIOB
#define CUBE_SPI_CLK_PIN                GPIO3
#define CUBE_SPI_MISO_PIN               GPIO4
#define CUBE_SPI_MOSI_PIN               GPIO5

#define CUBE_SHIFT_REGISTERS_NOE_PORT   GPIOA
#define CUBE_SHIFT_REGISTERS_NOE_PIN    GPIO8

#define CUBE_SHIFT_REGISTERS_STORE_PORT GPIOC
#define CUBE_SHIFT_REGISTERS_STORE_PIN  GPIO6

#define CUBE_SHIFT_REGISTERS_N_RESET_PORT GPIOC
#define CUBE_SHIFT_REGISTERS_N_RESET_PIN  GPIO7


#define CUBE_RESET_TIME_INTERVAL_US 50

/*
 * time interval for the push latch wire to be triggered
 */
#define CUBE_PUSH_LATCH_PULSE_DURATION_US 50U

typedef struct tag_layerGpioStruct
{
	uint32_t port;
	uint16_t pin;
} layerGpioStruct_t;

static const layerGpioStruct_t g_layerGpioDefs[] =
{
		{GPIOB, GPIO8},
		{GPIOC, GPIO15},
		{GPIOC, GPIO14},
		{GPIOC, GPIO13},
		{GPIOE, GPIO6},
		{GPIOE, GPIO5},
		{GPIOE, GPIO4},
		{GPIOE, GPIO2},
};

typedef struct tag_layerConfig
{
	/* in here is a bitarray of the configuration of a single layer */
	uint64_t data; /* 64 bit.. for every LED in a layer one bit*/
} layerConfig_t;

static cubeFrameBuf g_frameBuf;

#define CUBE_BUFFER_COUNT 3U

static layerConfig_t g_outputBuf[CUBE_BUFFER_COUNT][CUBE_CONFIG_NUMBER_OF_LAYERS];

static volatile uint8_t       g_currentOutputBuf = 0U;
static volatile uint8_t       g_currentLayer = 0U;


MSG_PUMP_DECLARE_MESSAGE_BUFFER_POOL(triggerRenderFunction, int, 2, MSG_ID_TRIGGER_RENDER_FUNCTION)


extern cubeRenderingHandle_t _renderingHandlesBegin;
extern cubeRenderingHandle_t _renderingHandlesEnd;
static cubeRenderingHandle_t *g_currentRenderingHandle;

static void onNextLayerTimer(void);


static void cube_disableLayer(uint8_t layer)
{
	if (layer < CUBE_CONFIG_NUMBER_OF_LAYERS)
	{
		gpio_clear(g_layerGpioDefs[layer].port, g_layerGpioDefs[layer].pin);
	}
}

static void cube_enableLayer(uint8_t layer)
{
	if (layer < CUBE_CONFIG_NUMBER_OF_LAYERS)
	{
		gpio_set(g_layerGpioDefs[layer].port, g_layerGpioDefs[layer].pin);
	}
}

/*
 * set the push latch wire
 */
static void cube_clearPushLatch(void)
{
	gpio_clear(CUBE_SHIFT_REGISTERS_STORE_PORT, CUBE_SHIFT_REGISTERS_STORE_PIN);

	/* resetup the dma */
	DMA_SM0AR(CUBE_TX_DMA, CUBE_TX_DMA_STREAM)  = (uint32_t)&(g_outputBuf[g_currentOutputBuf][g_currentLayer]);
	DMA_SNDTR(CUBE_TX_DMA, CUBE_TX_DMA_STREAM) = sizeof(g_outputBuf[0][0]);
	DMA_SCCR(CUBE_TX_DMA, CUBE_TX_DMA_STREAM)  |= DMA_CR_EN;
	swTimer_registerOnTimerUS(&onNextLayerTimer, CUBE_LAYER_FRAME_INTERVAL_US, true);
}

/*
 * clear the push latch wire
 */
static void cube_pushLatch(void)
{
	/* enable the "new" layer" */
	cube_disableLayer(g_currentLayer);

	gpio_set(CUBE_SHIFT_REGISTERS_STORE_PORT, CUBE_SHIFT_REGISTERS_STORE_PIN);
	g_currentLayer = (g_currentLayer + 1) % CUBE_CONFIG_NUMBER_OF_LAYERS;
	cube_enableLayer(g_currentLayer);

	gpio_clear(CUBE_SHIFT_REGISTERS_NOE_PORT, CUBE_SHIFT_REGISTERS_NOE_PIN);

	swTimer_registerOnTimerUS(&cube_clearPushLatch, CUBE_PUSH_LATCH_PULSE_DURATION_US, true);
}

/*
 * reset the shift registers
 */
static void cube_reset(void)
{
	gpio_clear(CUBE_SHIFT_REGISTERS_N_RESET_PORT, CUBE_SHIFT_REGISTERS_N_RESET_PIN);

	gpio_set(CUBE_SHIFT_REGISTERS_N_RESET_PORT, CUBE_SHIFT_REGISTERS_N_RESET_PIN);

	cube_pushLatch();
}

static void onNextLayerTimer(void)
{

	/* disable dma channel */
	DMA_SCCR(CUBE_TX_DMA, CUBE_TX_DMA_STREAM)  &= ~DMA_CR_EN;

	while (0 != (DMA_SCCR(CUBE_TX_DMA, CUBE_TX_DMA_STREAM) & DMA_CR_EN));
	DMA_LIFCR(CUBE_TX_DMA) = (DMA_LISR(CUBE_TX_DMA) & (0x3d << 22));
	/* the next dma transfer is handled after the push latch pulse is done */

	cube_pushLatch();
}

static void onRenderMsg(msgPump_MsgID_t msgID, const void *i_data);
static void onRenderMsg(msgPump_MsgID_t msgID, const void *i_data)
{
	if ((MSG_ID_TRIGGER_RENDER_FUNCTION == msgID) &&
		(NULL != i_data))
	{
		const uint8_t currentlyFreeBuffer = (g_currentOutputBuf + 1U) % CUBE_BUFFER_COUNT;

		/* call the current rendering function */
		if ((NULL != g_currentRenderingHandle) &&
				(NULL != g_currentRenderingHandle->renderFunction))
		{
			(void)(g_currentRenderingHandle->renderFunction)(&g_frameBuf);
		}

		/* render from g_frameBuf */
		{
			uint8_t i, j, k;
			for (i = 0U; i < CUBE_CONFIG_NUMBER_OF_LAYERS; ++i)
			{
				g_outputBuf[currentlyFreeBuffer][i].data = 0ULL;
				for (j = 0U; j < CUBE_CONFIG_NUMBER_OF_COLS; ++j)
				{
					for (k = 0U; k < CUBE_CONFIG_NUMBER_OF_ROWS; ++k)
					{
						if (0 < g_frameBuf[i][j][k])
						{
							g_outputBuf[currentlyFreeBuffer][i].data |= (1ULL << (j * CUBE_CONFIG_NUMBER_OF_COLS + k));
						}
					}
				}
			}
		}
		/* use the new outputBuffer */

		g_currentOutputBuf = currentlyFreeBuffer;
	}
}

static void onTriggerRenderFunctionTimerCB(void);
static void onTriggerRenderFunctionTimerCB(void)
{
	int bla = 0U;
	msgPump_postMessage(MSG_ID_TRIGGER_RENDER_FUNCTION, &bla);
}

static void triggerNextRenderFunction(void);
static void triggerNextRenderFunction(void)
{
	++g_currentRenderingHandle;
	if (g_currentRenderingHandle >= &_renderingHandlesEnd)
	{
		g_currentRenderingHandle = &_renderingHandlesBegin;
	}

	if (NULL != g_currentRenderingHandle->initFunction)
	{
		(void)(g_currentRenderingHandle->initFunction)(&g_frameBuf);
	}
	swTimer_registerOnTimerUS(&triggerNextRenderFunction, g_currentRenderingHandle->presentationTimeUS, true);
}

static void buttonEvaluation(void);
static void buttonEvaluation(void)
{
	triggerNextRenderFunction();
	EXTI_IMR |= CUBE_NEXT_RENDER_FUNCTION_BTN_PIN;
}


void exti0_isr(void)
{
	/* software debouncing by waiting some time to reenable the interrupt */
	swTimer_registerOnTimerUS(&buttonEvaluation, 100000, true);

	EXTI_IMR &= ~CUBE_NEXT_RENDER_FUNCTION_BTN_PIN;
	/* clear interupt pending bit */
	const uint32_t bla = EXTI_PR;
	EXTI_PR = (bla & (1 << 0));
}

/******************************************************/
/******************** INIT STUFF **********************/
/******************************************************/

static void cube_setupSpiInterface(void)
{
	RCC_APB2ENR |= RCC_APB2ENR_SPI1EN;
	SPI_CR2(CUBE_SPI)  = SPI_CR2_TXDMAEN;
	SPI_CR1(CUBE_SPI)  = SPI_CR1_BAUDRATE_FPCLK_DIV_64;
	SPI_CR1(CUBE_SPI) |= SPI_CR1_SSM | SPI_CR1_SSI;
	SPI_CR1(CUBE_SPI) |= SPI_CR1_MSTR;
	SPI_CR1(CUBE_SPI) |= SPI_CR1_SPE;
}

static void cube_setupDMA(void)
{
	/* disable dma channel */
	DMA_LIFCR(CUBE_TX_DMA) = (DMA_LISR(CUBE_TX_DMA) & (0x3d << 24));
	DMA_SCCR(CUBE_TX_DMA, CUBE_TX_DMA_STREAM)  = 0;
	while (0 != (DMA_SCCR(CUBE_TX_DMA, CUBE_TX_DMA_STREAM) & DMA_CR_EN));

	/* set channel */
	DMA_SCCR(CUBE_TX_DMA, CUBE_TX_DMA_STREAM)  |= (CUBE_TX_DMA_CHANNEL << DMA_CR_CHSEL_LSB);

	/* the size of a dma transfer is the size of one layer */
	DMA_SNDTR(CUBE_TX_DMA, CUBE_TX_DMA_STREAM) = sizeof(g_outputBuf[0][0]);

	/* set the current layer as source of the dma */
	DMA_SM0AR(CUBE_TX_DMA, CUBE_TX_DMA_STREAM)  = (uint32_t)&(g_outputBuf[g_currentLayer]);
	DMA_SPAR(CUBE_TX_DMA, CUBE_TX_DMA_STREAM)  = (uint32_t)&(SPI_DR(CUBE_SPI));

	/* set priority*/
	DMA_SCCR(CUBE_TX_DMA, CUBE_TX_DMA_STREAM)  |= DMA_CR_PL_MEDIUM;

	/* set memory size to 8-bit*/
	DMA_SCCR(CUBE_TX_DMA, CUBE_TX_DMA_STREAM)  |= DMA_CR_MSIZE_BYTE;
	DMA_SCCR(CUBE_TX_DMA, CUBE_TX_DMA_STREAM)  |= DMA_CR_PSIZE_BYTE;

	/* enable memory increment */
	DMA_SCCR(CUBE_TX_DMA, CUBE_TX_DMA_STREAM)  |= DMA_CR_MINC;

	/* set direction memory to peripherial */
	DMA_SCCR(CUBE_TX_DMA, CUBE_TX_DMA_STREAM)  |= DMA_CR_DIR;

	/* enable dma */
	DMA_SCCR(CUBE_TX_DMA, CUBE_TX_DMA_STREAM)  |= DMA_CR_EN;
}

static void cube_init(void);
MODULE_INIT_FUNCTION(cube, 8, cube_init)
static void cube_init(void)
{
	uint8_t i;

	RCC_AHB1ENR |= RCC_AHB1ENR_IOPAEN;
	RCC_AHB1ENR |= RCC_AHB1ENR_IOPBEN;
	RCC_AHB1ENR |= RCC_AHB1ENR_IOPCEN;
	RCC_AHB1ENR |= RCC_AHB1ENR_IOPEEN;

	RCC_AHB1ENR |= RCC_AHB1ENR_DMA2EN;

	RCC_APB2ENR |= RCC_APB2ENR_SYSCFGEN;

	gpio_mode_setup(CUBE_SPI_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, CUBE_SPI_CLK_PIN | CUBE_SPI_MOSI_PIN);

	gpio_set_output_options(CUBE_SPI_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_100MHZ, CUBE_SPI_CLK_PIN | CUBE_SPI_MOSI_PIN);

	gpio_set_af(CUBE_SPI_PORT, GPIO_AF5, CUBE_SPI_CLK_PIN | CUBE_SPI_MOSI_PIN);

	/* setup output pins of shift registers and put them in tri-state */
	gpio_mode_setup(CUBE_SHIFT_REGISTERS_NOE_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, CUBE_SHIFT_REGISTERS_NOE_PIN);
	gpio_set_output_options(CUBE_SHIFT_REGISTERS_NOE_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_100MHZ, CUBE_SHIFT_REGISTERS_NOE_PIN);
	gpio_set(CUBE_SHIFT_REGISTERS_NOE_PORT, CUBE_SHIFT_REGISTERS_NOE_PIN);

	gpio_mode_setup(CUBE_SHIFT_REGISTERS_STORE_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, CUBE_SHIFT_REGISTERS_STORE_PIN);
	gpio_set_output_options(CUBE_SHIFT_REGISTERS_STORE_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_100MHZ, CUBE_SHIFT_REGISTERS_STORE_PIN);
	gpio_clear(CUBE_SHIFT_REGISTERS_STORE_PORT, CUBE_SHIFT_REGISTERS_STORE_PIN);

	gpio_mode_setup(CUBE_SHIFT_REGISTERS_N_RESET_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, CUBE_SHIFT_REGISTERS_N_RESET_PIN);
	gpio_set_output_options(CUBE_SHIFT_REGISTERS_N_RESET_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_100MHZ, CUBE_SHIFT_REGISTERS_N_RESET_PIN);
	gpio_set(CUBE_SHIFT_REGISTERS_N_RESET_PORT, CUBE_SHIFT_REGISTERS_N_RESET_PIN);

	/* perform a reset of the registers */
	cube_reset();

	for (i = 0U; i < (sizeof(g_layerGpioDefs) / sizeof(g_layerGpioDefs[0])); ++i)
	{
		gpio_mode_setup(g_layerGpioDefs[i].port, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, g_layerGpioDefs[i].pin);
		gpio_set_output_options(g_layerGpioDefs[i].port, GPIO_OTYPE_OD, GPIO_OSPEED_100MHZ, g_layerGpioDefs[i].pin);
		gpio_clear(g_layerGpioDefs[i].port, g_layerGpioDefs[i].pin);
	}

	/* setup the button */
	EXTI_RTSR |= CUBE_NEXT_RENDER_FUNCTION_BTN_PIN;
	EXTI_IMR  |= CUBE_NEXT_RENDER_FUNCTION_BTN_PIN;
	gpio_mode_setup(CUBE_NEXT_RENDER_FUNCTION_BTN_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, CUBE_NEXT_RENDER_FUNCTION_BTN_PIN);
	/* clear interupt pending bit */
	const uint32_t bla = EXTI_PR;
	EXTI_PR = (bla & (1 << 0));
	nvic_enable_irq(NVIC_EXTI0_IRQ);

	/* wait some time to be sure that the mosfets are nonconducting */
	{
		const systemTime_t curTime = getSystemTimeUS();
		while (curTime + CUBE_RESET_TIME_INTERVAL_US > getSystemTimeUS());
	}

	memset(g_frameBuf, 0, sizeof(g_frameBuf));

	g_currentOutputBuf = 0U;
	g_currentLayer = 0U;

	/* setup the spi interface */
	cube_setupSpiInterface();

	cube_setupDMA();

	cube_pushLatch();

	swTimer_registerOnTimerUS(&onNextLayerTimer, CUBE_LAYER_FRAME_INTERVAL_US, true);
	swTimer_registerOnTimerUS(&onTriggerRenderFunctionTimerCB, CUBE_RENDER_NEW_FRAME_INTERVAL_US, false);

	swTimer_registerOnTimerUS(&triggerNextRenderFunction, CUBE_NEXT_RENDERING_FUNCTION_INTERVAL_US, true);


	msgPump_registerOnMessage(MSG_ID_TRIGGER_RENDER_FUNCTION, &onRenderMsg);

	g_currentRenderingHandle = NULL;
	if (&_renderingHandlesEnd > &_renderingHandlesBegin)
	{
		g_currentRenderingHandle = &_renderingHandlesBegin;

		if (NULL != g_currentRenderingHandle->initFunction)
		{
			(void)(g_currentRenderingHandle->initFunction)(&g_frameBuf);
		}
	}

	if (NULL != g_currentRenderingHandle &&
		(NULL != g_currentRenderingHandle->initFunction))
	{
		(void)(g_currentRenderingHandle->initFunction)(&g_frameBuf);
	}

	cube_enableLayer(g_currentLayer);
}

