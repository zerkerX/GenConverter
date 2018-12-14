/* Genesis to USB Converter
 * Copyright (C) 2018 Ryan Armstrong <git@zerker.ca>

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

