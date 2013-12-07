////////////////////////////////////////////////////////////
// TinyMatrix.c                                           //
// copyright 2012 Tim Toner (tigeruppp/at/gmail.com)      //
// licensed under the GNU GPL v2 or newer                 //
////////////////////////////////////////////////////////////

#include <inttypes.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <avr/pgmspace.h>


// 2D LED buffer
#define ROWS		5
#define COLS		7
char bitmap[ROWS][COLS];	
char need_refresh_line;
char need_render_frame;
unsigned char current_row;		// current lit row


// globals
int mode;						// current display mode
int t, frame, frame_delay;		// animation timing variables
int b1, last_b1, b2, last_b2;	// button states


// LED refresh interrupt, called 390 times per second
ISR(TIMER0_COMPA_vect)
{
	need_refresh_line = 1;	// flag to display new row of LEDs

	if (++current_row >= ROWS) 
		current_row = 0;

	if ( (t++%frame_delay) == 0 )
		need_render_frame = 1;

}

// attach a piezo to PORTD6 and GND.
void beep()
{
	int i;

	return;
	//	This routine is disabled.
	//	I'm using PORTD6 as a redundant switch input right now.
	//	It's at the bottom corner of the chip opposite GND.
	//	Perfect place to dead-bug a pushbutton.

	for(i=0; i<100; i++)
	{
		PORTD = _BV(0) | _BV(5) | _BV(6);		_delay_us(100);
		PORTD = 0;		_delay_us(100);
	}
	
}


/////////////////////////////////////////////////////////////////////
// ATTENTION:                                                      //
// The following 3 functions serve as the LED "driver":            //
// reset_led(), set_row(r), and set_column(c).
// Adjust these functions to match your LED module.                //
/////////////////////////////////////////////////////////////////////


// Two drivers are provided here.
// For Lite-on LTP-747 & LTP-757 families.

#define LTP	747
// #define LTP	757



#if LTP == 747

// Set port pins high and low in such order as to turn off the LED.
// This depends on the LED's cathode / anode arrangement etc.  
// At the same time, preserve pull-up resistors for switches.
void reset_led()
{
	// keep pull-up resistors active	
	PORTD = _BV(0) | _BV(5) | _BV(6);
		
	// hi/low the port pins to power off LED. (see datasheets)
	PORTA = _BV(1);
	PORTB = _BV(3) | _BV(5) | _BV(6);
	PORTD |= _BV(2) | _BV(3) | _BV(4);
}

// energize row r (call once)
void set_row(int r) 
{ 
	switch(r)
	{
		case 0:	PORTD |= _BV(1);	break;
		case 1:	PORTA |= _BV(0);	break;
		case 2:	PORTB |= _BV(4);	break;
		case 3:	PORTB |= _BV(1);	break;
		case 4:	PORTB |= _BV(2);	break;
	}
}

// energize col c (call once for each lit pixel in column)
void set_column(int c)
{
	switch(c)
	{
		case 6: PORTB &= ~_BV(6);		break;
		case 1: PORTD &= ~_BV(3);		break;
		case 2: PORTD &= ~_BV(2);		break;
		case 4: PORTA &= ~_BV(1);		break;
		case 3: PORTB &= ~_BV(3);		break;	
		case 5: PORTB &= ~_BV(5);		break;
		case 0: PORTD &= ~_BV(4);		break;
	}	
}

#endif

#if LTP == 757

// Set port pins high and low in such order as to turn off the LED.
// This depends on the LED's cathode / anode arrangement etc.  
// At the same time, preserve pull-up resistors for switches.
void reset_led()
{
	// keep pull-up resistors active	
	PORTD = _BV(0) | _BV(5) | _BV(6);
		
	// hi/low the port pins to power off LED. (see datasheets)
	PORTA = _BV(0);
	PORTB = _BV(1) | _BV(2) | _BV(4);
	PORTD |= _BV(1);
}

// energize row r (call once)
void set_row(int r) 
{ 
	switch(r)
	{
		case 0:	PORTD &= ~_BV(1);	break;
		case 1:	PORTA &= ~_BV(0);	break;
		case 2:	PORTB &= ~_BV(4);	break;
		case 3:	PORTB &= ~_BV(1);	break;
		case 4:	PORTB &= ~_BV(2);	break;
	}
}

