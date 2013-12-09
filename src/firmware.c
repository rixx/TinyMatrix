////////////////////////////////////////////////////////////
// TinyMatrix.c                                           //
// copyright 2012 Tim Toner (tigeruppp/at/gmail.com)      //
// modified 2013 rixx (git/at/foyfoy.de)                  //
// licensed under the GNU GPL v2 or newer                 //
////////////////////////////////////////////////////////////

// Two drivers are provided here.
// For Lite-on LTP-747 & LTP-757 families.

#if LTP == 747

// Set port pins high and low in such order as to turn off the LED.
// This depends on the LED's cathode / anode arrangement etc.  
// At the same time, preserve pull-up resistors for switches.
void reset_led() {
    // keep pull-up resistors active    AND hi/low the port pins to power off LED (see datasheets)
    PORTD = _BV(0) | _BV(5) | _BV(6)    | _BV(2) | _BV(3) | _BV(4);
    PORTA = _BV(1);
    PORTB = _BV(3) | _BV(5) | _BV(6);
}

// energize row r (call once)
void set_row(int r) { 
    switch(r) {
        case 0: PORTD |= _BV(1);
                break;
        case 1: PORTA |= _BV(0);
                break;
        case 2: PORTB |= _BV(4);
                break;
        case 3: PORTB |= _BV(1);
                break;
        case 4: PORTB |= _BV(2);
                break;
    }
}

// energize col c (call once for each lit pixel in column)
void set_column(int c) {
    switch(c) {
        case 6: PORTB &= ~_BV(6);
                break;
        case 1: PORTD &= ~_BV(3);
                break;
        case 2: PORTD &= ~_BV(2);
                break;
        case 4: PORTA &= ~_BV(1);
                break;
        case 3: PORTB &= ~_BV(3);
                break;  
        case 5: PORTB &= ~_BV(5);
                break;
        case 0: PORTD &= ~_BV(4);
                break;
    }   
}

#endif

#if LTP == 757

// Set port pins high and low in such order as to turn off the LED.
// This depends on the LED's cathode / anode arrangement etc.  
// At the same time, preserve pull-up resistors for switches.
void reset_led() {
    // keep pull-up resistors active    
    PORTD = _BV(0) | _BV(5) | _BV(6);
        
    // hi/low the port pins to power off LED. (see datasheets)
    PORTA = _BV(0);
    PORTB = _BV(1) | _BV(2) | _BV(4);
    PORTD |= _BV(1);
}

// energize row r (call once)
void set_row(int r) { 
    switch(r) {
        case 0: PORTD &= ~_BV(1);
                break;
        case 1: PORTA &= ~_BV(0);
                break;
        case 2: PORTB &= ~_BV(4);
                break;
        case 3: PORTB &= ~_BV(1);
                break;
        case 4: PORTB &= ~_BV(2);
                break;
    }
}

// energize col c (call once for each lit pixel in column)
void set_column(int c) {
    switch(c) {
        case 6: PORTB |= _BV(6);
                break;
        case 1: PORTD |= _BV(3);
                break;
        case 2: PORTD |= _BV(2);
                break;
        case 4: PORTA |= _BV(1);
                break;
        case 3: PORTB |= _BV(3);
                break;  
        case 5: PORTB |= _BV(5);
                break;
        case 0: PORTD |= _BV(4);
                break;
    }   
}

#endif
