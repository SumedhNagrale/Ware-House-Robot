#ifndef LCD_MoonDust

#define LCD_MoonDust
#endif 
#include <avr/io.h>
#include <math.h>
#include <util/delay.h>

#define F_CPU 16000000
//*********************************Code for LCD****************************************************
//start condition transmitted
#define TW_START 0x08
//repeated start condition transmitted
#define TW_REP_START 0x10
//SLA+W transmitted, ACK received
#define TW_MT_SLA_ACK 0x18

//define the codes for TWI actions
#define TWCR_START 0xA4 //send start condition
#define TWCR_STOP 0x94 //send stop condition
#define TWCR_RACK 0xC4 //receive byte and return ack to slave
#define TWCR_RNACK 0x84 //receive byte and return nack to slave
#define TWCR_SEND 0x84 //pokes the TWINT flag in TWCR and TWEN


//LCD Signals
#define LCD_ADDRESS 0x4E
//LCD address or the address of the device which is communicating with the arduino can be found out from the datasheet of the Device.
//I2C requires this in order to connect multiple devices at a time
#define LCD_DISABLE 0x04
#define LCD_ENABLE 0x00
#define LCD_WRITE 0x00
#define LCD_READ 0x01
#define LCD_RS 0x01
#define LCD_BL 0x08

//LCD Commands
#define CLEAR_DISPLAY 0x01
#define CURSOR_HOME 0x02
#define SET_ADDRESS 0x80

void InitializeLCD()
{
	
	//************************************CODE FOR LCD*********************************************
	//Initialize TWI
	TWBR = 0x30; //Setup prescaler
	TWCR |= (1<<TWEN); //Enable TWI
	
		//Initialize LCD
		_delay_us(250*64);//Wait for at least 15ms (15ms/64us = 234.3750)
		TWI(LCD_ADDRESS,0x30|LCD_DISABLE|LCD_WRITE|LCD_BL); //Function set (Interface is 8 bits long)
		TWI(LCD_ADDRESS,0x30|LCD_ENABLE|LCD_WRITE|LCD_BL);
		_delay_us(65*64);//Wait for 4.1ms (4.1ms/64us = 64.0625)
		TWI(LCD_ADDRESS,0x30|LCD_DISABLE|LCD_WRITE|LCD_BL); //Function set (Interface is 8 bits long)
		TWI(LCD_ADDRESS,0x30|LCD_ENABLE|LCD_WRITE|LCD_BL);
		_delay_us(2*64);//Wait for 100us (100us/64us = 1.5625)
		TWI(LCD_ADDRESS,0x30|LCD_DISABLE|LCD_WRITE|LCD_BL); //Function set (Interface is 8 bits long)
		TWI(LCD_ADDRESS,0x30|LCD_ENABLE|LCD_WRITE|LCD_BL);
		_delay_us(65*64);//Wait for 4.1ms (4.1ms/64us = 64.0625)
		TWI(LCD_ADDRESS,0x20|LCD_DISABLE|LCD_WRITE|LCD_BL); //Function set (Interface is 4 bits long)
		TWI(LCD_ADDRESS,0x20|LCD_ENABLE|LCD_WRITE|LCD_BL);
		//Setup for 2 lines
		TWI(LCD_ADDRESS,0x20|LCD_DISABLE|LCD_WRITE|LCD_BL); //Function set (Interface is 4 bits long)
		TWI(LCD_ADDRESS,0x20|LCD_ENABLE|LCD_WRITE|LCD_BL);
		TWI(LCD_ADDRESS,0x80|LCD_DISABLE|LCD_WRITE|LCD_BL); //Function set (Interface is 2 lines)
		TWI(LCD_ADDRESS,0x80|LCD_ENABLE|LCD_WRITE|LCD_BL);
		//Display no cursor
		LCD_Command(0x0C);
		//Automatic Increment
		LCD_Command(0x06);
		//Clear Display
		LCD_Command(CLEAR_DISPLAY);
		_delay_us(200*64);
}

void TWI(unsigned char address, unsigned char data)
{
	TWCR = TWCR_START;  //send start condition
	while(!(TWCR & (1<<TWINT))){} //wait for start condition to transmit
	TWDR = address;  //send address to get device attention
	TWCR = TWCR_SEND;  //Set TWINT to send address
	while(!(TWCR & (1<<TWINT))){} //wait for address to go out
	TWDR = data;	//send data to address
	TWCR = TWCR_SEND; //Set TWINT to send address
	while(!(TWCR & (1<<TWINT))){} //wait for data byte to transmit
	TWCR = TWCR_STOP;//finish transaction
}

void LCD_Command(unsigned char command)
{
	//Setup command high-nibble, with E high
	TWI(LCD_ADDRESS,(command &0xF0)|LCD_DISABLE|LCD_WRITE|LCD_BL);
	//Pull E low
	TWI(LCD_ADDRESS,(command &0xF0)|LCD_ENABLE|LCD_WRITE|LCD_BL);
	//Setup command low-nibble, with E high
	TWI(LCD_ADDRESS,((command<<4) & 0xF0)|LCD_DISABLE|LCD_WRITE|LCD_BL);
	//Pull E low
	TWI(LCD_ADDRESS,((command<<4) & 0xF0)|LCD_ENABLE|LCD_WRITE|LCD_BL);
}

void LCD_Display(unsigned char data)
{
	//Setup character data, with E high
	TWI(LCD_ADDRESS,(data & 0xF0)|LCD_DISABLE|LCD_WRITE|LCD_BL|LCD_RS);
	//Pull E low
	TWI(LCD_ADDRESS,(data & 0xF0)|LCD_ENABLE|LCD_WRITE|LCD_BL|LCD_RS);
	//Setup character data low nibble, with E high
	TWI(LCD_ADDRESS,((data<<4) & 0xF0)|LCD_DISABLE|LCD_WRITE|LCD_BL|LCD_RS);
	//Pule E low
	TWI(LCD_ADDRESS,((data<<4) & 0xF0)|LCD_ENABLE|LCD_WRITE|LCD_BL|LCD_RS);
}
