/*
 * @brief Hitex EVA 1850/4350 board file
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2012
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "board.h"
#include "string.h"

/* Include other sources files specific to this board */
#include "lpc_phy_dp83848.c"
#include "retarget.c"

/** @ingroup BOARD_HITEX_EVA_18504350
 * @{
 */

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/* Port and bit mapping for LEDs on GPIOs */
static const uint8_t ledports[] = {7 };
static const uint8_t ledbits[] = {14 };

typedef struct {
	int16_t ad_left;						/* left margin */
	int16_t ad_right;						/* right margin */
	int16_t ad_top;							/* top margin */
	int16_t ad_bottom;						/* bottom margin */
	int16_t lcd_width;						/* lcd horizontal size */
	int16_t lcd_height;						/* lcd vertical size */
	uint8_t swap_xy;						/* 1: swap x-y coords */
} TSC2046_Init_Type;

#define DC_CMD          (Chip_GPIO_WritePortBit(0x07, 8, false))
#define DC_DATA         (Chip_GPIO_WritePortBit(0x07, 8, true))

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* Initialize pin muxing for a UART */
void Board_UART_Init(LPC_USART_Type *UARTx)
{
	if (UARTx == LPC_USART0) {
		Chip_SCU_PinMux(0xF, 10, MD_PDN, FUNC1);	/* PF.10 : UART0_TXD */
		Chip_SCU_PinMux(0xF, 11, MD_PLN | MD_EZI | MD_ZI, FUNC1);	/* PF.11 : UART0_RXD */
	}
	else if (UARTx == LPC_UART1) {
		Chip_SCU_PinMux(0xC, 13, MD_PDN, FUNC2);	/* PC.13 : UART1_TXD - pin 1 of SV14 */
		Chip_SCU_PinMux(0xC, 14, MD_PLN | MD_EZI | MD_ZI, FUNC2);	/* PC.14 : UART1_RX - pin 2 of SV14 */
	}
	else if (UARTx == LPC_USART2) {
		/* P1.15 : UART2_TXD - pin 11 of SV6, P1.16 : UART2_RXD - pin 3 of SV6 */
		Chip_SCU_PinMux(0x1, 15, MD_PDN, FUNC1);
		Chip_SCU_PinMux(0x1, 16, MD_PLN | MD_EZI | MD_ZI, FUNC1);
	}
	else if (UARTx == LPC_USART3) {
		/* P9.3 : UART3_TXD - pin 15 of SV6, P9.4 : UART3_RXD - pin 7 of SV3 */
		Chip_SCU_PinMux(0x9, 3, MD_PDN, FUNC7);
		Chip_SCU_PinMux(0x9, 4, MD_PLN | MD_EZI | MD_ZI, FUNC7);
	}
}

/* Initialize debug output via UART for board */
void Board_Debug_Init(void)
{
#if defined(DEBUG_UART)
	Board_UART_Init(DEBUG_UART);

	Chip_UART_Init(DEBUG_UART);
	Chip_UART_SetBaud(DEBUG_UART, 115200);
	Chip_UART_ConfigData(DEBUG_UART, UART_DATABIT_8, UART_PARITY_NONE, UART_STOPBIT_1);

	/* Enable UART Transmit */
	Chip_UART_TxCmd(DEBUG_UART, ENABLE);
#endif
}

/* Sends a character on the UART */
void Board_UARTPutChar(char ch)
{
#if defined(DEBUG_UART)
	while (Chip_UART_SendByte(DEBUG_UART, (uint8_t) ch) == ERROR) {}
#endif
}

/* Gets a character from the UART, returns EOF if no character is ready */
int Board_UARTGetChar(void)
{
#if defined(DEBUG_UART)
	uint8_t data;

	if (Chip_UART_ReceiveByte(DEBUG_UART, &data) == SUCCESS) {
		return (int) data;
	}
#endif
	return EOF;
}

/* Outputs a string on the debug UART */
void Board_UARTPutSTR(char *str)
{
#if defined(DEBUG_UART)
	while (*str != '\0') {
		Board_UARTPutChar(*str++);
	}
#endif
}

/* Initializes board LED(s) */
void Board_LED_Init()
{
	int i;

	/* Set ports as outputs with initial states off */
	for (i = 0; i < (sizeof(ledports) / sizeof(ledports[0])); i++) {
		Chip_GPIO_WriteDirBit(ledports[i], ledbits[i], true);
		Chip_GPIO_WritePortBit(ledports[i], ledbits[i], true);
	}
	Chip_GPIO_WritePortBit(ledports[0], ledbits[0], false);
}

