/**
 * @file arducam_arch_raspberrypi.h
 * @brief Raspberry Pi interface to the Arducam camera library porting \n
 * This project is part of the Nanodrone project
 * 
 * @author Enrico Miglino <balearicdynamics@gmail.com>
 * @date August 2020
 * @version 0.1
*/

#ifndef __ARDUCAM_ARCH_H__
#define __ARDUCAM_ARCH_H__

#include "ArduCAM.h"

// Interface to the original C library ported from Arduino to C++
#ifdef __cplusplus
extern "C" {
#endif
extern bool wiring_init(void);
extern bool arducam_i2c_init(uint8_t sensor_addr);
extern void arducam_spi_write(uint8_t address, uint8_t value);
extern uint8_t arducam_spi_read(uint8_t address);

extern uint8_t arducam_spi_transfer(uint8_t data);
extern void arducam_spi_transfers(uint8_t *buf, uint32_t size);

//! Delay execution for delay milliseconds
extern void arducam_delay_ms(uint32_t delay);

//! Read/write 8 bit value to/from 8 bit register address
extern uint8_t arducam_i2c_write(uint8_t regID, uint8_t regDat);
extern uint8_t arducam_i2c_read(uint8_t regID, uint8_t* regDat);

//! Read/write 16 bit value to/from 8 bit register address
extern uint8_t arducam_i2c_write16(uint8_t regID, uint16_t regDat);
extern uint8_t arducam_i2c_read16(uint8_t regID, uint16_t* regDat);

//! Read/write 8 bit value to/from 16 bit register address
extern uint8_t arducam_i2c_word_write(uint16_t regID, uint8_t regDat);
extern uint8_t arducam_i2c_word_read(uint16_t regID, uint8_t* regDat);

//! Write 8 bit values to 8 bit register address
extern int arducam_i2c_write_regs(const struct sensor_reg reglist[]);

//! Write 16 bit values to 8 bit register address
extern int arducam_i2c_write_regs16(const struct sensor_reg reglist[]);

//! Write 8 bit values to 16 bit register address
extern int arducam_i2c_write_word_regs(const struct sensor_reg reglist[]);

#ifdef __cplusplus
}
#endif

#endif 
