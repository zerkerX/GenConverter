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
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usb_gamepad.h"
#include <stdbool.h>
#include <string.h>

#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n))

/** Type for all available Sega Genesis buttons
 * 
 * Sizing for key mapping/state arrys based on the NUM member of this type.
 * Allocating a spot for UNASSIGNED is going to waste one position, 
 * but it's worth it for a default of unassigned in the bit mapping arrays */
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


/** Current pressed/release state of each Sega Genesis button */
static bool button_states[NUM_GEN_BUTTONS] = { false };

/** Map of PortB pins to Genesis buttons when mux is high */
static const enum gen_buttons mux1_map[8] =
{
    [0] = GEN_RIGHT,
    [1] = GEN_LEFT,
    [2] = GEN_DOWN,
    [3] = GEN_UP,
    [4] = GEN_C,
    [6] = GEN_B
};

/** Map of PortB pins to Genesis buttons when mux is low */
static const enum gen_buttons mux0_map[8] =
{
    [4] = GEN_START,
    [6] = GEN_A
};

/** Map of PortB pins to Genesis buttons when the extra
 * buttons for the 6-button pad are reported. */
static const enum gen_buttons sixbutton_map[8] =
{
    [0] = GEN_MODE,
    [1] = GEN_X,
    [2] = GEN_Y,
    [3] = GEN_Z
};

/** Which Pin in Port B is used for mux control */
#define MUX_PIN 5

/** Mask to detect a 3-button Genesis pad */
#define LEFT_RIGHT_MASK 0x03

/** Mask to detect a 6-button Genesis pad */
#define ALL_DIRECTION_MASK 0x0F

static inline void mux_high(void)
{
    PORTB |= (1<<5);
    _delay_us(100);
}

static inline void mux_low(void)
{
    PORTB &= ~(1<<5);
    _delay_us(100);
}


/** Loads the current button states from Port B according to 
 * the provided map
 * 
 * \param mux_map Array mapping pin positions to Genesis buttons
 */
void load_buttons(const enum gen_buttons mux_map[])
{
    uint8_t portvals, i;
    enum gen_buttons button;
        
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

/** Update the USB HID Gamepad pressed/release status based on 
 * Genesis button states */
void update_usb_gamepad_state(void)
{
    gamepad_state.button1 = button_states[GEN_A];
    gamepad_state.button2 = button_states[GEN_B];
    gamepad_state.button3 = button_states[GEN_C];
    gamepad_state.button4 = button_states[GEN_X];
    gamepad_state.button5 = button_states[GEN_Y];
    gamepad_state.button6 = button_states[GEN_Z];
    
    gamepad_state.button_Select = button_states[GEN_MODE];
    gamepad_state.button_Start = button_states[GEN_START];
    
    if (button_states[GEN_LEFT])
        gamepad_state.xAxis = 0;
    else if (button_states[GEN_RIGHT])
        gamepad_state.xAxis = 255;
    else
        gamepad_state.xAxis = 127;

    if (button_states[GEN_UP])
        gamepad_state.yAxis = 0;
    else if (button_states[GEN_DOWN])
        gamepad_state.yAxis = 255;
    else
        gamepad_state.yAxis = 127;
    
    usb_gamepad_send();
}


/** Main program loop */
int main(void)
{
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
        usb_gamepad_reset_state();
        
        mux_high();
        load_buttons(mux1_map);
        mux_low();

        if ((PINB & LEFT_RIGHT_MASK) == 0)
        {
            /* Confirmed 3-button pad */
            load_buttons(mux0_map);
            
            /* Detection sequence for 6-button pad now... 
             * Also see https://segaretro.org/Six_Button_Control_Pad_(Mega_Drive) */
            mux_high();
            mux_low();
            mux_high();
            mux_low();
            
            if ((PINB & ALL_DIRECTION_MASK) == 0)
            {
                /* Confirmed 6-button pad */
                mux_high();
                load_buttons(sixbutton_map);
                mux_low();
            }
        }
        else
        {
            /* 1/2 button 8-bit computer stick or SMS pad. 
             * Re-order the buttons so the primary 8-bit computer
             * button is A, and the optional other button is B */
            button_states[GEN_A] = button_states[GEN_B];
            button_states[GEN_B] = button_states[GEN_C];
            button_states[GEN_C] = false;
        }
        
        update_usb_gamepad_state();
        _delay_ms(10);
    }
}

