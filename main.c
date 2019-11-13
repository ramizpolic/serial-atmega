/*
 *  Serial communication with Atmega328p
 *	Supports UART, Interrupts, EEPROM RW
 *  Extending to support Flash RW
 *	Datasheet: https://www.sparkfun.com/datasheets/Components/SMD/ATMega328.pdf
 *  
 *  Author: Ramiz Polic
 */ 


// Predefined constants
#define F_CPU			16000000
#define USART_BAUDRATE	9600
#define UBRR_VALUE		(((F_CPU/(USART_BAUDRATE*16UL)))-1)
#define RX_BUFFER_SIZE	512
#define RX_LINE_SIZE	128

#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

// Globals
unsigned char rx_buffer[RX_BUFFER_SIZE], rx_line[RX_LINE_SIZE];
unsigned char rx_buffer_pos=0, rx_line_pos=0;
short waiting = 1;

/************************************************************************/
/* Init module                                                          */
/************************************************************************/
void serial_init(){
	// Initialize USART
	UBRR0=UBRR_VALUE;				  // set baud rate
	UCSR0B|=(1<<TXEN0);				  // enable TX
	UCSR0B|=(1<<RXEN0);				  // enable RX
	UCSR0B|=(1<<RXCIE0);			  // RX complete interrupt
	UCSR0C|=(1<<UCSZ01)|(1<<UCSZ01);  // no parity, 1 stop bit, 8-bit data
}

/************************************************************************/
/* Adding transmit modules                                              */
/************************************************************************/
void serial_char(unsigned char data){
	while(!(UCSR0A & (1<<UDRE0)));
	UDR0 = data;
}

void serial_break(){
	serial_char(10); // LF 
	serial_char(13); // CR
}

void serial_string(char* s) {
	while (*s != '\0') {
		serial_char(*s);
		s++;
	}
}

/************************************************************************/
/* Interrupt Service Routine Receiver Handler							*/
/************************************************************************/
ISR(USART_RX_vect)
{
	// Empty buffer
	if(waiting == 0)
		memset(rx_line, 0, RX_LINE_SIZE);
	
	// Block
	waiting = 1;
	
	// Receive data
	unsigned char input = UDR0;
	if(input != '\n')
		rx_line[rx_line_pos++] = input;
	
	// Handle interrupt 
	if(rx_line_pos >= RX_LINE_SIZE || (input == '\n' && rx_line_pos > 0)) {				
		// Handle result
		logic_handler();
		
		// Resize
		rx_line_pos = 0;
		
		// Unblock
		waiting = 0;
	}
}

/************************************************************************/
/* Read/Write EEPROM												*/
/************************************************************************/
void save(char* data, int len) {
	eeprom_write_block((const void *)data, (void *)0, len);
	serial_string("Data saved to EEPROM.");
	serial_string("\n----- DATA -----\n");
	serial_string(data);
	serial_string("\n----------------\n");
}

void load(char* dest, int len) {
	eeprom_read_block((void *)dest , (const void *)0 , RX_BUFFER_SIZE);
	serial_string("Data loaded from EEPROM.");
	serial_string("\n----- DATA -----\n");
	serial_string(dest);
	serial_string("\n----------------\n");
}

/************************************************************************/
/* Logic handler                                                        */
/************************************************************************/
void logic_handler() {
	// Check commands
	if(strcmp(rx_line, "save") == 0) {
		save(rx_buffer, RX_BUFFER_SIZE);
	}
	else if(strcmp(rx_line, "load") == 0) {
		// Do something with load
		char data[RX_BUFFER_SIZE];
		load(data, RX_BUFFER_SIZE);
	}	
	else if(strcmp(rx_line, "all") == 0) {
		// Print all results so far
		serial_string("all results: \n");
		serial_string(rx_buffer);
		serial_break();
	}
	else if(rx_line[0] != '\0') {
		// Preview current
		serial_string("input: ");
		serial_string(rx_line);
		serial_break();
		
		// Append to buffer
		short int i = 0;
		while(rx_buffer_pos+1 < RX_BUFFER_SIZE && i < rx_line_pos)
			rx_buffer[rx_buffer_pos++] = rx_line[i++];
		rx_buffer[rx_buffer_pos++] = '\n';
	
		// Handle overflow
		if(rx_buffer_pos >= RX_BUFFER_SIZE)
		{
			rx_buffer_pos = 0;
			memset(rx_buffer, 0, RX_BUFFER_SIZE);
		}
	}
}

int main(void){
	// Initialize and enable interrupts
	serial_init();
	sei();
	
	// Preview commands
	serial_string("Commands: \n 'save' - save all results sent via UART to EEPROM\n 'load' - load saved results from EEPROM\n");
	serial_string(" 'all' - show buffer data \n 'x' - send data");
	serial_break();
	
	// Loop until dead
	for(;;){		
		// do nothing here
	}	
}