////////////////////////////////////////////////////////////
// TinyMatrix.c                                           //
// copyright 2012 Tim Toner (tigeruppp/at/gmail.com)      //
// modified 2013 rixx (git/at/foyfoy.de)                  //
// licensed under the GNU GPL v2 or newer                 //
////////////////////////////////////////////////////////////

#define LTP 747
#define ROWS 5
#define COLS 7
#define EVER ;;

#include <inttypes.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include "firmware.c"


char bitmap[ROWS][COLS];    
char need_refresh_line;
char need_render_frame;
unsigned char current_row;      // current lit row


// globals
int mode;                       // current display mode
int t, frame, frame_delay;      // animation timing variables
int b1;   // button states


// LED refresh interrupt, called 390 times per second
ISR(TIMER0_COMPA_vect) {
    need_refresh_line = 1;  // flag to display new row of LEDs

    if (++current_row >= ROWS) 
        current_row = 0;

    if (t++ % frame_delay == 0)
        need_render_frame = 1;
}


// render and energize the current row, based on bitmap array
void refresh_line() {
    int c;

    reset_led();
    set_row(current_row);
    
    for (c = 0; c < COLS; c++) 
        if (bitmap[current_row][c]) 
            set_column(c);

    need_refresh_line = 0;
}


// zero out the bitmap array
void clear_bitmap() {
    int c, r;

    for (c = 0; c < COLS; c++)
        for (r = 0; r < ROWS; r++)
            bitmap[r][c] = 0;
}



/////////////////////////////////////////////////////////////////////
//                                   static 5x7 graphics / symbols //
/////////////////////////////////////////////////////////////////////
#define CHARS 11
const unsigned char charset[CHARS][5] PROGMEM = {
    { 0xFF, 0x41, 0x5D, 0x41, 0xFF },   // psycho 2
    { 0x00, 0x3E, 0x22, 0x3E, 0x00 },   // psycho 1
    { 0x0C, 0x12, 0x24, 0x12, 0x0C },   // heart
    { 0x08, 0x14, 0x2A, 0x14, 0x08 },   // diamond
    // { 0x07, 0x49, 0x71, 0x49, 0x07 },   // cup
    { 0x22, 0x14, 0x6B, 0x14, 0x22 },   // star2
    { 0x36, 0x36, 0x08, 0x36, 0x36 },   // star3
    { 0x2a, 0x14, 0x7f, 0x14, 0x2a },   //snowflake
    { 0x2a, 0x1c, 0x36, 0x1c, 0x2a },   //snowflake2
    { 0x14, 0x5e, 0x7f, 0x5e, 0x14 },   //tree_full
    // { 0x7b, 0x7c, 0x7c, 0x7a, 0x7a },   //present
    { 0x38, 0x7c, 0x7f, 0x74, 0x38 },   //ball
    { 0x7e, 0x29, 0x7d, 0x2, 0x0 },   //sled



//{ 0x0, 0x74, 0x7a, 0x74, 0x0 },   //candle
//{ 0x0, 0x70, 0x7e, 0x74, 0x0 },   //candle2
//{ 0x0, 0x70, 0x7c, 0x76, 0x0 },   //candle3
//{ 0x0, 0x74, 0x7e, 0x70, 0x0 },   //candle4
//{ 0x0, 0x76, 0x7c, 0x70, 0x0 },   //candle5




};


// renders character c onto the bitmap
void render_character(int c) {
    int x, y, byte;

    clear_bitmap();

    for (y = 0; y < ROWS; y++) {
        byte = pgm_read_byte(&(charset[c][y]));

        for (x = 0; x < COLS; x++) {
            if (byte & _BV(0)) 
                bitmap[y][x] = 1;
            byte = byte >> 1;
        }       
    }   
}


/////////////////////////////////////////////////////////////////////
//                                                      animations //
/////////////////////////////////////////////////////////////////////

void render_checkerboard() {
    int c, r;

    frame_delay = 300;

    // fill the frame buffer with a procedural pattern
    for (c = 0; c < COLS; c++)
        for (r = 0; r < ROWS; r++) {
            bitmap[r][c] = (r + c + frame) % 2;
        }
}

void render_rain() {
    int y;

    frame_delay = 20;
    clear_bitmap();

    // this is a modulus based particle system

    y = frame%19;   
    if (y < COLS) 
        bitmap[0][y] = 1;

    y = frame%11;   
    if (y < COLS)
        bitmap[2][y] = 1;

    y = frame%17;   
    if (y < COLS)
        bitmap[4][y] = 1;
}


void render_psycho() {
    frame_delay = 40;
    clear_bitmap();

    //simple 2 frame animation
    
    if (frame%2)
        render_character(0);
    else
        render_character(1);

}

void render_wave() {
    int c, r, on;
    frame_delay = 60;
    clear_bitmap();


    for (c = 0; c < COLS; c++) {
        for (r = 0; r < ROWS; r++) {
            on = abs((COLS) - ((frame%8))-c-1*(r%2));
            bitmap[r][c] = !(on) | !(on-3) | !(on-6);
        }
    }
}