/* Sets the state of a board LED to on or off */
void Board_LED_Set(uint8_t LEDNumber, bool On)
{

	if (LEDNumber < 1) {
		Chip_GPIO_WritePortBit(ledports[LEDNumber], ledbits[LEDNumber], !On);
	}
}

/* Returns the current state of a board LED */
bool Board_LED_Test(uint8_t LEDNumber)
{
	bool On = false;

	if (LEDNumber < 4) {
		On = (bool) !Chip_GPIO_ReadPortBit(ledports[LEDNumber], ledbits[LEDNumber]);

		if (LEDNumber == 0) {
			On = ~On;
		}
	}

	return On;
}

/* Initialize button(s) interface on board */
void Board_Buttons_Init(void)	// FIXME not functional ATM
{
	Chip_SCU_PinMux(0xD, 7, MD_PUP | MD_EZI, FUNC4);			// GPIO6[21]
	Chip_GPIO_WriteDirBit(BUTTONS_BUTTON1_GPIO_PORT_NUM, BUTTONS_BUTTON1_GPIO_BIT_NUM, false);	// input
}

/* Returns button(s) state on board */
uint32_t Buttons_GetStatus(void)
{
	uint8_t ret = NO_BUTTON_PRESSED;
	if (Chip_GPIO_ReadPortBit(BUTTONS_BUTTON1_GPIO_PORT_NUM, BUTTONS_BUTTON1_GPIO_BIT_NUM) == 0) {
		ret |= BUTTONS_BUTTON1;
	}
	return ret;
}

/**
 * System Clock Frequency (Core Clock)
 */
uint32_t SystemCoreClock;

/* Update system core clock rate, should be called if the system has
   a clock rate change */
void SystemCoreClockUpdate(void)
{
	/* CPU core speed */
	SystemCoreClock = Chip_Clock_GetRate(CLK_MX_MXCORE);
}

/* Returns the MAC address assigned to this board */
void Board_ENET_GetMacADDR(uint8_t *mcaddr)
{
	const uint8_t boardmac[] = {0x00, 0x60, 0x37, 0x12, 0x34, 0x56};

	memcpy(mcaddr, boardmac, 6);
}

/**
 * LCD configuration data
 */
const LCD_Config_Type EA320x240 = {
	38,		/* Horizontal back porch in clocks */
	20,		/* Horizontal front porch in clocks */
	30,		/* HSYNC pulse width in clocks */
	320,	/* Pixels per line */
	15,		/* Vertical back porch in clocks */
	5,		/* Vertical front porch in clocks */
	4,		/* VSYNC pulse width in clocks */
	240,	/* Lines per panel */
	0,		/* Invert output enable, 1 = invert */
	1,		/* Invert panel clock, 1 = invert */
	1,		/* Invert HSYNC, 1 = invert */
	1,		/* Invert VSYNC, 1 = invert */
	1,		/* AC bias frequency in clocks (not used) */
	6,		/* Maximum bits per pixel the display supports */
	LCD_TFT,		/* LCD panel type */
	LCD_COLOR_FORMAT_BGR,		/* BGR or RGB */
	0		/* Dual panel, 1 = dual panel display */
};


/* Initialize the LCD for the current board */
void lcdInit(uint32_t framebuffer_addr)
{
	/* Board specific LCD pre-setup */
	Board_LCD_Init();

	/* Setup for current board */
	Chip_LCD_Init( (LCD_Config_Type *) &EA320x240);
	Chip_LCD_SetUPFrameBuffer( (void *) framebuffer_addr);
	Chip_LCD_Power(ENABLE);
// TODO 	lcdDelay(100);

	/* Turn on backlight */
	Board_LCD_Set_Backlight(1);
}

/* Set up and initialize all required blocks and functions related to the
   board hardware */
void Board_Init(void)
{
	/* Sets up DEBUG UART */
	DEBUGINIT();

	/* Updates SystemCoreClock global var with current clock speed */
	SystemCoreClockUpdate();

	/* Initializes GPIO */
	Chip_GPIO_Init();

	/* Setup GPIOs for USB demos */
#if 0 /* FIXME: the following call removed on the Hitex board as it interferes with muxed MII state */
	//	Chip_SCU_PinMux(0x9, 5, (MD_PUP | MD_EZI), FUNC2);			// P9_5 USB1_VBUS_EN, USB1 VBus function
#endif
	Chip_SCU_PinMux(0x2, 5, (MD_PLN | MD_EZI | MD_ZI), FUNC2);	// P2_5 USB1_VBUS, MUST CONFIGURE THIS SIGNAL FOR USB1 NORMAL OPERATION
	Chip_SCU_PinMux(0x6, 3, (MD_PUP | MD_EZI), FUNC1);			// P6_3 USB0_PWR_EN, USB0 VBus function

}

