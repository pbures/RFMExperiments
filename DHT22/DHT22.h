/* DHT library
MIT license
written by Adafruit Industries
*/
#ifndef DHT22_H
#define DHT22_H

#include <stdio.h>
#include <inttypes.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <TimerClass.h>

// Uncomment to enable printing out nice debug messages.
//#define DHT_DEBUG

// Define where debug output will be printed.
#define LOW false
#define HIGH true

class DHT {
	public:
	DHT(volatile uint8_t *ddr, volatile uint8_t *port, volatile uint8_t *pin, uint8_t bit);
	void begin(void);
	float readTemperature(bool S=false, bool force=false);
	float C2F(float);
	float F2C(float);
	//float computeHeatIndex(float temperature, float percentHumidity, bool isFahrenheit=true);
	float readHumidity(bool force=false);
	bool read(bool force=false);

	private:
	uint8_t bytes[5];
	volatile uint8_t *_ddr, *_port, *_pin;
	uint8_t _bit;
	volatile uint32_t _lastreadtime;
	bool _lastresult;
	
	 void setLow() {
		 (*_port) &= ~(1<<_bit);
		 };
		 
	 void setHigh() {
		 (*_port) |= (1<<_bit);
		 };
		
	void setOutputMode() {(*_ddr) |= (1<<_bit);};
	void setInputMode() {(*_ddr) &= ~(1<<_bit);};
};

class InterruptLock {
	public:
	InterruptLock() {
		cli();
	}
	~InterruptLock() {
		sei();
	}
};

#endif //DHT22_H