void render_heartbeat() {
    frame_delay = 40;

    // how to sequence frames using case statement

    switch (frame%10) {
        case 0: render_character(2);
                break;  
        case 1: clear_bitmap();
                break;  
        case 2: render_character(2);
                break;  
        case 3: clear_bitmap();
                break;  
    }
}

// void render_candle() {
//     frame_delay = 20;

//     // how to sequence frames using case statement

//     switch (frame%40) {
//         case 0: render_character(13);
//                 break;
//         case 1: render_character(14);
//                 break;
//         case 2: render_character(15);
//                 break;
//         case 9: render_character(14);
//                 break;
//         case 10: render_character(13);
//                 break;
//         case 20: render_character(16);
//                 break;
//         case 21: render_character(17);
//                 break;
//         case 29: render_character(16);
//                 break;
//         case 30: render_character(13);
//                 break;
//     }
// }

void render_fire() {
    int r, c;

    frame_delay = 40;
    clear_bitmap();

    // another modulus based particle system

    // fire body
    r = (frame)%2;
    bitmap[2][6-r] = 1;

    r = !r;
    c = 6 - r;
    bitmap[1][c] = 1;
    bitmap[3][c] = 1;

    r = (frame)%3;
    c = 6 - r;
    bitmap[0][c] = 1;
    bitmap[2][c] = 1;

    r = (frame)%4;
    bitmap[2][6-r] = 1;

    r = (frame + 1)%3;
    bitmap[4][6-r] = 1;

    r = (frame + 1)%4;
    bitmap[1][6-r] = 1;

    r = (frame + 1)%5;
    bitmap[1][6-r] = 1;

    r = (frame + 2)%4;
    bitmap[4][6-r] = 1;

    r = (frame + 2)%5;
    bitmap[3][6-r] = 1;

    r = (frame + 3)%4;
    bitmap[3][6-r] = 1;

    r = (frame + 4)%4;
    bitmap[0][6-r] = 1;


    // sparks
    r = (frame)%17;
    if (r<COLS)
        bitmap[0][6-r] = 1;

    r = (frame)%6;
    if (r<COLS)
        bitmap[1][6-r] = 1;

    r = (frame)%7;
    if (r<COLS)
        bitmap[2][6-r] = 1;

    r = (frame+2)%6;
    if (r<COLS)
        bitmap[3][6-r] = 1;

    r = (frame)%19;
    if (r<COLS)
        bitmap[4][6-r] = 1;
}


// renders the correct image / animation onto the bitmap
#define MODES 15
void render_buffer() {
    frame++;
    need_render_frame = 0;
    
    switch (mode) {
        case 0: render_checkerboard();
                break;
        case 1: render_wave();
                break;
        case 2: render_psycho();
                break;
        case 3: render_heartbeat();
                break;
        case 4: render_rain();
                break;
        case 5: render_fire();
                break;
        case 6: render_character(2);
                break;
        case 7: render_character(3);
                break;
        case 8: render_character(4);
                break;
        case 9: render_character(5);
                break;
        case 10: render_character(6);
                 break;
        case 11: render_character(7);
                 break;
        case 12: render_character(8);
                 break;
        case 13: render_character(9);
                 break;
        case 14: render_character(10);
                 break;
    }
}

// poll the pushbuttons, and record their states.
// increment/decrement 'mode' in response.
void check_inputs() {

    // button 1 state (PORTD0 or PORTD6)
    if ((PIND & _BV(0)) == 0 || (PIND & _BV(6)) == 0)
        b1++;
    else 
        b1 = 0;

    if (b1 == 10) {
        mode = ++mode % MODES;
        need_render_frame = 1;
    }
}



////////////////////////////////////////////////////////////
//                                         initialization //
////////////////////////////////////////////////////////////
void init() {
    // set output pins
    DDRA = _BV(0) | _BV(1);
    DDRB = _BV(1) | _BV(2) | _BV(3) | _BV(4) | _BV(5) | _BV(6);
    DDRD = _BV(1) | _BV(2) | _BV(3) | _BV(4);

    // setup Timer/Counter0 for LED refresh
    TCCR0A = _BV(WGM01);    // Set CTC mode
    TCCR0B = _BV(CS02);     // Set prescaler clk/256
    OCR0A = 40;             // ~ 390 interrupts/sec (by 5 cols = ~78fps)
    TIMSK = _BV(OCIE0A);    // Enable T/C 0A

    sei();

    mode = 0;   // Initial display pattern
}

//////////////////////////////////////////////////////////// 
//                                              main loop // 
//////////////////////////////////////////////////////////// 
void main_loop() { 
    for (EVER) {
        if (need_render_frame)
            render_buffer();     

        if (need_refresh_line) {   
            refresh_line();
            check_inputs();
        }
    }   
}


////////////////////////////////////////////////////////////
//                                                  main  //
////////////////////////////////////////////////////////////
int main(void) {

    init();
    main_loop();
    return (0);
}