/* Sets up board specific ADC interface */
void Board_ADC_Init(void)
{
	/* Analog function ADC1_2 selected on pin PF_9 */
	Chip_SCU_ADC_Channel_Config(1, 2);
}

/* Sets up board specific I2C interface */
void Board_I2C_Init(LPC_I2C_Type *I2Cx)
{
	if (I2Cx == LPC_I2C1) {
		/* Configure pin function for I2C1 on PE.13 (I2C1_SDA) and PE.15 (I2C1_SCL) */
		Chip_SCU_PinMux(0xE, 13, MD_ZI | MD_EZI, FUNC2);
		Chip_SCU_PinMux(0xE, 15, MD_ZI | MD_EZI, FUNC2);
	}
}

/* Initialize the LCD interface */
void Board_LCD_Init(void)
{
	/* Attach main PLL clock to divider A with a divider of 2 */
	Chip_Clock_SetDivider(CLK_IDIV_A, CLKIN_MAINPLL, 2);

	/* Route divider A output to LCD base clock and enable base clock */
	Chip_Clock_SetBaseClock(CLK_BASE_LCD, CLKIN_IDIVA, true, false);

	/* Reset LCD and wait for reset to complete */
	Chip_RGU_TriggerReset(RGU_LCD_RST);
	while (Chip_RGU_InReset(RGU_LCD_RST)) {}

}

/* Initialize TSC2046 touchscreen controller */
void Init_Touch_Controller(void)
{
	// TODO
}

/* Get Touch coordinates */
bool GetTouchPos(int16_t *pX, int16_t *pY)
{
	// TODO
	return true;
}

/* Turn on Board LCD Backlight */
void Board_LCD_Set_Backlight(uint8_t Intensity)
{
	bool OnOff = (bool) (Intensity != 0);

	Chip_GPIO_WriteDirBit(4, 11, true);
	Chip_GPIO_WritePortBit(4,11, OnOff);
}

/* Initialize pin muxing for SSP interface */
void Board_SSP_Init(LPC_SSP_Type *SSPx)
{
	if (SSPx == LPC_SSP0) {
		/* Set up clock and muxing for SSP0 interface */
		// #if !defined(HITEX_LCD_TERM)
		/* PC.11 connected to GPIO = SSEL_MUX_A, PC.12 connected to GPIO = SSEL_MUX_B */
		Chip_SCU_PinMux(0xC, 11, MD_PLN, FUNC4);
		Chip_SCU_PinMux(0xC, 12, MD_PLN, FUNC4);
		Chip_GPIO_WriteDirBit(0x6, 10, true);
		Chip_GPIO_WriteDirBit(0x6, 11, true);
		// #endif
		/* PF.0 connected to SCL/SCLK	func2=SSP0 SCK0 */
		Chip_SCU_PinMux(0xF, 0, MD_PLN_FAST, FUNC0);
		/* PF.1 connected to nCS		func2=SSP0 SSEL0 */
		Chip_SCU_PinMux(0xF, 1, MD_PLN_FAST, FUNC2);
		/* PF.2 connected to SO			func2=SSP0 MISO0 */
		Chip_SCU_PinMux(0xF, 2, MD_PLN | MD_EZI | MD_ZI, FUNC2);
		/* PF.3 connected to nSI		func2=SSP0 MOSI0 */
		Chip_SCU_PinMux(0xF, 3, MD_PLN | MD_EZI | MD_ZI, FUNC2);

		Chip_Clock_Enable(CLK_MX_SSP0);
	}
	else if (SSPx == LPC_SSP1) {
		/* Set up clock and muxing for SSP1 interface */
		/* P1.19 connected to SCL/SCLK	func1=SSP1 SCK1 */
		Chip_SCU_PinMux(0x1, 19, MD_PLN_FAST, FUNC1);
		/* P1.20 connected to nCS		func1=SSP1 SSEL1 */
		Chip_SCU_PinMux(0x1, 20, MD_PLN_FAST, FUNC1);
		/* P0.0 connected to SO			func1=SSP1 MISO1 */
		Chip_SCU_PinMux(0x0, 0, MD_PLN | MD_EZI | MD_ZI, FUNC1);
		/* P0.1 connected to nSI		func2=SSP1 MOSI1 */
		Chip_SCU_PinMux(0x0, 1, MD_PLN | MD_EZI | MD_ZI, FUNC1);

		Chip_Clock_Enable(CLK_MX_SSP1);
	}
}

/**
 * @}
 */
