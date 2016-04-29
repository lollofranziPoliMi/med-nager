/*
 * RegCtl_I2cHw.c
 *
 *  Created on: 19 janv. 2011
 *      Author: frq04766
 */

#include <driver_config.h>
#include <ph_Status.h>
#include <i2c.h>
#include <RegCtl_I2cHw.h>
#include <debug_printf.h>

extern volatile uint32_t I2CCount;
extern volatile uint8_t I2CMasterBuffer[BUFSIZE];
extern volatile uint8_t I2CSlaveBuffer[BUFSIZE];
extern volatile uint32_t I2CMasterState;
extern volatile uint32_t I2CReadLength, I2CWriteLength;

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
void RegCtl_I2cHwInit(void)
{
	  I2CInit((uint32_t)I2CMASTER);	/* initialize I2c */
}

/*********************************************************************//**
 * @brief 		Get a databyte from a register
 * @param[in]	address		Address of the register
 * @param[in]	*reg_data	Pointer to the databyte to be read
 *
 * @return 		- PH_ERR_BFL_SUCCESS
 *              - PH_ERR_INTERFACE_ERROR
 **********************************************************************/
phStatus_t RegCtl_I2cHwGetReg(uint8_t address, uint8_t *reg_data)
{
	/* Write SLA(W), address and one data byte */
	I2CWriteLength = 2;					/* 2 bytes to write */
	I2CReadLength = 0;					/* no bytes to read */
	I2CMasterBuffer[0] = RC663_ADDR;
	I2CMasterBuffer[1] = address;		/* address */
	I2CEngine();

	/* Operation not OK ? */
	if (I2C_OK != I2CMasterState)
	{
		/* other return error */
		return PH_ERR_INTERFACE_ERROR;
	}

	/* Write SLA(R) and read one byte back. */
	I2CWriteLength = 1;
	I2CReadLength = 1;
	I2CMasterBuffer[0] = RC663_ADDR | RD_BIT;	/* address */
	I2CEngine();

	/* Operation not OK ? */
	if (I2C_OK != I2CMasterState)
	{
		/* no data read - return a dummy databyte */
		*reg_data = 0xFFU;

		/* other return error */
		return PH_ERR_INTERFACE_ERROR;
	}

	/* return the byte read */
	*reg_data = I2CSlaveBuffer[0];

#if DEBUG_REG
	debug_printf("GR");
	debug_puts_hex(address);
	debug_puts_hex(I2CSlaveBuffer[0]);
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
phStatus_t RegCtl_I2cHwSetReg(uint8_t address, uint8_t reg_data)
{
#if DEBUG_REG
	debug_printf("SR");
	debug_puts_hex(address);
	debug_puts_hex(reg_data);
#endif

	/* Write SLA(W), address and one data byte */
	I2CWriteLength = 3;					/* 3 bytes to write */
	I2CReadLength = 0;					/* no bytes to read */
	I2CMasterBuffer[0] = RC663_ADDR;
	I2CMasterBuffer[1] = address;		/* address */
	I2CMasterBuffer[2] = reg_data;		/* Data */

	I2CEngine();

	/* Operation not OK ? */
	if (I2C_OK != I2CMasterState)
	{
		/* other return error */
		return PH_ERR_INTERFACE_ERROR;
	}

	/* return success */
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
phStatus_t RegCtl_I2cHwModReg(uint8_t address, uint8_t mask, uint8_t set)
{
    uint8_t  reg_data;

    /* get the register first */
    RegCtl_I2cHwGetReg(address, &reg_data);

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
	RegCtl_I2cHwSetReg(address, reg_data);

    return PH_ERR_SUCCESS;
}

