/*
 * SPIEEPROM.h
 *
 *  Created on: 10. 12. 2016
 *      Author: pbures
 */

#ifndef SPIEEPROM_H_
#define SPIEEPROM_H_

#include <avr/io.h>
#include <util/delay.h>

#include "SPI.h"
#include "pindefs.h"

#define SLAVE_SELECT    SPI_SS_PORT &= ~(1 << SPI_SS_BIT)
#define SLAVE_DESELECT  SPI_SS_PORT |= (1 << SPI_SS_BIT)

// Instruction Set -- from data sheet
#define EEPROM_READ      0b00000011                     /* read memory */
#define EEPROM_WRITE     0b00000010                 /* write to memory */

#define EEPROM_WRDI      0b00000100                   /* write disable */
#define EEPROM_WREN      0b00000110                    /* write enable */

#define EEPROM_RDSR      0b00000101            /* read status register */
#define EEPROM_WRSR      0b00000001           /* write status register */

// EEPROM Status Register Bits -- from data sheet
// Use these to parse status register
#define EEPROM_WRITE_IN_PROGRESS    0
#define EEPROM_WRITE_ENABLE_LATCH   1
#define EEPROM_BLOCK_PROTECT_0      2
#define EEPROM_BLOCK_PROTECT_1      3

#define EEPROM_BYTES_PER_PAGE       64
#define EEPROM_BYTES_MAX            0x7FFF


class SPIEEPROM {
public:
	SPIEEPROM();
	void init();
	void end();

	uint8_t readByte(uint16_t address);
	uint16_t readWord(uint16_t address);
	uint16_t readString(uint16_t address, uint8_t* buf, uint16_t buflen);

	void writeByte(uint16_t address, uint8_t byte);
	void writeWord(uint16_t address, uint16_t word);
	void writeString(uint16_t address, uint8_t* string);

	uint8_t readStatus(void);
	void writeEnable(void);

	void clearAll(void);
};

#endif /* SPIEEPROM_H_ */
