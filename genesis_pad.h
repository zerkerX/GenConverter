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
#include <stdbool.h>

/** Type for all available Sega Genesis buttons
 * 
 * Sizing for key mapping/state arrys based on the NUM member of this type.
 * Allocating a spot for UNASSIGNED is going to waste one position, 
 * but it's worth it for a default of unassigned in the bit mapping arrays */
enum genesis_buttons {
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

enum genesis_type {
    GEN_TYPE_1_2_BUTTON = 0,
    GEN_TYPE_3_BUTTON,
    GEN_TYPE_6_BUTTON
};

/** Current pressed/release state of each Sega Genesis button */
extern bool genesis_button_states[];

/** Which gamepad type is connected */
extern enum genesis_type genesis_pad_type;

/** Prepare the input port for use */
void genesis_init(void);

/** Load the current button states into the data array */
void genesis_load(void);
