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
const unsigned char pictograms[14][5] PROGMEM = {
    { 0xFF, 0x41, 0x5D, 0x41, 0xFF },   // psycho 2
    { 0x00, 0x3E, 0x22, 0x3E, 0x00 },   // psycho 1
    { 0x0E, 0x3B, 0x17, 0x3B, 0x0E },   // skull
    { 0x0C, 0x12, 0x24, 0x12, 0x0C },   // heart
    { 0x0A, 0x00, 0x55, 0x00, 0x0A },   // flower
    { 0x08, 0x14, 0x2A, 0x14, 0x08 },   // diamond
    { 0x07, 0x49, 0x71, 0x49, 0x07 },   // cup
    { 0x22, 0x14, 0x6B, 0x14, 0x22 },   // star2
    { 0x36, 0x36, 0x08, 0x36, 0x36 },   // star3
    { 0x0F, 0x1A, 0x3E, 0x1A, 0x0F },   // fox
    { 0x6C, 0x1A, 0x6F, 0x1A, 0x6C },   // alien
    { 0x7D, 0x5A, 0x1E, 0x5A, 0x7D },   // alien
    { 0x4E, 0x7B, 0x0F, 0x7B, 0x4E },   // alien
    { 0x3D, 0x66, 0x7C, 0x66, 0x3D },   // alien
};

