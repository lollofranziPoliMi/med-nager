/*
 * Copyright(C) NXP Semiconductors, 2012
 * All rights reserved.
 *
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
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "board.h"
//#define CHIP_LPC43XX	1  see sys_config.h
//#define USE_RMII		1  see sys_config.h

/** @defgroup BOARD_HITEX_EVA_18504350_SYSINIT LPC1850 and LPC4350 Hitex EVA board System Init code
 * @ingroup BOARD_HITEX_EVA_18504350
 * The System initialization code is called prior to the application and
 * initializes the board for run-time operation. Board initialization
 * for the Hitex EVA boards includes clock setup, default pin muxing, and
 * memory configuration.
 *
 * With the exception of stack space, no RW memory is used for this call.
 *
 * SerizII<BR>
 *  Clocking:<BR>
 *   All base clocks enabled by default (Save power by disabling un-needed clocks)<BR>
 *   CPU PLL set to maximum clock frequency (as defined by MAX_CLOCK_FREQ value)<BR>
 *   SPIFI FLASH clock setup for fastest speed<BR>
 *  Pin muxing:<BR>
 *   Sets up various pin mux functions for the board (Ethernet, LEDs, etc.)<BR>
 *   Sets up the external memory controller signals<BR>
 *  Memory:<BR>
 *   Sets up DRAM, static RAM, and NOR FLASH.
 * @{
 */

#ifndef CORE_M0
/* SCR pin definitions for pin muxing */
typedef struct {
	uint8_t pingrp;	/* Pin group */
	uint8_t pinnum;	/* Pin number */
	uint8_t pincfg;	/* Pin configuration for SCU */
	uint8_t funcnum;/* Function number */
} PINMUX_GRP_T;

/* Structure for initial base clock states */
struct CLK_BASE_STATES {
	CGU_BASE_CLK_T clk;	/* Base clock */
	CGU_CLKIN_T clkin;	/* Base clock source, see UM for allowable souorces per base clock */
	bool autoblock_enab;/* Set to true to enable autoblocking on frequency change */
	bool powerdn;		/* Set to true if the base clock is initially powered down */
};

/* Initial base clock states are mostly on */
STATIC const struct CLK_BASE_STATES InitClkStates[] = {
	{CLK_BASE_SAFE, CLKIN_IRC, true, false},
	{CLK_BASE_APB1, CLKIN_MAINPLL, true, false},
	{CLK_BASE_APB3, CLKIN_MAINPLL, true, false},
	{CLK_BASE_USB0, CLKIN_USBPLL, true, false},
#if defined(CHIP_LPC43XX)
	{CLK_BASE_PERIPH, CLKIN_MAINPLL, true, false},
#endif
	{CLK_BASE_USB1, CLKIN_USBPLL, true, false},
#if defined(CHIP_LPC43XX)
	{CLK_BASE_SPI, CLKIN_MAINPLL, true, false},
#endif
	{CLK_BASE_PHY_TX, CLKIN_ENET_TX, true, false},
#if defined(USE_RMII)
	{CLK_BASE_PHY_RX, CLKIN_ENET_TX, true, false},
#else
	{CLK_BASE_PHY_RX, CLKIN_ENET_RX, true, false},
#endif
	{CLK_BASE_LCD, CLKIN_MAINPLL, true, true},
#if defined(CHIP_LPC43XX)
	{CLK_BASE_VADC, CLKIN_MAINPLL, true, true},
#endif
	{CLK_BASE_SDIO, CLKIN_MAINPLL, true, false},
	{CLK_BASE_SSP0, CLKIN_MAINPLL, true, false},
	{CLK_BASE_SSP1, CLKIN_MAINPLL, true, false},
	{CLK_BASE_UART0, CLKIN_MAINPLL, true, false},
	{CLK_BASE_UART1, CLKIN_MAINPLL, true, false},
	{CLK_BASE_UART2, CLKIN_MAINPLL, true, false},
	{CLK_BASE_UART3, CLKIN_MAINPLL, true, false},
	{CLK_BASE_OUT, CLKINPUT_PD, true, false},
	{CLK_BASE_APLL, CLKINPUT_PD, true, false},
	{CLK_BASE_CGU_OUT0, CLKINPUT_PD, true, false},
	{CLK_BASE_CGU_OUT1, CLKINPUT_PD, true, false}
};

