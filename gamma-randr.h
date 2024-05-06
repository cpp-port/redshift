/* gamma-redshift.h -- X redshift gamma adjustment header
   This file is part of Redshift.

   Redshift is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Redshift is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Redshift.  If not, see <http://www.gnu.org/licenses/>.

   Copyright (c) 2010-2014  Jon Lund Steffensen <jonlst@gmail.com>
*/

#ifndef REDSHIFT_GAMMA_redshift_H
#define REDSHIFT_GAMMA_redshift_H

#include <stdio.h>
#include <stdint.h>

#include <xcb/xcb.h>


// Ubuntu : libxcb-randr0-dev
#include <xcb/randr.h>

#include "redshift.h"
//#include "__standard_type.h"


typedef struct {
	xcb_randr_crtc_t crtc;
	unsigned int ramp_size;
	unsigned short *saved_ramps;
} redshift_crtc_state_t;

typedef struct _REDSHIFT_STATE {
	xcb_connection_t *conn;
	xcb_screen_t *screen;
	int preferred_screen;
	int preserve;
	int screen_num;
	int crtc_num;
	unsigned int crtc_count;
	redshift_crtc_state_t *crtcs;
} redshift_state_t;


int redshift_init(redshift_state_t *state);
int redshift_start(redshift_state_t *state);
void redshift_free(redshift_state_t *state);

void redshift_print_help(FILE *f);
int redshift_set_option(redshift_state_t *state, const char * key, const char * value);

void redshift_restore(redshift_state_t *state);
int redshift_set_temperature(redshift_state_t *state,
			  const color_setting_t *setting);


#endif /* ! REDSHIFT_GAMMA_redshift_H */
