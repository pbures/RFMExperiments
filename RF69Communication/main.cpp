/*
* RF69Communication.cpp
*
* Created: 3.1.2017 18:49:02
* Author : pbures
*/
#define F_CPU 1000000L

#define TRANSCIEVER

#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <util/delay.h>
#include "RFM69.h"
#include "SPI.h"
#include "uart.h"
#include "TimerClass.h"

static int uart_putchar(char c, FILE *stream);
static FILE mystdout = {0};

// http://www.nongnu.org/avr-libc/user-manual/group__avr__stdio.html
static int uart_putchar(char c, FILE *stream) {
	txByte(c);
	return 0;
}

int main(void)
{
	SET_OUTPUT_MODE(LED_DDR, LED_BIT);
	SET_LOW(LED_PORT, LED_BIT);
	
	SET_OUTPUT_MODE(RF69_SPI_CS_DDR, RF69_SPI_CS_BIT);
	SET_HIGH(RF69_SPI_CS_PORT, RF69_SPI_CS_BIT);
	
	initUart();
	fdev_setup_stream(&mystdout, uart_putchar, NULL, _FDEV_SETUP_WRITE);
	stdout = &mystdout;
	
	#ifdef TRANSCIEVER
	printf_P(PSTR("\n\r===========Transciever started=========\n\r"));
	#else
	printf_P(PSTR("\n\r===========Receiver started=========\n\r"));
	#endif
	
	printf("...Initializing RFM.\r\n");

	sei();
	
	for(uint8_t i=0; i<5; i++) {
		SET_HIGH(LED_PORT, LED_BIT);
		_delay_ms(500);
		SET_LOW(LED_PORT, LED_BIT);
		_delay_ms(500);
	}
	
	RFM69 radio(/*isRFM69HW:*/true, RF69_IRQ_NUM);
	#ifdef TRANSCIEVER
	bool res = radio.initialize(RF69_868MHZ, RFM_TRANSCIEVER_DEVICE_ID, RFM_NETWORK_ID);
	#else
	bool res = radio.initialize(RF69_868MHZ, RFM_RECEIVER_DEVICE_ID, RFM_NETWORK_ID);
	#endif
	
	if (res) {
		printf("...RF69 Initialization successful.\r\n");
		} else {
		printf("...RF69 Failed.\r\n");
	}

	printf("\n\r...Reading all registers.\r\n");
	radio.readAllRegs();
	printf_P(PSTR("...Read all registers read\n\r"));
	
	radio.setHighPower(); //uncomment only for RFM69HW!
	//radio.encrypt(ENCRYPTKEY);
	//radio.enableAutoPower(ATC_RSSI);
	//printf_P(PSTR("Auto power enabled.\r\n"));
	//printf_P(PSTR("Sending START command.\r\n"));
	
	uint32_t count=0;
	
	
	#ifdef TRANSCIEVER
	while(true) {
		printf("\r\n...Sending commad with ACK, attemt: %lu\r\n", (unsigned long) count++);
		bool sent = radio.sendWithRetry(RFM_RECEIVER_DEVICE_ID, "START", 6);
		printf_P(PSTR("...Command sent: %s\r\n"),
		sent ? " successfully" : " unsuccessfully");
		_delay_ms(2000);
		
		//SET_HIGH(LED_PORT, LED_BIT);
		//_delay_ms(100);
		//SET_LOW(LED_PORT, LED_BIT);
		//
		//uint16_t count = 0;
		
		for(uint8_t i=0; i<10; i++){
			SET_HIGH(LED_PORT, LED_BIT);
			_delay_ms(100);
			SET_LOW(LED_PORT, LED_BIT);
			_delay_ms(100);
		}
	}
	#else
	uint32_t packetCount = 0;
	while(true){
		printf("Checking if radio received signal\r\n");
		  if (radio.receiveDone())
		  {
			  printf_P(PSTR("#[%ul]", (unsigned long)packetCount));
			  printf("[%d]", radio.SENDERID);
			  if (promiscuousMode)
				  printf(" to [%d]", radio.TARGEDID);
				  
			  for (byte i = 0; i < radio.DATALEN; i++)
			  Serial.print((char)radio.DATA[i]);
			  Serial.print("   [RX_RSSI:");Serial.print(radio.RSSI);Serial.print("]");
			  
			  if (radio.ACKRequested())
			  {
				  byte theNodeID = radio.SENDERID;
				  radio.sendACK();
				  Serial.print(" - ACK sent.");

				  // When a node requests an ACK, respond to the ACK
				  // and also send a packet requesting an ACK (every 3rd one only)
				  // This way both TX/RX NODE functions are tested on 1 end at the GATEWAY
				  if (ackCount++%3==0)
				  {
					  Serial.print(" Pinging node ");
					  Serial.print(theNodeID);
					  Serial.print(" - ACK...");
					  delay(3); //need this when sending right after reception .. ?
					  if (radio.sendWithRetry(theNodeID, "ACK TEST", 8, 0))  // 0 = only 1 attempt, no retries
					  Serial.print("ok!");
					  else Serial.print("nothing");
				  }
			  }
			  Serial.println();
			  Blink(LED,3);
		  }
	}
	#endif
}

