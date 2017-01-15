/*
 * SPIEEPROM.cpp
 *
 *  Created on: 10. 12. 2016
 *      Author: pbures
 */

#include "SPIEEPROM.h"

#include <stdio.h>

SPIEEPROM::SPIEEPROM() {
	init();
}

void SPIEEPROM::init() {
	SET_OUTPUT_MODE(SPI_SS_DDR, SPI_SS_BIT);
	SLAVE_DESELECT;
	SPI.begin();
}

void SPIEEPROM::end() {
	SLAVE_DESELECT;
	SPI.end();
}

uint8_t SPIEEPROM::readByte(uint16_t address) {
	uint8_t ret = 0;
	SLAVE_SELECT;
	SPI.transfer(EEPROM_READ);
	SPI.transfer16(address);
	ret = SPI.transfer(0);
	SLAVE_DESELECT;
	return ret;
}

uint8_t SPIEEPROM::readStatus(void) {
	uint8_t ret = 0;
	SLAVE_SELECT;
	SPI.transfer(EEPROM_RDSR);
	ret = SPI.transfer(0);
	SLAVE_DESELECT;
	return ret;
}

uint16_t SPIEEPROM::readWord(uint16_t address) {
	uint16_t word;

	SLAVE_SELECT;
	SPI.transfer(EEPROM_READ);
	SPI.transfer16(address);
	word = SPI.transfer16(0);
	SLAVE_DESELECT;

	return word;
}

uint16_t SPIEEPROM::readString(uint16_t address, uint8_t* buf,
		uint16_t buflen) {
	uint16_t p = 0;
	uint8_t c = 0;

	do {
		SLAVE_SELECT;
		SPI.transfer(EEPROM_READ);
		SPI.transfer16(address + p);
		c = SPI.transfer(0);
		SLAVE_DESELECT;
		buf[p] = c;
		p++;
	} while ((c != 0) && (p < buflen));

	return p;
}

void SPIEEPROM::writeByte(uint16_t address, uint8_t byte) {
	writeEnable();

	SLAVE_SELECT;
	SPI.transfer(EEPROM_WRITE);
	SPI.transfer16(address);
	SPI.transfer(byte);
	SLAVE_DESELECT;

	while (readStatus() & _BV(EEPROM_WRITE_IN_PROGRESS)) {
		;
	}
}

#ifdef NEVERDEF
void SPIEEPROM::writeString(uint16_t address, uint8_t* string) {
	writeEnable();

	SLAVE_SELECT;
	SPI.transfer(EEPROM_WRITE);
	SPI.transfer16(address);

	uint8_t p = 0;
	uint8_t c = 0;

	while ((c = *(string + p)) != 0) {
		SPI.transfer(c);
		p++;
	}
	SPI.transfer(c);

	SLAVE_DESELECT;
	while (readStatus() & _BV(EEPROM_WRITE_IN_PROGRESS)) {
		;
	}
}
#endif

void SPIEEPROM::writeString(uint16_t address, uint8_t* string) {
	writeEnable();

	uint8_t p = 0;
	uint8_t c = *(string + p);
	uint16_t addr = address;
	uint8_t stop=0;

	SLAVE_SELECT;
	SPI.transfer(EEPROM_WRITE);
	SPI.transfer16(address);

	do {
		SPI.transfer(c);
		if (c == 0) stop = 1;

		p++;
		addr++;
		c = *(string + p);

		if ( ((addr % EEPROM_BYTES_PER_PAGE) == 0) || (stop)) {
			SLAVE_DESELECT;
			while (readStatus() & _BV(EEPROM_WRITE_IN_PROGRESS)) {;}

			if ((!stop) && (addr < EEPROM_BYTES_MAX)) {
				writeEnable();
				SLAVE_SELECT;
				SPI.transfer(EEPROM_WRITE);
				SPI.transfer16(addr);
			}
		}

	} while ((!stop) && (addr < EEPROM_BYTES_MAX));
}

void SPIEEPROM::writeWord(uint16_t address, uint16_t word) {
	writeEnable();

	SLAVE_SELECT;
	SPI.transfer(EEPROM_WRITE);
	SPI.transfer16(address);
	SPI.transfer((uint8_t) (word >> 8));
	SPI.transfer((uint8_t) word);
	SLAVE_DESELECT;

	while (readStatus() & _BV(EEPROM_WRITE_IN_PROGRESS)) {
		;
	}
}

void SPIEEPROM::clearAll(void) {
	uint8_t i;
	uint16_t pageAddress = 0;
	while (pageAddress < EEPROM_BYTES_MAX) {
		writeEnable();
		SLAVE_SELECT;
		SPI.transfer(EEPROM_WRITE);
		SPI.transfer16(pageAddress);
		for (i = 0; i < EEPROM_BYTES_PER_PAGE; i++) {
			SPI.transfer(0);
		}
		SLAVE_DESELECT;
		pageAddress += EEPROM_BYTES_PER_PAGE;
		while (readStatus() & _BV(EEPROM_WRITE_IN_PROGRESS)) {
			;
		}
	}
}

void SPIEEPROM::writeEnable(void) {
	SLAVE_SELECT;
	SPI.transfer(EEPROM_WREN);
	SLAVE_DESELECT;
}