/* SPIFI high speed pin mode setup */
STATIC const PINMUX_GRP_T spifipinmuxing[] = {
	{0x3, 3,  (MD_PLN_FAST),  FUNC3},	/* SPIFI CLK */
	{0x3, 4,  (MD_PLN_FAST),  FUNC3},	/* SPIFI D3 */
	{0x3, 5,  (MD_PLN_FAST),  FUNC3},	/* SPIFI D2 */
	{0x3, 6,  (MD_PLN_FAST),  FUNC3},	/* SPIFI D1 */
	{0x3, 7,  (MD_PLN_FAST),  FUNC3},	/* SPIFI D0 */
	{0x3, 8,  (MD_PLN_FAST),  FUNC3}	/* SPIFI CS/SSEL */
};

/* Setup system clocking */
STATIC void SystemSetupClocking(void)
{
	int i;

	/* Switch main system clocking to crystal */
	Chip_Clock_EnableCrystal();
	Chip_Clock_SetBaseClock(CLK_BASE_MX, CLKIN_CRYSTAL, true, false);

	/* Setup PLL for 100MHz and switch main system clocking */
	Chip_Clock_SetupMainPLLHz(CLKIN_CRYSTAL, CRYSTAL_MAIN_FREQ_IN, 100 * 1000000, 100 * 1000000);
	Chip_Clock_SetBaseClock(CLK_BASE_MX, CLKIN_MAINPLL, true, false);

	/* Setup PLL for maximum clock */
	Chip_Clock_SetupMainPLLHz(CLKIN_CRYSTAL, CRYSTAL_MAIN_FREQ_IN, MAX_CLOCK_FREQ, MAX_CLOCK_FREQ);

	/* Setup system base clocks and initial states. This won't enable and
	   disable individual clocks, but sets up the base clock sources for
	   each individual peripheral clock. */
	for (i = 0; i < (sizeof(InitClkStates) / sizeof(InitClkStates[0])); i++) {
		Chip_Clock_SetBaseClock(InitClkStates[i].clk, InitClkStates[i].clkin,
								InitClkStates[i].autoblock_enab, InitClkStates[i].powerdn);
	}

#define RESET32KHZ	( 1 << 2 )
#define	PD32KHZ		( 1 << 3 )
	/* Reset and enable 32Khz oscillator */
	LPC_CREG->CREG0 &= ~( PD32KHZ | RESET32KHZ);
	LPC_CREG->CREG0 |= (1 << 1) | (1 << 0);

	/* SPIFI pin setup is done prior to setting up system clocking */
	for (i = 0; i < (sizeof(spifipinmuxing) / sizeof(spifipinmuxing[0])); i++) {
		Chip_SCU_PinMux(spifipinmuxing[i].pingrp, spifipinmuxing[i].pinnum,
						spifipinmuxing[i].pincfg, spifipinmuxing[i].funcnum);
	}

	/* Setup a divider E for main PLL clock switch SPIFI clock to that divider.
	   Divide rate is based on CPU speed and speed of SPI FLASH part. */
#if (MAX_CLOCK_FREQ > 180000000)
	Chip_Clock_SetDivider(CLK_IDIV_E, CLKIN_MAINPLL, 5);
#else
	Chip_Clock_SetDivider(CLK_IDIV_E, CLKIN_MAINPLL, 4);
#endif
	Chip_Clock_SetBaseClock(CLK_BASE_SPIFI, CLKIN_IDIVE, true, false);
}