// energize col c (call once for each lit pixel in column)
void set_column(int c)
{
	switch(c)
	{
		case 6: PORTB |= _BV(6);		break;
		case 1: PORTD |= _BV(3);		break;
		case 2: PORTD |= _BV(2);		break;
		case 4: PORTA |= _BV(1);		break;
		case 3: PORTB |= _BV(3);		break;	
		case 5: PORTB |= _BV(5);		break;
		case 0: PORTD |= _BV(4);		break;
	}	
}

#endif

/////////////////////////////////////////////////////////////////////
// end of LED specific hardware code                               //
/////////////////////////////////////////////////////////////////////



// render and energize the current row, based on bitmap array
void refresh_line()
{
	int c;

	reset_led();
	set_row(current_row);
	
	for (c=0; c<COLS; c++) 
		if (bitmap[current_row][c]) set_column(c);

	/* GARY - Tuesday Sept 26 11:44pm */

	need_refresh_line = 0;
}


// zero out the bitmap array
void clear_bitmap()
{
	int c,r;

	for (c=0; c<COLS; c++)
		for (r=0; r<ROWS; r++)
			bitmap[r][c] = 0 ;
}



/////////////////////////////////////////////////////////////////////
//                                   static 5x7 graphics / symbols //
/////////////////////////////////////////////////////////////////////
#define CHARS 15
const unsigned char charset[CHARS][5] PROGMEM = 
{
	{ 0xFF, 0x41, 0x5D, 0x41, 0xFF },	// psycho 2
	{ 0x00, 0x3E, 0x22, 0x3E, 0x00 },	// psycho 1
	{ 0x0E, 0x3B, 0x17, 0x3B, 0x0E },	// skull
 	{ 0x0C, 0x12, 0x24, 0x12, 0x0C },	// heart
 	{ 0x0A, 0x00, 0x55, 0x00, 0x0A },	// flower
	{ 0x08, 0x14, 0x2A, 0x14, 0x08 },	// diamond
	{ 0x07, 0x49, 0x71, 0x49, 0x07 },	// cup
 	{ 0x22, 0x14, 0x6B, 0x14, 0x22 },	// star2
	{ 0x36, 0x36, 0x08, 0x36, 0x36 },	// star3
	{ 0x06, 0x15, 0x69, 0x15, 0x06 },	// nuke
 	{ 0x0F, 0x1A, 0x3E, 0x1A, 0x0F },	// fox
	{ 0x6C, 0x1A, 0x6F, 0x1A, 0x6C },	// alien
	{ 0x7D, 0x5A, 0x1E, 0x5A, 0x7D },	// alien
	{ 0x4E, 0x7B, 0x0F, 0x7B, 0x4E },	// alien
	{ 0x3D, 0x66, 0x7C, 0x66, 0x3D }	// alien
//	{ 0x, 0x, 0x, 0x, 0x }
};


// renders character c onto the bitmap
void render_character(int c)
{
	int x,y, byte;

	clear_bitmap();

	for (y=0; y<ROWS; y++)
	{
		byte = pgm_read_byte(&(charset[c][y]));

		for (x=0; x<COLS; x++)
		{
			if (byte & _BV(0)) bitmap[y][x] = 1;
			byte = byte >> 1;
		}		
	}	
}


/////////////////////////////////////////////////////////////////////
//                                                      animations //
/////////////////////////////////////////////////////////////////////

void render_checkerboard()
{
	int c,r;

	frame_delay = 300;

	// fill the frame buffer with a procedural pattern

	for (c=0; c<COLS; c++)
		for (r=0; r<ROWS; r++)
		{
			bitmap[r][c] = (r + c + frame) % 2 ;
		}
}

void render_rain()
{
	int y;

	frame_delay = 20;
	clear_bitmap();

	// this is a modulus based particle system

	y = frame%19;	
	if (y<COLS) bitmap[0][y] = 1;

	y = frame%11;	
	if (y<COLS) bitmap[2][y] = 1;

	y = frame%17;	
	if (y<COLS) bitmap[4][y] = 1;
}


void render_psycho()
{
	frame_delay = 30;

	// simple 2 frame animation
	
	if (frame%2) render_character(0);
	else render_character(1);
}

void render_heartbeat()
{
	frame_delay = 40;

	// how to sequence frames using case statement

	switch (frame%10)
	{
		case 0: render_character(3);	break;	
		case 1: clear_bitmap();			break;	
		case 2: render_character(3);	break;	
		case 3: clear_bitmap();			break;	
	}
}

