/* DHT library
MIT license
written by Adafruit Industries
*/

#include "DHT22.h"

#define MIN_INTERVAL 2000


DHT::DHT(volatile uint8_t *ddr, volatile uint8_t *port, volatile uint8_t *pin, uint8_t bit) :
_ddr(ddr), _port(port), _pin(pin), _bit(bit)
{
}

void DHT::begin() {
	
	DDRD |= (1<<PORTD7);
	PORTD &= ~(1<<PORTD7);
	
	setInputMode();
	setHigh();
	
	// Using this value makes sure that millis() - lastreadtime will be
	// >= MIN_INTERVAL right away. Note that this assignment wraps around,
	// but so will the subtraction.
	_lastreadtime = -MIN_INTERVAL;
}

//boolean S == Scale.  True == Fahrenheit; False == Celcius
float DHT::readTemperature(bool S, bool force) {
	float f = NAN;

	if (read(force)) {
		f = bytes[2] & 0x7F;
		f *= 256;
		f += bytes[3];
		f *= 0.1;
		if (bytes[2] & 0x80) {
			f *= -1;
		}
		if(S) {
			f = C2F(f);
		}
	}
	return f;
}

float DHT::C2F(float c) {
	return c * 1.8 + 32;
}

float DHT::F2C(float f) {
	return (f - 32) * 0.55555;
}

float DHT::readHumidity(bool force) {
	float f = NAN;
	if (read()) {
		f = bytes[0];
		f *= 256;
		f += bytes[1];
		f *= 0.1;
	}
	return f;
}

////boolean isFahrenheit: True == Fahrenheit; False == Celcius
//float DHT::computeHeatIndex(float temperature, float percentHumidity, bool isFahrenheit) {
//// Using both Rothfusz and Steadman's equations
//// http://www.wpc.ncep.noaa.gov/html/heatindex_equation.shtml
//float hi;
//
//if (!isFahrenheit)
//temperature = C2F(temperature);
//
//hi = 0.5 * (temperature + 61.0 + ((temperature - 68.0) * 1.2) + (percentHumidity * 0.094));
//
//if (hi > 79) {
//hi = -42.379 +
//2.04901523 * temperature +
//10.14333127 * percentHumidity +
//-0.22475541 * temperature*percentHumidity +
//-0.00683783 * pow(temperature, 2) +
//-0.05481717 * pow(percentHumidity, 2) +
//0.00122874 * pow(temperature, 2) * percentHumidity +
//0.00085282 * temperature*pow(percentHumidity, 2) +
//-0.00000199 * pow(temperature, 2) * pow(percentHumidity, 2);
//
//if((percentHumidity < 13) && (temperature >= 80.0) && (temperature <= 112.0))
//hi -= ((13.0 - percentHumidity) * 0.25) * sqrt((17.0 - abs(temperature - 95.0)) * 0.05882);
//
//else if((percentHumidity > 85.0) && (temperature >= 80.0) && (temperature <= 87.0))
//hi += ((percentHumidity - 85.0) * 0.1) * ((87.0 - temperature) * 0.2);
//}
//
//return isFahrenheit ? hi : F2C(hi);
//}

bool DHT::read(bool force) {
	// Check if sensor was read less than two seconds ago and return early to use last reading.
	uint32_t currenttime = Timer.millis();
	if (!force && ((currenttime - _lastreadtime) < 2000)) {
		return _lastresult; // return last correct measurement
	}
	_lastreadtime = currenttime;

	// Reset 40 bits of received data to zero.
	// https://cdn-shop.adafruit.com/datasheets/DHT22.pdf
	bytes[0] = bytes[1] = bytes[2] = bytes[3] = bytes[4] = 0;
	
	// Go into high impedence state to let pull-up raise data line level and start the reading process.
	setHigh();
	_delay_ms(250);

	// First set data line low for 20 milliseconds.
	setOutputMode();
	setLow();
	_delay_ms(20);

	volatile uint8_t cycles[80];
	uint16_t data[256];
	
	{
		InterruptLock lock;

		/* Set the line high for 40us, starting the measurement */
		setHigh();
		_delay_us(40);
		setInputMode();

		/* Start recording Timer1 values at changing values. Timer1 values tick in microseconds. */
		//TODO: So far I have no idea why I can't refernce the PINB using *_pin value;
		uint8_t myPinb = PINB;
		uint16_t myTcnt=TCNT1;
		TCNT1 = 0;

		/* This is extremely time critical, running on the edge. Do not make it even a bit more complex,
		* or the edges are not caught. Interrupt based handlinng does not work either.
		*/
		for(uint8_t c=0;c<84;c++){
			while(!(PINB ^ myPinb)&(0x1)) {;} //TODO: Here this may get stuck forever! Need to add some protection.
			myPinb = PINB;
			data[c] = TCNT1;
			//PORTD ^= (1<<PORTD7);
		}
		
		//TODO: We may have overflow!, check on that.
		TCNT1 += myTcnt;
	}
	
	for(uint8_t c=0; c<83;c++) {
		data[c] = (data[c+1] - data[c]);
	}
	#ifdef DHT_DEBUG
	printf("\r\nT:");
	for(uint8_t c=0; c<83;c++){
		printf(" %i",data[c]);
	}
	printf("\r\nB:      ");
	#endif
	
	// Inspect pulses and determine which ones are 0 (high state cycle count < low
	// state cycle count), or 1 (high state cycle count > low state cycle count).
	for (int i=0; i<40; ++i) {
		
		bytes[i/8] <<= 1;
		if (data[i*2 + 3] > data[i*2 + 2]) {
			// Ones are cycles are longer than 50us low gap. Zeros are shorter than the gap.
			bytes[i/8] |= 1;
			#ifdef DHT_DEBUG
			printf("     1");
			#endif
			} else {
			#ifdef DHT_DEBUG
			printf("     0");
			#endif
		}
	}
	#ifdef DHT_DEBUG
	printf("\r\nReceived: %x, %x, %x, %x. %x == %x ?\n\r",
	bytes[0], bytes[1], bytes[2], bytes[3], bytes[4],
	(bytes[0] + bytes[1] + bytes[2] + bytes[3]) & 0xFF);
	
	printf("\r\n");
	#endif

	// Check we read 40 bits and that the checksum matches.
	if (bytes[4] == ((bytes[0] + bytes[1] + bytes[2] + bytes[3]) & 0xFF)) {
		_lastresult = true;
		return _lastresult;
	}
	else {
		#ifdef DHT_DEBUG
		printf_P(PSTR("Checksum failure!"));
		#endif
		_lastresult = false;
		return _lastresult;
	}
}