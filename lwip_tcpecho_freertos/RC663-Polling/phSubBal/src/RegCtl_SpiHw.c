/*
 * RegCtl_SPIHw.c
 *
 *  Created on: 8/01/2012
 *      Author: paolo
 */

#include "board.h"

typedef uint16_t phStatus_t;
#define PH_ERR_SUCCESS                  0x0000U /**< Returned in case of no error. */

#define DEBUG_REG	0

/*******************************************************************************
**   Main Function  main()
*******************************************************************************/

/*********************************************************************//**
 * @brief 		Initialise the i2c bus for master mode
 * @param[in]	None
 *
 * @return 		None
 **********************************************************************/
void RegCtl_SpiHwInit(void)
{

	SSP_ConfigFormat ssp_format1;

  /* Configure SSP0 pins*/
	Chip_SCU_PinMux(0xF,0,MD_PDN,FUNC0);	// PF_0 connected to SCL/SCLK	func0=SSP0 SCK
	Chip_SCU_PinMux(0x9,0,MD_PDN,FUNC0);	// P9_0 connected to nCS  RC663	func2=SSP0 SSEL0 as standard I/O pin GPIO4.12
	Chip_SCU_PinMux(0x9,2,MD_PDN,FUNC7);	// P9_2 connected to MOSI RC663	func2=SSP0 MOSI0
	Chip_SCU_PinMux(0x9,1,MD_EZI,FUNC7);	// P9_1 connected to MISO RC663	func2=SSP0 MISO0
	
    /* Set port pin P4.12 to output for SSEL0 */
	Chip_GPIO_WriteDirBit(0x4, 12, true);
	Chip_GPIO_WritePortBit(0x4, 12, true);
    
	Chip_Clock_Enable(CLK_MX_SSP0);
	Chip_SSP_Init(LPC_SSP0);
	Chip_SSP_Set_Master(LPC_SSP0, true);
	Chip_SSP_Set_BitRate(LPC_SSP0, 1000000);

	ssp_format1.frameFormat = SSP_FRAMEFORMAT_SPI;
	ssp_format1.bits = SSP_BITS_8;
	ssp_format1.clockFormat = SSP_CLOCK_MODE0;

	Chip_SSP_Set_Format(LPC_SSP0, &ssp_format1);
	Chip_SSP_Cmd(LPC_SSP0, ENABLE);

}

/*********************************************************************//**
 * @brief 		Get a databyte from a register
 * @param[in]	address		Address of the register
 * @param[in]	*reg_data	Pointer to the databyte to be read
 *
 * @return 		- PH_ERR_BFL_SUCCESS
 *              - PH_ERR_INTERFACE_ERROR
 **********************************************************************/
phStatus_t RegCtl_SpiHwGetReg(uint8_t address, uint8_t *reg_data)
{

  Chip_SSP_DATA_SETUP_Type xferConfig;
  uint8_t Tx_Buf[2];    // Tx buffer
  uint8_t Rx_Buf[2];    // Rx buffer
   
    /* Asser SSEL0 port pin P0.17 */
//	GPIO_ClearValue(0x4, (1<<12));
    
    // address = (0x7f << 1) +1;
    Tx_Buf[0] = address;
    Tx_Buf[1] = 0x00;
    Rx_Buf[0] = 0x00;
    Rx_Buf[1] = 0x00;
    
    xferConfig.length = 2;
    xferConfig.tx_data = Tx_Buf;
    xferConfig.tx_cnt = 0;
    xferConfig.rx_data = reg_data;
    xferConfig.rx_cnt = 0;
#if 0
	/* Clear all remaining data in RX FIFO */
	while (LPC_SSP0->SR & SSP_SR_RNE){
		Rx_Buf[0] = (uint32_t) SSP_ReceiveData(LPC_SSP0);
	}
    
	GPIO_ClearValue(0x4, (1<<12));
        LPC_SSP0->DR = SSP_DR_BITMASK(Tx_Buf[0]);
        while( ( LPC_SSP0->SR & SSP_SR_RNE ) == 0 );
	reg_data[0] = SSP_ReceiveData(LPC_SSP0);
        LPC_SSP0->DR = SSP_DR_BITMASK(Tx_Buf[1]);
        while( ( LPC_SSP0->SR & SSP_SR_RNE ) == 0 );
	reg_data[1] = SSP_ReceiveData(LPC_SSP0);
#else
	Chip_GPIO_WritePortBit(0x4, 12, false);
    Chip_SSP_RWFrames_Blocking(LPC_SSP0, &xferConfig);
#endif    
   /* Deassert SSEL0 port pin P0.17  */  
	Chip_GPIO_WritePortBit(0x4, 12, true);
    
#if DEBUG_REG
	debug_printf("GR");
	debug_puts_hex(address);
	debug_puts_hex(reg_data[1]);
#endif

	/* return success */
	return PH_ERR_SUCCESS;

}

/*********************************************************************//**
 * @brief 		Set a databyte from a register
 * @param[in]	address		Address of the register
 * @param[in]	*reg_data	Databyte to be written
 *
 * @return 		- PH_ERR_BFL_SUCCESS
 *              - PH_ERR_INTERFACE_ERROR
 **********************************************************************/
phStatus_t RegCtl_SpiHwSetReg(uint8_t address, uint8_t reg_data)
{

	uint8_t buf[2];
#if DEBUG_REG
	debug_printf("SR");
	debug_puts_hex(address);
	debug_puts_hex(reg_data);
#endif
   /* Asser SSEL0 port pin P4.12 */
	Chip_GPIO_WritePortBit(0x4, 12, false);

	buf[0] = address;
	buf[1] = reg_data;

	Chip_SSP_WriteFrames_Blocking(LPC_SSP0, buf, 2);

	   /* Dessser SSEL0 port pin P2.12 */
	Chip_GPIO_WritePortBit(0x4, 12, true);

   return PH_ERR_SUCCESS;
}

/*********************************************************************//**
 * @brief 		Modify a bit in a register
 * @param[in]	address		Address of the register
 * @param[in]	*mask		Mask of the bit to set/reset
 * @param[in]	set			1: set 1: reset
 *
 * @return 		- PH_ERR_BFL_SUCCESS
 *
 **********************************************************************/
phStatus_t RegCtl_SpiHwModReg(uint8_t address, uint8_t mask, uint8_t set)
{

    uint8_t  reg_data;

    /* get the register first */
    RegCtl_SpiHwGetReg(address, &reg_data);

	if (set)
	{
		/* The bits of the mask, set to one are set in the new data: */
		reg_data |= mask;
	}
	else
	{
		/* The bits of the mask, set to one are cleared in the new data: */
		reg_data &= (uint8_t)(~mask);
	}

	/* set the register */
	RegCtl_SpiHwSetReg(address, reg_data);

    return PH_ERR_SUCCESS;
}

