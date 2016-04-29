/*
 * RegCtl_I2cHw.h
 *
 *  Created on: 19 janv. 2011
 *      Author: frq04766
 */

#ifndef REGCTL_I2CHW_H_
#define REGCTL_I2CHW_H_

#include <ph_Status.h>

//#define I2C_USED
#define RC663_ADDR		0x50U

extern void RegCtl_I2cHwInit(void);
extern phStatus_t RegCtl_I2cHwGetReg(uint8_t address, uint8_t *reg_data);
extern phStatus_t RegCtl_I2cHwSetReg(uint8_t address, uint8_t reg_data);
extern phStatus_t RegCtl_I2cHwModReg(uint8_t address, uint8_t mask, uint8_t set);

#endif /* REGCTL_I2CHW_H_ */
