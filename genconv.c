/* LED Blink Example with USB Debug Channel for Teensy USB Development Board
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2008, 2010 PJRC.COM, LLC
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usb_keyboard.h"


#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n))

/* Notes:
 * For C64 mode:
 * 0 = Right
 * 1 = Left
 * 2 = Down
 * 3 = Up
 * 6 = B1
 * 4 = B2
 * 
 * For Genesis, 5 should be the MUX
 */ 

uint8_t number_keys[]=
    {KEY_0,KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7};

uint16_t idle_count=0;

int main(void)
{
    uint8_t mux1, mux0, mask, i;
    uint8_t mux1_prev=0xFF, mux0_prev=0xFF;

    // set for 16 MHz clock
    CPU_PRESCALE(0);
    
    DDRB = 0x00 | (1 << 5);
    PORTB = 0xFF;

    // Initialize the USB, and then wait for the host to set configuration.
    // If the Teensy is powered without a PC connected to the USB port,
    // this will wait forever.
    usb_init();
    while (!usb_configured()) {}

    // Wait an extra second for the PC's operating system to load drivers
    // and do whatever it does to actually be ready for input
    _delay_ms(1000);

    while (1)
    {
        // read all port B and port D pins
        mux1 = PINB;

        // check if any pins are low, but were high previously
        mask = 1;
        for (i=0; i<8; i++) {
            if (((mux1 & mask) == 0) && (mux1_prev & mask) != 0)
            {
                usb_keyboard_press(number_keys[i], 0);
            }
            //~ else if (((b & mask) != 0) && (b_prev & mask) == 0)
            //~ {
                //~ usb_keyboard_press(number_keys[i], 0);
            //~ }
            mask = mask << 1;
        }

        // now the current pins will be the previous, and
        // wait a short delay so we're not highly sensitive
        // to mechanical "bounce".
        mux1_prev = mux1;
        
        PORTB &= ~(1<<5);
        _delay_ms(2);
        
        mux0 = PINB;
        
        mask = (1<<4);
        if (((mux0 & mask) == 0) && (mux0_prev & mask) != 0)
        {
            usb_keyboard_press(KEY_Z, 0);
        }
        mask = (1<<6);
        if (((mux0 & mask) == 0) && (mux0_prev & mask) != 0)
        {
            usb_keyboard_press(KEY_X, 0);
        }
        
        mux0_prev = mux0;
        
            
        PORTB |= (1<<5);
        _delay_ms(2);
    }
}

