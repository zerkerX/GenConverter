/* Genesis to USB Converter
 * Copyright (C) 2018 Ryan Armstrong <git@zerker.ca>
 *
 * Based on work by:
 *   Josh Kropf <https://github.com/jiggak/teensy-snes>
 *
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
#include "usb_gamepad.h"
#include "genesis_pad.h"

#include <stdbool.h>


#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n))


/** Update the USB HID Gamepad pressed/release status based on 
 * Genesis button states */
void update_usb_gamepad_state(void)
{
    if (genesis_pad_type == GEN_TYPE_6_BUTTON)
    {
        /* 6-Button layout is more similar to existing 
         * PS3 fighting sticks (though proving L1/R1 instead of R1/R2)*/
        gamepad_state.square_btn = genesis_button_states[GEN_X];
        gamepad_state.cross_btn = genesis_button_states[GEN_A];
        gamepad_state.circle_btn = genesis_button_states[GEN_B];
        gamepad_state.triangle_btn = genesis_button_states[GEN_Y];
        gamepad_state.l1_btn = genesis_button_states[GEN_Z];
        gamepad_state.r1_btn = genesis_button_states[GEN_C];
        
        gamepad_state.select_btn = genesis_button_states[GEN_MODE];
    }
    else
    {
        /* 3-Button layout prefers having all three buttons on the face */
        gamepad_state.square_btn = genesis_button_states[GEN_A];
        gamepad_state.cross_btn = genesis_button_states[GEN_B];
        gamepad_state.circle_btn = genesis_button_states[GEN_C];
    }
    
    gamepad_state.start_btn = genesis_button_states[GEN_START];
    
    if (genesis_button_states[GEN_UP])
    {
        if (genesis_button_states[GEN_LEFT])
            gamepad_state.direction = dir_upleft;
        else if (genesis_button_states[GEN_RIGHT])
            gamepad_state.direction = dir_upright;
        else
            gamepad_state.direction = dir_up;
    }
    else if (genesis_button_states[GEN_DOWN])
    {
        if (genesis_button_states[GEN_LEFT])
            gamepad_state.direction = dir_downleft;
        else if (genesis_button_states[GEN_RIGHT])
            gamepad_state.direction = dir_downright;
        else
            gamepad_state.direction = dir_down;
    }
    else if (genesis_button_states[GEN_LEFT])
        gamepad_state.direction = dir_left;
    else if (genesis_button_states[GEN_RIGHT])
        gamepad_state.direction = dir_right;
    else
        gamepad_state.direction = dir_center;
    
    usb_gamepad_send();
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
        usb_gamepad_reset_state();
        genesis_load();
        update_usb_gamepad_state();
        _delay_ms(10);
    }
}

