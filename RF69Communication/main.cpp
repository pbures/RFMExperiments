 /*
 RF69Communication.cpp

 License
 **********************************************************************************
 This program is free software; you can redistribute it
 and/or modify it under the terms of the GNU General
 Public License as published by the Free Software
 Foundation; either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will
 be useful, but WITHOUT ANY WARRANTY; without even the
 implied warranty of MERCHANTABILITY or FITNESS FOR A
 PARTICULAR PURPOSE. See the GNU General Public
 License for more details.

 Licence can be viewed at
 http://www.gnu.org/licenses/gpl-3.0.txt

 Please maintain this license information along with authorship
 and copyright notices in any redistribution of this code
 **********************************************************************************
 */
 
 #define F_CPU 1000000L

 #include <inttypes.h>
 #include <avr/io.h>
 #include <avr/pgmspace.h>
 #include <stdio.h>
 #include <util/delay.h>
 #include <string.h>
 #include "RFM69.h"
 #include "SPI.h"
 #include "uart.h"
 #include "TimerClass.h"
 #include "pindefs.h"
 #include <DHT22.h>

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
	  
	 DHT myDht(&DHT22_DDR, &DHT22_PORT, &DHT22_PIN, DHT22_BIT);
	 myDht.begin();
	 
	 
	 while(true){
		 float temp = myDht.readTemperature(false,true);
		 float humidity = myDht.readHumidity();
		 
		 printf("Temp = %f   Humidity = %f\r\n", (double)temp, (double)humidity);
		 _delay_ms(500);
	 }
	 
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
	 
	 #ifdef TRANSCIEVER
	 char buffer[32];
	 uint32_t count=0;
	 while(true) {
		 snprintf(buffer,32,"START[%lu]", (unsigned long)count);
		 printf("\r\n...Sending commad with ACK, attemt: %lu\r\n", (unsigned long) count++);
		 bool sent = radio.sendWithRetry(RFM_RECEIVER_DEVICE_ID, buffer, strlen(buffer)+1);
		 printf_P(PSTR("...Command sent: %s\r\n"),
		 sent ? " successfully" : " unsuccessfully");
		 _delay_ms(2000);
		 
		 for(uint8_t i=0; i<10; i++){
			 SET_HIGH(LED_PORT, LED_BIT);
			 _delay_ms(30);
			 SET_LOW(LED_PORT, LED_BIT);
			 _delay_ms(30);
		 }
	 }
	 #endif
	 
	 #ifdef RECEIVER
	 radio.promiscuous(true);
	 uint32_t packetCount = 0;
	 uint8_t ackCount = 0;
	 
	 while(true){
		 printf("Checking if radio received signal...\r\n");
		 if (radio.receiveDone())
		 {
			 printf("#[%lu]", (unsigned long)packetCount);
			 
			 printf("[%iu]", radio.SENDERID);
			 printf(" to [%iu]", radio.TARGETID);
			 
			 for (uint8_t i = 0; i < radio.DATALEN; i++)
			 printf("%c",(char)radio.DATA[i]);
			 printf("   [RX_RSSI: %d]", radio.RSSI);
			 
			 if (radio.ACKRequested())
			 {
				 uint8_t theNodeID = radio.SENDERID;
				 radio.sendACK();
				 printf_P(PSTR(" - ACK sent."));

				 // When a node requests an ACK, respond to the ACK
				 // and also send a packet requesting an ACK (every 3rd one only)
				 // This way both TX/RX NODE functions are tested on 1 end at the GATEWAY
				 if (ackCount++%3==0)
				 {
					 printf(" Pinging node %d - ACK...", theNodeID);
					 _delay_ms(3000);
					 if (radio.sendWithRetry(theNodeID, "ACK TEST", 8, 0))  // 0 = only 1 attempt, no retries
					 printf_P(PSTR("ok!"));
					 else printf_P(PSTR("nothing"));
				 }
			 }
			 printf("\r\n");
			 
		 }
		 SET_LOW(LED_PORT, LED_BIT);
		 _delay_ms(1500);
		 SET_HIGH(LED_PORT, LED_BIT);
	 }
	 #endif
	 
	 #ifndef TRANSCIEVER
	 #ifndef RECEIVER
	 #error "TRANSCIEVER or RECEIVER must be defined";
	 #endif
	 #endif
 }

