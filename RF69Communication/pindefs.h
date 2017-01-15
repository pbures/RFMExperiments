#ifndef PINDEFS_H_
#define PINDEFS_H_

#define SET_INPUT_MODE(ddr,bit) ddr &= ~(1<<bit)
#define SET_OUTPUT_MODE(ddr,bit) ddr |= (1<<bit)

#define SET_HIGH(port,bit) port |= (1<<bit)
#define SET_LOW(port,bit) port &= ~(1<<bit)

#define BLINK(port,bit) SET_HIGH(port,bit);SET_LOW(port,bit);

#define SET_HIGH_REF(port,bit) *port |= (1<<bit)
#define SET_LOW_REF(port,bit) *port &= ~(1<<bit)

/* RF69 Definition */
#define RF69_SPI_CS_DDR    DDRB
#define RF69_SPI_CS_PORT   PORTB
#define RF69_SPI_CS_PIN    PINB
#define RF69_SPI_CS_BIT    PB2

#define RF69_DIO0_DDR  DDRD
#define RF69_DIO0_PORT PORTD
#define RF69_DIO0_PIN  PIND
#define RF69_DIO0_BIT  PD2

/* Pin the control LED */
#define LED_DDR 	DDRB
#define LED_PORT 	PORTB
#define LED_PIN 	PINB
#define LED_BIT 	PB1

#define SPI_SS_BIT  PB6
#define SPI_SS_PORT PORTB
#define SPI_SS_PIN  PINB
#define SPI_SS_DDR  DDRB

#define SPI_MOSI       PB3
#define SPI_MOSI_PORT  PORTB
#define SPI_MOSI_PIN   PINB
#define SPI_MOSI_DDR   DDRB

#define SPI_MISO       PB4
#define SPI_MISO_PORT  PORTB
#define SPI_MISO_PIN   PINB
#define SPI_MISO_DDR   DDRB

#define SPI_SCK        PB5
#define SPI_SCK_PORT   PORTB
#define SPI_SCK_PIN    PINB
#define SPI_SCK_DDR    DDRB

#define I2C_SDA        PC4
#define I2C_SDA_PORT   PORTC
#define I2C_SDA_PIN    PINC
#define I2C_SDA_DDR    DDRC

#define I2C_SCL        PC5
#define I2C_SCL_PORT   PORTC
#define I2C_SCL_PIN    PINC
#define I2C_SCL_DDR    DDRC

#endif /* PINDEFS_H_ */