STATIC const PINMUX_GRP_T pinmuxing[] = {
#if defined(USE_RMII)
	/* RMII pin group */
	{0x1, 19, (MD_EHS | MD_PLN | MD_EZI | MD_ZI),  FUNC0},	// ENET_TX_CLK
	{0xC, 4,  (MD_EHS | MD_PLN | MD_ZI),           FUNC3},  // ENET_TX_EN
	{0x1, 18, (MD_EHS | MD_PLN | MD_ZI),           FUNC3},	// ENET_TXD0
	{0x1, 20, (MD_EHS | MD_PLN | MD_ZI),           FUNC3},	// ENET_TXD1
	{0xC, 8,  (MD_EHS | MD_PLN | MD_EZI | MD_ZI),  FUNC3},	// ENET_RX_DV
	{0xC, 9,  (MD_EHS | MD_PLN | MD_EZI | MD_ZI),  FUNC3},	// ENET_RX_ER
	{0x1, 15, (MD_EHS | MD_PLN | MD_EZI | MD_ZI),  FUNC3},	// ENET_RXD0
	{0x0, 0,  (MD_EHS | MD_PLN | MD_EZI | MD_ZI),  FUNC2},	// ENET_RXD1
	{0x1, 16, (MD_EHS | MD_PLN | MD_EZI | MD_ZI),  FUNC3},	// ENET_CRS
	{0x1, 17, (MD_EHS | MD_PLN | MD_EZI | MD_ZI),  FUNC3},	// ENET_MDIO
	{0x7, 7,  (MD_EHS | MD_PLN | MD_ZI),           FUNC6},	// ENET_MDC
	{0x7, 5,  (MD_PLN),                            FUNC0}, 	// 50MHz_EN
#else
	/* MII pin group */
	{0x1, 19, (MD_PLN | MD_EZI),                   FUNC0},
	{0x0, 1,  (MD_PLN),                            FUNC6},
	{0x1, 18, (MD_PLN),                            FUNC3},
	{0x1, 20, (MD_PLN),                            FUNC3},
	{0x1, 17, (MD_PLN | MD_EZI),                   FUNC3},
	{0xC, 1,  (MD_PLN),                            FUNC3},
	{0x1, 16, (MD_PLN | MD_EZI),                   FUNC7},
	{0x1, 15, (MD_PLN | MD_EZI),                   FUNC3},
	{0x0, 0,  (MD_PLN | MD_EZI),                   FUNC2},
	{0x9, 4,  (MD_PLN),                            FUNC5},
	{0x9, 5,  (MD_PLN),                            FUNC5},
	{0xC, 0,  (MD_PLN | MD_EZI),                   FUNC3},
	{0x9, 0,  (MD_PLN | MD_EZI),                   FUNC5},
	{0x9, 1,  (MD_PLN | MD_EZI),                   FUNC5},
	{0x9, 6,  (MD_PLN | MD_EZI),                   FUNC5},
	{0x9, 3,  (MD_PLN | MD_EZI),                   FUNC5},
	{0x9, 2,  (MD_PLN | MD_EZI),                   FUNC5},
	{0xC, 8,  (MD_PLN | MD_EZI),                   FUNC4},
#endif
	/* External data lines D0 .. D15 */
	{0x1, 7,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC3},
	{0x1, 8,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC3},
	{0x1, 9,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC3},
	{0x1, 10, (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC3},
	{0x1, 11, (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC3},
	{0x1, 12, (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC3},
	{0x1, 13, (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC3},
	{0x1, 14, (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC3},
	{0x5, 4,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC2},
	{0x5, 5,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC2},
	{0x5, 6,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC2},
	{0x5, 7,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC2},
	{0x5, 0,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC2},
	{0x5, 1,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC2},
	{0x5, 2,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC2},
	{0x5, 3,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC2},
	/* Address lines A0 .. A23 */
	{0x2, 9,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC3},
	{0x2, 10, (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC3},
	{0x2, 11, (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC3},
	{0x2, 12, (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC3},
	{0x2, 13, (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC3},
	{0x1, 0,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC2},
	{0x1, 1,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC2},
	{0x1, 2,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC2},
	{0x2, 8,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC3},
	{0x2, 7,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC3},
	{0x2, 6,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC2},
	{0x2, 2,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC2},
	{0x2, 1,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC2},
	{0x2, 0,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC2},
	{0x6, 8,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC1},
	{0x6, 7,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC1},
	{0xD, 16, (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC2},
	{0xD, 15, (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC2},
	{0xE, 0,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC3},
	{0xE, 1,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC3},
	{0xE, 2,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC3},
	{0xE, 3,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC3},
	{0xE, 4,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC3},
	{0xA, 4,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC3},
	/* EMC control signals */
	{0x1, 4,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC3},  	// EMC_BLSO
	{0x6, 6,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC1},  	// EMC_BLS1
//	{0xD, 13, (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC2},  	// EMC_BLS2 - not used, T14
//	{0xD, 10, (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC2},  	// EMC_BLS3 - not used, P11
	{0x6, 9,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC3},  	// EMC_DYCS0 - J15
	{0x1, 6,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC3},		// EMC_WE - T4
	{0x6, 4,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC3},		// EMC_CAS - R16
	{0x6, 5,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC3},		// EMC_RAS - P16
	{PINMUX_CLK, 0,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC0}, // EMC_CLK0 - N5
	{PINMUX_CLK, 1,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC0}, // EMC_CLK1
	{PINMUX_CLK, 2,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC0}, // EMC_CLK2
	{PINMUX_CLK, 3,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC0}, // EMC_CLK3
	{0x6, 11, (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC3}, 		// EMC_CLKE - H12
	{0x6, 12, (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC3}, 		// EMC_DQM0 - G15
	{0x6, 10, (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC3}, 		// EMC_DQM1 - H15
//	{0xD, 0,  (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC2},		// EMC_DQM2 - N2
//	{0xE, 13, (MD_PLN | MD_EZI | MD_ZI | MD_EHS),  FUNC3},		// EMC_DQM3 - G14
	{0x1, 3,  MD_PLN_FAST,                         FUNC3},		// EMC_OE - P5
	{0x1, 4,  MD_PLN_FAST,                         FUNC3},  	// EMC_BLSO, already programmed ???
	{0x6, 6,  MD_PLN_FAST,                         FUNC3},  	// EMC_BLS1, already programmed ???
	{0x1, 5,  MD_PLN_FAST,                         FUNC3},  	// EMC_CS0
	{0x6, 3,  MD_PLN_FAST,                         FUNC3},  	// EMC_CS1
	/* LCD interface, 16bpp */
	{0x4,	3,	MD_PLN_FAST,	FUNC2},	//LCD_VD2
    {0x7,	4,	MD_PLN_FAST,	FUNC4},	//LCD_VD4
    {0x8,	6,	MD_PLN_FAST,	FUNC3},	//LCD_VD5
    {0x8,	5,	MD_PLN_FAST,	FUNC3},	//LCD_VD6
	{0x7,	1,	MD_PLN_FAST,	FUNC4},	//LCD_VD7
    {0x8,	3,	MD_PLN_FAST,	FUNC3},	//LCD_VD12
	{0x4,	0,	MD_PLN_FAST,	FUNC5},	//LCD_VD13
	{0x3,	2,	MD_PLN_FAST,	FUNC6},	//LCD_VD14
	{0x3,	1,	MD_PLN_FAST,	FUNC6},	//LCD_VD15
	{0x7,	2,	MD_PLN_FAST,	FUNC3},	//LCD_VD18
    {0xB,	6,	MD_PLN_FAST,	FUNC6},	//LCD_VD19
    {0xB,	3,	MD_PLN_FAST,	FUNC2},	//LCD_VD20
    {0xB,	2,	MD_PLN_FAST,	FUNC2},	//LCD_VD21
    {0xB,	1,	MD_PLN_FAST,	FUNC2},	//LCD_VD22
    {0xB,	0,	MD_PLN_FAST,	FUNC2},	//LCD_VD23

	{0x4,	5,	MD_PLN_FAST,	FUNC2},	//LCD_FP
    {0x7,	6,	MD_PLN_FAST,	FUNC3},	//LCD_LP
	{0x4,	7,	MD_PLN_FAST,	FUNC0},	//LCD_DCLK
//	(0x4,	6,	MD_PLN_FAST,	FUNC2},	//LCD_ENAB
	{0x4,	6,	MD_PLN_FAST,	FUNC0},	//GPIO2.6
//	(0x7,	0,	MD_PLN_FAST,	FUNC3},	//LCD_LE
	{0x7,	0,	MD_PLN_FAST,	FUNC0},	//GPIO3.8
	/* Nacklight */
	{0x9, 6,  MD_PDN,                              FUNC0},  /* GPIO4.11 */
	/* Board LEDs */
	{0xE,14,  MD_PDN,                              FUNC4},  /* GPIO7.14 */
	/* SSP0 */
	/* ise set under the RS663 initialization */
	/* Board ADC */
//	{0xF, 9,  MD_PLN,                              FUNC7},
	/*  I2S  */
//	{0x3, 0,  MD_PLN_FAST,                         FUNC2},
//	{0x6, 0,  MD_PLN_FAST,                         FUNC4},
//	{0x7, 2,  MD_PLN_FAST,                         FUNC2},
//	{0x6, 2,  MD_PLN_FAST,                         FUNC3},
//	{0x7, 1,  MD_PLN_FAST,                         FUNC2},
//	{0x6, 1,  MD_PLN_FAST,                         FUNC3},
};

/* Sets up system pin muxing */
STATIC void SystemSetupMuxing(void)
{
	int i;

	/* Setup system level pin muxing */
	for (i = 0; i < (sizeof(pinmuxing) / sizeof(pinmuxing[0])); i++) {
		Chip_SCU_PinMux(pinmuxing[i].pingrp, pinmuxing[i].pinnum,
						pinmuxing[i].pincfg, pinmuxing[i].funcnum);
	}
	/* enable 50MHz ENET oscillator */
	Chip_GPIO_WriteDirBit(3, 13, true);
	Chip_GPIO_WritePortBit(3,13, true);

}

/* EMC clock delay */
#define CLK0_DELAY 7

/* Hitex SDRAM timing and chip Config */
STATIC const IP_EMC_DYN_CONFIG_Type MT48LC4M16A2_config = {
	EMC_NANOSECOND(64000000 / 4096),	/* Row refresh time */
	0x01,	/* Command Delayed */
	EMC_NANOSECOND(20),
	EMC_NANOSECOND(42),
	EMC_NANOSECOND(63),
	EMC_CLOCK(0x05),
	EMC_CLOCK(0x05),
	EMC_CLOCK(0x02),
	EMC_NANOSECOND(63),
	EMC_NANOSECOND(63),
	EMC_NANOSECOND(63),
	EMC_NANOSECOND(14),
	EMC_CLOCK(0x02),
	{
		{
			EMC_ADDRESS_DYCS0,	/* SerizII Board uses only DYCS0 for SDRAM */
			3,	/* RAS */

			EMC_DYN_MODE_WBMODE_PROGRAMMED |
			EMC_DYN_MODE_OPMODE_STANDARD |
			EMC_DYN_MODE_CAS_3 |
			EMC_DYN_MODE_BURST_TYPE_SEQUENTIAL |
			EMC_DYN_MODE_BURST_LEN_8,

			EMC_DYN_CONFIG_DATA_BUS_16 |
			EMC_DYN_CONFIG_4Mx16_4BANKS_12ROWS_8COLS |
			EMC_DYN_CONFIG_MD_SDRAM

		},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0}
	}
};

/* Hitex NorFlash timing and chip Config */
STATIC const IP_EMC_STATIC_CONFIG_Type SST39VF320_config = {
	0,
	EMC_STATIC_CONFIG_MEM_WIDTH_16 |
	EMC_STATIC_CONFIG_CS_POL_ACTIVE_LOW |
	EMC_STATIC_CONFIG_BLS_HIGH /* |
							      EMC_CONFIG_BUFFER_ENABLE*/,

	EMC_NANOSECOND(0),
	EMC_NANOSECOND(35),
	EMC_NANOSECOND(70),
	EMC_NANOSECOND(70),
	EMC_NANOSECOND(40),
	EMC_CLOCK(4)
};

/* Setup external memories */
STATIC void SystemSetupMemory(void)
{
	/* Setup EMC Delays */
	/* Move all clock delays together */
	LPC_SCU->EMCDELAYCLK = ((CLK0_DELAY) | (CLK0_DELAY << 4) | (CLK0_DELAY << 8) | (CLK0_DELAY << 12));

	/* Setup EMC Clock Divider for divide by 2 */
	Chip_Clock_EnableOpts(CLK_MX_EMC_DIV, true, true, 2);
	LPC_CREG->CREG6 |= (1 << 16);
	Chip_Clock_Enable(CLK_MX_EMC);

	/* Init EMC Controller -Enable-LE mode- clock ratio 1:1 */
	Chip_EMC_Init(1, 0, 0);
	/* Init EMC Dynamic Controller */
	Chip_EMC_Dynamic_Init((IP_EMC_DYN_CONFIG_Type *) &MT48LC4M16A2_config);
	/* Init EMC Static Controller CS0 */
	Chip_EMC_Static_Init((IP_EMC_STATIC_CONFIG_Type *) &SST39VF320_config);

	/* Enable Buffer for External Flash */
	LPC_EMC->STATICCONFIG0 |= 1 << 19;
}

#endif

/*
 * Setup LCD
 */
/**
 * @brief	Setup the system
 *			SystemInit() is called prior to the application and sets up system
 *			clocking, memory, and any resources needed prior to the application
 *			starting.
 * @return	none
 */
void SystemInit(void)
{
#if defined(CORE_M3) || defined(CORE_M4)
	unsigned int *pSCB_VTOR = (unsigned int *) 0xE000ED08;

#if defined(__IAR_SYSTEMS_ICC__)
	extern void *__vector_table;

	*pSCB_VTOR = (unsigned int) &__vector_table;
#elif defined(__CODE_RED)
	extern void *g_pfnVectors;

	*pSCB_VTOR = (unsigned int) &g_pfnVectors;
#elif defined(__ARMCC_VERSION)
	extern void *__Vectors;

	*pSCB_VTOR = (unsigned int) &__Vectors;
#endif

#if defined(__FPU_PRESENT) && __FPU_PRESENT == 1
	fpuInit();
#endif

	/* Setup system clocking and memory. This is done early to allow the
	   application and tools to clear memory and use scatter loading to
	   external memory. */
	SystemSetupClocking();
	SystemSetupMuxing();
	SystemSetupMemory();
#endif
}

/**
 * @}
 */