const unsigned char charset[96][5] PROGMEM = {
    { 0x00, 0x00, 0x00, 0x00, 0x00 }, // (space)
    { 0x00, 0x00, 0x5F, 0x00, 0x00 }, // !
    { 0x00, 0x07, 0x00, 0x07, 0x00 }, // "
    { 0x14, 0x7F, 0x14, 0x7F, 0x14 }, // #
    { 0x24, 0x2A, 0x7F, 0x2A, 0x12 }, // $
    { 0x23, 0x13, 0x08, 0x64, 0x62 }, // %
    { 0x36, 0x49, 0x55, 0x22, 0x50 }, // &
    { 0x00, 0x05, 0x03, 0x00, 0x00 }, // '
    { 0x00, 0x1C, 0x22, 0x41, 0x00 }, // (
    { 0x00, 0x41, 0x22, 0x1C, 0x00 }, // )
    { 0x08, 0x2A, 0x1C, 0x2A, 0x08 }, // *
    { 0x08, 0x08, 0x3E, 0x08, 0x08 }, // +
    { 0x00, 0x50, 0x30, 0x00, 0x00 }, // ,
    { 0x08, 0x08, 0x08, 0x08, 0x08 }, // -
    { 0x00, 0x60, 0x60, 0x00, 0x00 }, // .
    { 0x20, 0x10, 0x08, 0x04, 0x02 }, // /
    { 0x3E, 0x51, 0x49, 0x45, 0x3E }, // 0
    { 0x00, 0x42, 0x7F, 0x40, 0x00 }, // 1
    { 0x42, 0x61, 0x51, 0x49, 0x46 }, // 2
    { 0x21, 0x41, 0x45, 0x4B, 0x31 }, // 3
    { 0x18, 0x14, 0x12, 0x7F, 0x10 }, // 4
    { 0x27, 0x45, 0x45, 0x45, 0x39 }, // 5
    { 0x3C, 0x4A, 0x49, 0x49, 0x30 }, // 6
    { 0x01, 0x71, 0x09, 0x05, 0x03 }, // 7
    { 0x36, 0x49, 0x49, 0x49, 0x36 }, // 8
    { 0x06, 0x49, 0x49, 0x29, 0x1E }, // 9
    { 0x00, 0x36, 0x36, 0x00, 0x00 }, // :
    { 0x00, 0x56, 0x36, 0x00, 0x00 }, // ;
    { 0x00, 0x08, 0x14, 0x22, 0x41 }, // <
    { 0x14, 0x14, 0x14, 0x14, 0x14 }, // =
    { 0x41, 0x22, 0x14, 0x08, 0x00 }, // >
    { 0x02, 0x01, 0x51, 0x09, 0x06 }, // ?
    { 0x32, 0x49, 0x79, 0x41, 0x3E }, // @
    { 0x7E, 0x11, 0x11, 0x11, 0x7E }, // A
    { 0x7F, 0x49, 0x49, 0x49, 0x36 }, // B
    { 0x3E, 0x41, 0x41, 0x41, 0x22 }, // C
    { 0x7F, 0x41, 0x41, 0x22, 0x1C }, // D
    { 0x7F, 0x49, 0x49, 0x49, 0x41 }, // E
    { 0x7F, 0x09, 0x09, 0x01, 0x01 }, // F
    { 0x3E, 0x41, 0x41, 0x51, 0x32 }, // G
    { 0x7F, 0x08, 0x08, 0x08, 0x7F }, // H
    { 0x00, 0x41, 0x7F, 0x41, 0x00 }, // I
    { 0x20, 0x40, 0x41, 0x3F, 0x01 }, // J
    { 0x7F, 0x08, 0x14, 0x22, 0x41 }, // K
    { 0x7F, 0x40, 0x40, 0x40, 0x40 }, // L
    { 0x7F, 0x02, 0x04, 0x02, 0x7F }, // M
    { 0x7F, 0x04, 0x08, 0x10, 0x7F }, // N
    { 0x3E, 0x41, 0x41, 0x41, 0x3E }, // O
    { 0x7F, 0x09, 0x09, 0x09, 0x06 }, // P
    { 0x3E, 0x41, 0x51, 0x21, 0x5E }, // Q
    { 0x7F, 0x09, 0x19, 0x29, 0x46 }, // R
    { 0x46, 0x49, 0x49, 0x49, 0x31 }, // S
    { 0x01, 0x01, 0x7F, 0x01, 0x01 }, // T
    { 0x3F, 0x40, 0x40, 0x40, 0x3F }, // U
    { 0x1F, 0x20, 0x40, 0x20, 0x1F }, // V
    { 0x7F, 0x20, 0x18, 0x20, 0x7F }, // W
    { 0x63, 0x14, 0x08, 0x14, 0x63 }, // X
    { 0x03, 0x04, 0x78, 0x04, 0x03 }, // Y
    { 0x61, 0x51, 0x49, 0x45, 0x43 }, // Z
    { 0x00, 0x00, 0x7F, 0x41, 0x41 }, // [
    { 0x02, 0x04, 0x08, 0x10, 0x20 }, // "\"
    { 0x41, 0x41, 0x7F, 0x00, 0x00 }, // ]
    { 0x04, 0x02, 0x01, 0x02, 0x04 }, // ^
    { 0x40, 0x40, 0x40, 0x40, 0x40 }, // _
    { 0x00, 0x01, 0x02, 0x04, 0x00 }, // `
    { 0x20, 0x54, 0x54, 0x54, 0x78 }, // a
    { 0x7F, 0x48, 0x44, 0x44, 0x38 }, // b
    { 0x38, 0x44, 0x44, 0x44, 0x20 }, // c
    { 0x38, 0x44, 0x44, 0x48, 0x7F }, // d
    { 0x38, 0x54, 0x54, 0x54, 0x18 }, // e
    { 0x08, 0x7E, 0x09, 0x01, 0x02 }, // f
    { 0x08, 0x14, 0x54, 0x54, 0x3C }, // g
    { 0x7F, 0x08, 0x04, 0x04, 0x78 }, // h
    { 0x00, 0x44, 0x7D, 0x40, 0x00 }, // i
    { 0x20, 0x40, 0x44, 0x3D, 0x00 }, // j
    { 0x00, 0x7F, 0x10, 0x28, 0x44 }, // k
    { 0x00, 0x41, 0x7F, 0x40, 0x00 }, // l
    { 0x7C, 0x04, 0x18, 0x04, 0x78 }, // m
    { 0x7C, 0x08, 0x04, 0x04, 0x78 }, // n
    { 0x38, 0x44, 0x44, 0x44, 0x38 }, // o
    { 0x7C, 0x14, 0x14, 0x14, 0x08 }, // p
    { 0x08, 0x14, 0x14, 0x18, 0x7C }, // q
    { 0x7C, 0x08, 0x04, 0x04, 0x08 }, // r
    { 0x48, 0x54, 0x54, 0x54, 0x20 }, // s
    { 0x04, 0x3F, 0x44, 0x40, 0x20 }, // t
    { 0x3C, 0x40, 0x40, 0x20, 0x7C }, // u
    { 0x1C, 0x20, 0x40, 0x20, 0x1C }, // v
    { 0x3C, 0x40, 0x30, 0x40, 0x3C }, // w
    { 0x44, 0x28, 0x10, 0x28, 0x44 }, // x
    { 0x0C, 0x50, 0x50, 0x50, 0x3C }, // y
    { 0x44, 0x64, 0x54, 0x4C, 0x44 }, // z
    { 0x00, 0x08, 0x36, 0x41, 0x00 }, // {
    { 0x00, 0x00, 0x7F, 0x00, 0x00 }, // |
    { 0x00, 0x41, 0x36, 0x08, 0x00 }, // }
    { 0x08, 0x08, 0x2A, 0x1C, 0x08 }, // ->
    { 0x08, 0x1C, 0x2A, 0x08, 0x08 }  // <-
};

