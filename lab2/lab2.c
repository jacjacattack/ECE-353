// Matthew Bolognese
// Jacqueline Lagasse
// ECE 353 Project 2
// Group 20B


////////// INCLUDE STATEMENTS //////////
#include <avr/io.h>    // Standard AVR header 
#include <avr/iom32.h> // Standard AVR header for ATmega32
#include <avr/eeprom.h> // eeprom library
#include <util/delay.h> // delay library
#include <avr/interrupt.h> // interrupt library


////////// GLOBAL VARIABLES //////////
#define baud 31250; // note the baud rate, used behind the scenes byt eh processor
volatile uint8_t overflow_counter = 0; // initialize and set overflow counter to zero


////////// USART FUNCTIONS //////////
void USART_Transmit(unsigned char data){ // USART transmit function used to playback
	while( !(UCSRA & (1<<UDRE)) ) // wait to receive data to transmit
		;
	UDR = data; // stores data in temporary buffer
}


unsigned char USART_Receive(void){ // USART receive function, used to record
	while( !(UCSRA & (1<<RXC)) && PINA & (1 << 5)) // wait to receive data AND still have switch on
		;
	return UDR; // returns data from temporary buffer
}


////////// EEPROM FUNCTIONS //////////
void EEPROM_write(unsigned int uiAddress, unsigned char ucData){ // writes to the EEPROM memory
	while(EECR & (1<<EEWE)) // wait for completion of previous write
		;
	EEAR = uiAddress;	// set up address register
	EEDR = ucData;		// set up data register
	EECR |= (1<<EEMWE);	// set EEMWE to high
	EECR |= (1<<EEWE);	// this starts EEPROM writing
	PORTB = ucData;		// prints note data to LED
}


unsigned char EEPROM_read(unsigned int uiAddress){ // reads from the EEPROM memory
	while(EECR & (1<<EEWE)) // wait for completion of previous write
		;
	EEAR = uiAddress;	// set up address register
	EECR |= (1<<EERE);	// start eeprom read
	PORTB = EEDR;		// prints note data to LED
	return EEDR;		// return data from data register
}


////////// TIMING AND COUNTERS //////////
void init_timing(){ // initialize timer and prescaler
	TIMSK = (1<<TOIE1); // timer set to interupt when overflow
	TCNT1=0;			// set timer1 to zero to initialize
	TCCR1B = (1<<CS10); // turn on timer no prescaler
	sei(); // global variable required to start clock
}


ISR(TIMER1_OVF_vect){ // interrupt service routine
	overflow_counter++; // increment overflow counter
}


////////// OTHER //////////
void init(){ // initializes values needed at the beginning of main method
	DDRA = 0x00;	// set Port A direction as input (switches / modify)
	DDRB = 0xFF;    // set Port B direction as output (LED lights)
	// initialize USART and Baud rate
	UBRRL = 7; // (4M/(16*baud))-1 = 7
	UBRRH = 0; // set to zero
	UCSRB = (1<<RXEN)|(1<<TXEN); // enable receiver and transmitter
	UCSRC = (1<<URSEL)|(1<<UCSZ0)|(1<<UCSZ1); // set frame, 8 data, 2 stop
}


////////// MODIFY //////////
unsigned int ReadADC1(void){ //changes the playback speed  
	ADMUX=0b10100000; // selects ADC channel to PA0 (ADC0)
	ADMUX = (1<<REFS0); // set mux
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS0); // divided by a prescale of 32
	ADCSRA|=(1<<ADSC); // clear ADSC by writing one to it
	while(!(ADCSRA & (1<<ADSC))) // wait for conversion to complete
		;
	return(ADC); // returns a 10-bit unsigned number
}


unsigned int ReadADC2(void){ // 
	ADMUX=0b10100001;	// selects ADC channel to PA1 (ADC1)
	ADMUX = (1<<REFS0); // set mux
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS0); // divided by a prescale of 32
	ADCSRA|=(1<<ADSC); // clear ADSC by writing one to it
	while(!(ADCSRA&(1<<ADSC))) // wait for conversion to complete
		;
	return(ADC); // returns a 10-bit unsigned number
}


