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
	 
	 for(uint8_t i=0; i<5; i++) {
		 SET_HIGH(LED_PORT, LED_BIT);
		 _delay_ms(500);
		 SET_LOW(LED_PORT, LED_BIT);
		 _delay_ms(500);
	 }
	 
	 DHT myDht(&DHT22_DDR, &DHT22_PORT, &DHT22_PIN, DHT22_BIT);
	 myDht.begin();
	 
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
	 //printf_P(PSTR("...Read all registers read\n\r"));
	 
	 radio.setHighPower(); //uncomment only for RFM69HW!
	 //radio.encrypt(ENCRYPTKEY);
	 //radio.enableAutoPower(ATC_RSSI);
	 //printf_P(PSTR("Auto power enabled.\r\n"));
	 //printf_P(PSTR("Sending START command.\r\n"));
	 
	 #ifdef TRANSCIEVER
	 char buffer[22];
	 
	 while(true) {

		 _delay_ms(2000);
		 float temperature = myDht.getTemperature();
		 float humidity = myDht.getHumidity();

		 snprintf(buffer,22,"T[%+07.2f] H[%+07.2f]", (double)temperature, (double)humidity);
		 printf("%s", buffer);
		 
		 bool sent = radio.sendWithRetry(RFM_RECEIVER_DEVICE_ID, buffer, strlen(buffer)+1);
		 printf_P(PSTR("...%s\r\n"), sent ? " success" : " failure");
		 
		 for(uint8_t i=0; i<15; i++){
			 //SET_HIGH(LED_PORT, LED_BIT);
			 _delay_ms(30);
			 //SET_LOW(LED_PORT, LED_BIT);
			 _delay_ms(30);
		 }
	 }
	 #endif
	 
	 #ifdef RECEIVER
	 radio.promiscuous(false);
	 uint32_t packetCount = 0;
	 uint8_t ackCount = 0;
	 
     printf("Checking if radio received signal...\r\n");
	 while(true){
		 if (radio.receiveDone())
		 {
			 
			 printf("[sender:%i]", radio.SENDERID);
			 printf(" to [%i/(me:%i)l:%i]", radio.TARGETID, RFM_RECEIVER_DEVICE_ID, radio.DATALEN);
			 
			 for (uint8_t i = 0; i < radio.DATALEN; i++)
			 printf("%c",(char)radio.DATA[i]);
			 printf("[RX_RSSI: %d]", radio.RSSI);
			 
			 if (radio.ACKRequested())
			 {
				 uint8_t theNodeID = radio.SENDERID;
				 radio.sendACK();
				 printf_P(PSTR(" ACK"));

				 // When a node requests an ACK, respond to the ACK
				 // and also send a packet requesting an ACK (every 3rd one only)
				 // This way both TX/RX NODE functions are tested on 1 end at the GATEWAY
				 //if (ackCount++%3==0)
				 //{
					 //printf(" Pinging node %d - ACK...", theNodeID);
					 //_delay_ms(3000);
					 //if (radio.sendWithRetry(theNodeID, "ACK TEST", 8, 0))  // 0 = only 1 attempt, no retries
					 //printf_P(PSTR("ok!"));
					 //else printf_P(PSTR("nothing"));
				 //}
			 }
			 printf("\r\n");
			_delay_ms(500);
		 }
	 }
	 #endif
	 
	 #ifndef TRANSCIEVER
	 #ifndef RECEIVER
	 #error "TRANSCIEVER or RECEIVER must be defined";
	 #endif
	 #endif
 }

