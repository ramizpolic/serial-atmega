// Predefined constants
#define F_CPU			16000000
#define USART_BAUDRATE	9600
#define UBRR_VALUE		(((F_CPU/(USART_BAUDRATE*16UL)))-1)
#define RX_BUFFER_SIZE	1024
#define RX_LINE_SIZE	128

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// Globals
unsigned char rx_buffer[RX_BUFFER_SIZE], rx_line[RX_LINE_SIZE];
volatile unsigned char rx_read_pos=0, rx_line_pos=0;
volatile short waiting = 1;

/************************************************************************/
/* Init module                                                          */
/************************************************************************/
void serial_init(){
	// initialize USART
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
	// send a single char via USART, wait then transmit
	while(!(UCSR0A&(1<<UDRE0)));
	UDR0 = data;
}

void serial_break(){
	serial_char(10); // LF 
	serial_char(13); // CR
}

void serial_comma(){
	serial_char(',');
	serial_char(' ');
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
void wait_transmission(void) {
	while(waiting == 1);
}

ISR(USART_RX_vect)
{
	// empty line buffer
	if(waiting == 0) 
		memset(rx_line, 0, RX_LINE_SIZE);
	
	// block
	waiting = 1;
    
	// receive data
    rx_buffer[rx_read_pos] = UDR0;
    rx_line[rx_line_pos] = UDR0;
	rx_read_pos++;
	rx_line_pos++;
	
	// Handle interrupt 
	if(rx_line_pos >= RX_LINE_SIZE || UDR0 == '\n' || UDR0 == '\0') {
		rx_line_pos = 0;
			
		// unblock
		waiting = 0;
	}
	
	// Handle overflows
    if(rx_read_pos >= RX_BUFFER_SIZE)
    {
        rx_read_pos = 0;
        memset(rx_buffer, 0, RX_BUFFER_SIZE);
    }
}

/// Transmit result
void transmit_recieved(void) {
	if(rx_line[0] != '\0') {
		// Print current input
		serial_string("input: ");
		serial_string(rx_line);		
		serial_break();
		
		// Print all inputs so far
		// serial_string("buffer: ");
		// serial_string(rx_buffer);
		// serial_break();
	}
}

int main(void){
	// Initialize and enable interrupts
	serial_init();
	sei();
	
	// Loop until dead
	for(;;){
		// wait for input
		wait_transmission();
		
		// read and send input back
		transmit_recieved();
	}	
}