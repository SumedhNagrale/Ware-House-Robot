#ifndef UARTInit
#define UARTInit
#endif
#include <avr/io.h>
#include <math.h>
#define EVEN 0
#define ODD 1
unsigned char ReceiveUART0(void)
{
	while (!(UCSR0A & (1<<RXC0)));
	return UDR0;
}
void TransmitUART0(unsigned char data)
{
	//wait until the transmitter is ready
	while(!(UCSR0A & (1<<UDRE0)));
	//get the data out here
	UDR0 = data;
}
void InitializeUART0 ( int baud,char AsyncDoubleSpeed,char DataSizeInBits, char ParityEvenOrOdd, char StopBits)
{
	if(AsyncDoubleSpeed == 1) UCSR0A = (1 << U2X0); //setting the U2X bit to 1 for double speed asynchronous
	uint16_t UBBRValue =lrint((F_CPU / (16L * baud) ) - 1 );
	//Put the upper part of the baud number here (bits 8 to 11)
	UBRR0H = (unsigned char) (UBBRValue >> 8);
	//Put the remaining part of the baud number here
	UBRR0L  = (unsigned char) UBBRValue;
	//Enable the receiver and transmitter
	UCSR0B  = (1 << RXEN0) | (1 << TXEN0);
	//Set 2 stop bits
	UCSR0C = (1 << USBS0);
	if(ParityEvenOrOdd==EVEN) UCSR0C |= (1 << UPM01); //Sets parity to EVEN
	if(ParityEvenOrOdd==ODD) UCSR0C |= (3 << UPM00); //Alternative way to set parity to ODD
	if(DataSizeInBits ==6) UCSR0C |= (1 << UCSZ00); //6-bit data length
	if(DataSizeInBits ==7) UCSR0C |= (2 << UCSZ00); //7-bit data length
	if(DataSizeInBits ==8) UCSR0C |= (3 << UCSZ00); //8-bit data length
	if(DataSizeInBits ==9) UCSR0C |= (7 << UCSZ00);//9-bit data length
	if(StopBits == 2) UCSR0C |= (1 << USBS0); //Sets 2 stop bits
}
