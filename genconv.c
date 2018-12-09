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

static bool six_button_pad = false;

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
    if (six_button_pad)
    {
        /* 6-Button layout is more similar to existing 
         * PS3 fighting sticks (though proving L1/R1 instead of R1/R2)*/
        gamepad_state.square_btn = button_states[GEN_X];
        gamepad_state.cross_btn = button_states[GEN_A];
        gamepad_state.circle_btn = button_states[GEN_B];
        gamepad_state.triangle_btn = button_states[GEN_Y];
        gamepad_state.l1_btn = button_states[GEN_Z];
        gamepad_state.r1_btn = button_states[GEN_C];
        
        gamepad_state.select_btn = button_states[GEN_MODE];
    }
    else
    {
        /* 3-Button layout prefers having all three buttons on the face */
        gamepad_state.square_btn = button_states[GEN_A];
        gamepad_state.cross_btn = button_states[GEN_B];
        gamepad_state.circle_btn = button_states[GEN_C];
    }
    
    gamepad_state.start_btn = button_states[GEN_START];
    
    if (button_states[GEN_UP])
    {
        if (button_states[GEN_LEFT])
        {
            gamepad_state.direction = dir_upleft;
        }
        else if (button_states[GEN_RIGHT])
        {
            gamepad_state.direction = dir_upright;
        }
        else
        {
            gamepad_state.direction = dir_up;
        }
    }
    else if (button_states[GEN_DOWN])
    {
        if (button_states[GEN_LEFT])
        {
            gamepad_state.direction = dir_downleft;
        }
        else if (button_states[GEN_RIGHT])
        {
            gamepad_state.direction = dir_downright;
        }
        else
        {
            gamepad_state.direction = dir_down;
        }
    }
    else if (button_states[GEN_LEFT])
    {
        gamepad_state.direction = dir_left;
    }
    else if (button_states[GEN_RIGHT])
    {
        gamepad_state.direction = dir_right;
    }
    else
    {
        gamepad_state.direction = dir_center;
    }
    
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
        six_button_pad = false;
        
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
                six_button_pad = true;
                
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