// renders pictogram c onto the bitmap
void render_pictogram(int c) {
    int x, y, byte;

    clear_bitmap();

    for (y = 0; y < ROWS; y++) {
        byte = pgm_read_byte(&(pictograms[c][y]));

        for (x = 0; x < COLS; x++) {
            if (byte & _BV(0))
                bitmap[y][x] = 1;
            byte = byte >> 1;
        }
    }
}

// renders character c onto the bitmap
void render_character(char c) {
    int x, y, byte;

    clear_bitmap();

    for (y = 0; y < ROWS; y++) {
        byte = pgm_read_byte(&(charset[c - 32][abs(y - 4)]));

        for (x = 0; x < COLS; x++) {
            if (byte & _BV(0))
                bitmap[y][x] = 1;
            byte = byte >> 1;
        }
    }
}

void render_string(char* string, int length) {
    frame_delay = 60;
    int frame_at = frame % (length * 4);
    if (frame_at % 4 != 0) return;
    render_character(string[frame_at / 4]);
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
    frame_delay = 30;

    // simple 2 frame animation

    if (frame%2)
        render_pictogram(0);
    else
        render_pictogram(1);
}



void render_ready_to_code() {
    render_string("Ready to Code", 13);
    // frame_delay = 40;
    // switch(frame%53) { //frame % ((number of chars) * 4 + 5)
    //
    //     case 0: render_character(26); break; // R
    //     case 4: render_character(17); break; // E
    //     case 8: render_character(16); break; // D
    //     case 12: render_character(29); break; // Y
    //     case 16: clear_bitmap(); break;
    //     case 20: render_character(28); break; // T
    //     case 24: render_character(25); break; // O
    //     case 28: clear_bitmap(); break;
    //     case 32: render_character(15); break; // C
    //     case 36: render_character(25); break; // O
    //     case 40: render_character(16); break; // D
    //     case 44: render_character(17); break; // E
    //     case 48: clear_bitmap(); break;
    // }
}

void render_heartbeat() {
    frame_delay = 40;

    // how to sequence frames using case statement

    switch (frame%10) {
        case 0: render_pictogram(3);
                break;
        case 1: clear_bitmap();
                break;
        case 2: render_pictogram(3);
                break;
        case 3: clear_bitmap();
                break;
    }
}

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
#define MODES 18
void render_buffer() {
    frame++;
    need_render_frame = 0;

    switch (mode) {
        case 0: render_checkerboard(); break;
        case 1: render_psycho(); break;
        case 2: render_rain(); break;
        case 3: render_ready_to_code(); break;
        case 4: render_fire(); break;
        case 5: render_heartbeat(); break;
        case 6: render_pictogram(2); break;
        case 7: render_pictogram(3); break;
        case 8: render_pictogram(4); break;
        case 9: render_pictogram(5); break;
        case 10: render_pictogram(6); break;
        case 11: render_pictogram(7); break;
        case 12: render_pictogram(8); break;
        case 13: render_pictogram(9); break;
        case 14: render_pictogram(10); break;
        case 15: render_pictogram(11); break;
        case 16: render_pictogram(12); break;
        case 17: render_pictogram(13); break;
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

    mode = 3;   // Initial display pattern
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
