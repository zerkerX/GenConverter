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
#include <stdbool.h>

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

/* Going to waste one position, but it's worth it for a default of unassigned
 * in the mapping arrays */
enum gen_buttons {
    GEN_UNASSIGNED = 0,
    GEN_RIGHT,
    GEN_LEFT,
    GEN_UP,
    GEN_DOWN,
    GEN_A,
    GEN_B,
    GEN_C,
    GEN_START,
    GEN_X,
    GEN_Y,
    GEN_Z,
    GEN_MODE,
    NUM_GEN_BUTTONS
};

enum gamepad_mode {
    MODE_C64 = 0,
    MODE_3BUTTON,
    MODE_6BUTTON
};

static const uint8_t button_mappings[NUM_GEN_BUTTONS] = 
{
    [GEN_RIGHT] = KEY_RIGHT,
    [GEN_LEFT] = KEY_LEFT,
    [GEN_UP] = KEY_UP,
    [GEN_DOWN] = KEY_DOWN,
    [GEN_A] = KEY_Z,
    [GEN_B] = KEY_X,
    [GEN_C] = KEY_C,
    [GEN_START] = KEY_ENTER,
    [GEN_X] = KEY_A,
    [GEN_Y] = KEY_S,
    [GEN_Z] = KEY_D,
    [GEN_MODE] = KEY_SPACE
};

static bool button_states[NUM_GEN_BUTTONS] = { false };

static const enum gen_buttons mux1_map[8] =
{
    [0] = GEN_RIGHT,
    [1] = GEN_LEFT,
    [2] = GEN_DOWN,
    [3] = GEN_UP,
    [4] = GEN_C,
    [6] = GEN_B
};

static const enum gen_buttons mux0_map[8] =
{
    [4] = GEN_START,
    [6] = GEN_A
};

#define MUX_PIN 5
#define LEFT_RIGHT_MASK 0x03

void update_mux(bool muxstate, const enum gen_buttons mux_map[])
{
    uint8_t portvals, i;
    enum gen_buttons button;
        
    if (muxstate)
        PORTB |= (1<<5);
    else
        PORTB &= ~(1<<5);
    _delay_ms(1);
    
    portvals = PINB;
    
    for (i = 0; i < 8; i++)
    {
        button = mux_map[i];
        
        if (button != GEN_UNASSIGNED)
        {
            button_states[button] = (portvals & (1 << i)) == 0 ? true : false;
        }
    }
}


void update_keyboard_state(void)
{
    uint8_t i;
    enum gen_buttons button;
    
    for (i = 0; i < MAX_PRESSED_KEYS; i++)
    {
        keyboard_keys[i] = 0;
    }
    
    i = 0;
    for (button = GEN_RIGHT; button < NUM_GEN_BUTTONS; button++)
    {
        if (button_states[button] && i < MAX_PRESSED_KEYS)
        {
            keyboard_keys[i] = button_mappings[button];
            i++;
        }
    }
    
    usb_keyboard_send();
}



int main(void)
{
    enum gamepad_mode gpmode;
    
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
        gpmode = MODE_C64;
        update_mux(true, mux1_map);
        update_mux(false, mux0_map);
        if ((PINB & LEFT_RIGHT_MASK) == 0)
        {
            gpmode = MODE_3BUTTON;
            /* TODO: Continue to test for 6 button now.
             * Maybe don't even need the mode variable? */
        }
        else
        {
            /* Re-order the buttons to allow C64 buttons to be primary */
            button_states[GEN_A] = button_states[GEN_B];
            button_states[GEN_B] = button_states[GEN_C];
            button_states[GEN_C] = false;
            button_states[GEN_START] = false;
        }
        
        update_keyboard_state();
    }
}