void USART_FLUSH(void){
	unsigned char dummy;
	while(UCSRA & (1<<RXC)) dummy = UDR;
}


////////// MAIN METHOD //////////
int main(void) { // starts main method


	init();			// run initializing code
	init_timing(); 	// run timing initializing code


	uint8_t EEPROM_WRITE_ADDRESS = 0; // initialize and set EEPROM write adress to zero
	uint8_t EEPROM_READ_ADDRESS = 0;  // initialize and set EEPROM read adress to zero


	unsigned int modify_note = 0; // this is the value that the note will be modified by if modify switch is on
	unsigned int modify_speed = 0; // this is the value that the speed will be modified by if modify switch is on


	uint8_t timing_array[1024];	  // make timing array


	uint8_t ovf1; // overflow time 1
	uint8_t ovf2; // overflow time 2


	while(1){ // while loop looks continues indefinitely


		if(PINA & (1 << 5)){ // Record is on

			EEPROM_WRITE_ADDRESS = 0; // set to write from the beginning of memory

			while( (PINA & (1 << 5)) && EEPROM_WRITE_ADDRESS<1024 ){ // while pin 5 is on and there is room in EEPROM memory
			
				USART_FLUSH(); // flush USART to prevent errors

				ovf1=overflow_counter; // count overflow here

				USART_Receive(); // receive start character, skip
				EEPROM_write(EEPROM_WRITE_ADDRESS, (USART_Receive())); // receive note character, store
				USART_Receive(); // receive velocity character, skip

				USART_Receive(); // receive terminal start character, skip

				ovf2=overflow_counter; // count overflow here

				USART_Receive(); // receive terminal note character, skip
				USART_Receive(); // receive terminal velocity character, skip

				timing_array[EEPROM_WRITE_ADDRESS]=(ovf2-ovf1); // store differnce in overflow in array, array index matches EEPROM memory index

				EEPROM_WRITE_ADDRESS++; // increment write address

			} // end of while loop 

		} // end of Record


		else if(PINA & (1 << 6)){ // Playback is on

			EEPROM_READ_ADDRESS=0; // set to read from the beginning of memory

			while( (PINA & (1 << 6)) && EEPROM_READ_ADDRESS<(EEPROM_WRITE_ADDRESS-1) ){ // while pin 6 is on and we have not reached the end of written memory
				
				USART_FLUSH(); // flush USART to prevent errors

				modify_speed = 0; // modify value is zero unless modify is on
				modify_note = 0; // modify value is zero unless modify is on

				if(PINA & (1 << 7)){ // modify is on, change values 
					modify_speed = ReadADC1(); // represents aanalog voltage from photoresistor 1
					modify_note = ReadADC2(); // represents aanalog voltage from photoresistor 2
				}

				USART_Transmit(144); // receive start character, skip
				USART_Transmit( (EEPROM_read(EEPROM_READ_ADDRESS) + modify_note) % 128 ); // receive note character, access plus modify if activated

				for(uint8_t i=0; i<( timing_array[EEPROM_READ_ADDRESS] ); i++){ // delays 16ms for each overflow plus modify if activated
					_delay_ms(16.384 + (double)(modify_speed % 64) ); // 16ms is (2^16)/(4MHz), estimated time elapsed for each overflow	
				}

				USART_Transmit(100); // receive velocity character, skip

				// NOTE in the Lab Report logic analyzer printouts, we had delay set to 25ms
				// we realized before submission that we could get the same effect with a 10ms delay
				_delay_ms(10); // additional nominal delay to give computer enough time to process signal and play back the music audibly
				
				USART_Transmit(128); // receive terminal start character, skip
				USART_Transmit( (EEPROM_read(EEPROM_READ_ADDRESS) + modify_note) % 128 ); // receive terminal note character, skip, modify if activated
				USART_Transmit(64); // receive terminal velocity character, skip
				
				EEPROM_READ_ADDRESS++; // increment read address
				
			} // end of while loop


			while(PINA & (1 << 6)){ // stalls here until playback switch is turned off, prevents playback from looping infinitely
				;
			} // end of while loop


		} // end of Playback 


	} // end of while(1) loop


} // end of main



