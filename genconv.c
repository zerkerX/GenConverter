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
#include <util/delay.h>
#include "usb_keyboard.h"
#include "genesis_pad.h"

#include <stdbool.h>


#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n))


/** Mapping of Sega Genesis button to keyboard keys */
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


/** Update the Keyboard button pressed/release status based on 
 * Genesis button states */
void update_keyboard_state(void)
{
    uint8_t i;
    enum genesis_buttons button;
    
    for (i = 0; i < MAX_PRESSED_KEYS; i++)
    {
        keyboard_keys[i] = 0;
    }
    
    i = 0;
    for (button = GEN_RIGHT; button < NUM_GEN_BUTTONS; button++)
    {
        if (genesis_button_states[button] && i < MAX_PRESSED_KEYS)
        {
            keyboard_keys[i] = button_mappings[button];
            i++;
        }
    }
    
    usb_keyboard_send();
}


/** Main program loop */
int main(void)
{
    // set for 16 MHz clock
    CPU_PRESCALE(0);
    
    genesis_init();

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
        genesis_load();
        update_keyboard_state();
        _delay_ms(10);
    }
}

