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

#include "genesis_pad.h"


/** Map of PortB pins to Genesis buttons when mux is high */
static const enum genesis_buttons mux1_map[8] =
{
    [0] = GEN_RIGHT,
    [1] = GEN_LEFT,
    [2] = GEN_DOWN,
    [3] = GEN_UP,
    [4] = GEN_C,
    [6] = GEN_B
};

/** Map of PortB pins to Genesis buttons when mux is low */
static const enum genesis_buttons mux0_map[8] =
{
    [4] = GEN_START,
    [6] = GEN_A
};

/** Map of PortB pins to Genesis buttons when the extra
 * buttons for the 6-button pad are reported. */
static const enum genesis_buttons sixbutton_map[8] =
{
    [0] = GEN_MODE,
    [1] = GEN_X,
    [2] = GEN_Y,
    [3] = GEN_Z
};


/** Current pressed/release state of each Sega Genesis button */
bool genesis_button_states[NUM_GEN_BUTTONS] = { false };

/** Which gamepad type is connected */
enum genesis_type genesis_pad_type = GEN_TYPE_1_2_BUTTON;


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
static void load_buttons(const enum genesis_buttons mux_map[])
{
    uint8_t portvals, i;
    enum genesis_buttons button;
        
    portvals = PINB;
    
    for (i = 0; i < 8; i++)
    {
        button = mux_map[i];
        
        if (button != GEN_UNASSIGNED)
        {
            genesis_button_states[button] = (portvals & (1 << i)) == 0 ? true : false;
        }
    }
}


/* Public methods follow */

void genesis_init(void)
{
    DDRB = 0x00 | (1 << 5);
    PORTB = 0xFF;
}


void genesis_load(void)
{
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
            genesis_pad_type = GEN_TYPE_6_BUTTON;
            
            mux_high();
            load_buttons(sixbutton_map);
            mux_low();
        }
        else
        {
            genesis_pad_type = GEN_TYPE_3_BUTTON;
        }
    }
    else
    {
        /* 1/2 button 8-bit computer stick or SMS pad. 
         * Re-order the buttons so the primary 8-bit computer
         * button is A, and the optional other button is B */
        genesis_button_states[GEN_A] = genesis_button_states[GEN_B];
        genesis_button_states[GEN_B] = genesis_button_states[GEN_C];
        genesis_button_states[GEN_C] = false;
        
        genesis_pad_type = GEN_TYPE_1_2_BUTTON;
    }

}
