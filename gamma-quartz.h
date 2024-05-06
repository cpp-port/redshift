/* gamma-quartz.h -- Quartz (OSX) gamma adjustment header
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

   Copyright (c) 2014  Jon Lund Steffensen <jonlst@gmail.com>
*/

#ifndef REDSHIFT_GAMMA_redshift_H
#define REDSHIFT_GAMMA_redshift_H

#include <stdint.h>

#include <ApplicationServices/ApplicationServices.h>

#include "redshift.h"

typedef struct {
	CGDirectDisplayID display;
	uint32_t ramp_size;
	float *saved_ramps;
} redshift_display_state_t;

typedef struct _REDSHIFT_STATE {
	redshift_display_state_t *displays;
	uint32_t display_count;
	int preserve;
} redshift_state_t;

#include "gamma.h"


//int redshift_init(redshift_state_t *state);
//int redshift_start(redshift_state_t *state);
//void redshift_free(redshift_state_t *state);
//
//void redshift_print_help(FILE *f);
//int redshift_set_option(redshift_state_t *state, const char * key,
//		      const char *value);
//
//void redshift_restore(redshift_state_t *state);
//int redshift_set_temperature(redshift_state_t *state,
//			   const color_setting_t *setting);
//

#endif /* ! REDSHIFT_GAMMA_redshift_H */