void render_fire()
{
	int r;

	frame_delay = 40;
	clear_bitmap();

	// another modulus based particle system

	// fire body
	r = (frame+0)%3;	bitmap[0][6-r] = 1;
	r = (frame+1)%2;	bitmap[1][6-r] = 1;
	r = (frame+0)%2;	bitmap[2][6-r] = 1;
	r = (frame+1)%2;	bitmap[3][6-r] = 1;
	r = (frame+1)%3;	bitmap[4][6-r] = 1;

	r = (frame+1)%5;	bitmap[1][6-r] = 1;
	r = (frame+0)%3;	bitmap[2][6-r] = 1;
	r = (frame+2)%5;	bitmap[3][6-r] = 1;

	r = (frame+4)%4;	bitmap[0][6-r] = 1;
	r = (frame+1)%4;	bitmap[1][6-r] = 1;
	r = (frame+0)%4;	bitmap[2][6-r] = 1;
	r = (frame+3)%4;	bitmap[3][6-r] = 1;
	r = (frame+2)%4;	bitmap[4][6-r] = 1;

	// sparks
	r = (frame+0)%19;	if (r<COLS) bitmap[0][6-r] = 1;
	r = (frame+0)%6;	if (r<COLS) bitmap[1][6-r] = 1;
	r = (frame+0)%7;	if (r<COLS) bitmap[2][6-r] = 1;
	r = (frame+2)%6;	if (r<COLS) bitmap[3][6-r] = 1;
	r = (frame+0)%17;	if (r<COLS) bitmap[4][6-r] = 1;

}




// renders the correct image / animation onto the bitmap
#define MODES 18
void render_buffer()
{
	frame++;
	need_render_frame = 0;
	
	switch(mode)
	{
		case 1:	render_checkerboard();	break;
		case 2:	render_psycho();	break;
		case 3:	render_heartbeat();	break;
		case 4:	render_rain();		break;
		case 5:	render_fire();		break;
		case 6:	render_character(2);	break;
		case 7: render_character(3);	break;
		case 8: render_character(4);	break;
		case 9: render_character(5);	break;
		case 10: render_character(6);	break;
		case 11: render_character(7);	break;
		case 12: render_character(8);	break;
		case 13: render_character(9);	break;
		case 14: render_character(10);	break;
		case 15: render_character(11);	break;
		case 16: render_character(12);	break;
		case 17: render_character(13);	break;
		case 18: render_character(14);	break;
	}
}

// poll the pushbuttons, and record their states.
// increment/decrement 'mode' in response.
void check_inputs()
{
	// button 1 state (PORTD0 or PORTD6)
	if ((PIND & _BV(0)) == 0 || (PIND & _BV(6)) == 0) b1++;	else b1 = 0;

	// button 2 state (PORTD5)
	if ((PIND & _BV(5)) == 0) b2++;	else b2 = 0;

	// rudimentary de-bouncing
	if (b2 == 10) 		{ beep(); mode--; need_render_frame = 1; }
	if (b1 == 10) 		{ beep(); mode++; need_render_frame = 1; }

	// wraparound (optional)
	if (mode > MODES) mode = 1;
	if (mode < 1) mode = MODES;
}



////////////////////////////////////////////////////////////
//                                         initialization //
////////////////////////////////////////////////////////////
void init()
{
	// set output pins
	DDRA = _BV(0) | _BV(1);
	DDRB = _BV(1) | _BV(2) | _BV(3) | _BV(4) | _BV(5) | _BV(6);
	DDRD = _BV(1) | _BV(2) | _BV(3) | _BV(4);

	// setup Timer/Counter0 for LED refresh
	TCCR0A = _BV(WGM01);	// Set CTC mode
	TCCR0B = _BV(CS02);		// Set prescaler clk/256
	OCR0A = 40;				// ~ 390 interrupts/sec (by 5 cols = ~78fps)
	TIMSK = _BV(OCIE0A);	// Enable T/C 0A

	sei();

	mode = 6;	// Initial display pattern
}

//////////////////////////////////////////////////////////// 
//                                              main loop // 
//////////////////////////////////////////////////////////// 
void main_loop() 
{ 
	for (;;)
	{
		if (need_render_frame) render_buffer();		
		if (need_refresh_line)	
		{	
			refresh_line();
			check_inputs();
		}
	}	
}




////////////////////////////////////////////////////////////
//                                                  main  //
////////////////////////////////////////////////////////////
int main (void)
{
	init();
	main_loop();
	return (0);
}

