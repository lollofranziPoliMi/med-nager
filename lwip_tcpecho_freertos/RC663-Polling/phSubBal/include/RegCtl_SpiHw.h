/*
 * RegCtl_I2cHw.h
 *
 *  Created on: 19 janv. 2011
 *      Author: frq04766
 */

#ifndef REGCTL_SPIHW_H_
#define REGCTL_SPIHW_H_

#include <ph_Status.h>

#define SPI_USED

void RegCtl_SpiHwInit(void);
phStatus_t RegCtl_SpiHwGetReg(uint8_t address, uint8_t *reg_data);
phStatus_t RegCtl_SpiHwSetReg(uint8_t address, uint8_t reg_data);
phStatus_t RegCtl_SpiHwModReg(uint8_t address, uint8_t mask, uint8_t set);
//extern phStatus_t RegCtl_SpiHwSetMultiData(uint8_t *buf, uint32_t Len);
//extern phStatus_t RegCtl_SpiHwGetMultiData(uint8_t *txbuf, uint8_t *rxbuf, uint32_t Len);

#endif /* REGCTL_SPIHW_H_ */
