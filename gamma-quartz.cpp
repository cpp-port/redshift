/* gamma-quartz.c -- Quartz (OSX) gamma adjustment
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

#include "framework.h"


#include <stdio.h>


#include <ApplicationServices/ApplicationServices.h>


#undef _

#ifdef ENABLE_NLS
# include <libintl.h>
# define _(s) gettext(s)
#else
# define _(s) s
#endif

#include "gamma-quartz.h"
#include "colorramp.h"


int
redshift_init(redshift_state_t *state)
{
	state->preserve = 0;
	state->displays = nullptr;

	return 0;
}

int
redshift_start(redshift_state_t *state)
{
	//int r;
	CGError error;
	uint32_t display_count;

	/* Get display count */
	error = CGGetOnlineDisplayList(0, nullptr, &display_count);
	if (error != kCGErrorSuccess) return -1;

	state->display_count = display_count;

	CGDirectDisplayID* displays =(CGDirectDisplayID* )
		malloc(sizeof(CGDirectDisplayID)*display_count);
	if (displays == nullptr) {
		fprintf(stderr, "malloc");
		return -1;
	}

	/* Get list of displays */
	error = CGGetOnlineDisplayList(display_count, displays,
				       &display_count);
	if (error != kCGErrorSuccess) {
		free(displays);
		return -1;
	}

	/* Allocate list of display state */
	state->displays = (redshift_display_state_t *)malloc(display_count *
				 sizeof(redshift_display_state_t));
	if (state->displays == nullptr) {
		fprintf(stderr, "malloc");
		free(displays);
		return -1;
	}

	/* Copy display indentifiers to display state */
	for (int i = 0; i < display_count; i++) {
		state->displays[i].display = displays[i];
		state->displays[i].saved_ramps = nullptr;
	}

	free(displays);

	/* Save gamma ramps for all displays in display state */
	for (int i = 0; i < display_count; i++) {
		CGDirectDisplayID display = state->displays[i].display;

		uint32_t ramp_size = CGDisplayGammaTableCapacity(display);
		if (ramp_size == 0) {
			fprintf(stderr, _("Gamma ramp size_i32 too small: %i\n"),
				ramp_size);
			return -1;
		}

		state->displays[i].ramp_size = ramp_size;

		/* Allocate space for saved ramps */
		state->displays[i].saved_ramps =
			(float *) malloc(3 * ramp_size * sizeof(float));
		if (state->displays[i].saved_ramps == nullptr) {
			fprintf(stderr, "malloc");
			return -1;
		}

		float *gamma_r = &state->displays[i].saved_ramps[0*ramp_size];
		float *gamma_g = &state->displays[i].saved_ramps[1*ramp_size];
		float *gamma_b = &state->displays[i].saved_ramps[2*ramp_size];

		/* Copy the ramps to allocated space */
		uint32_t sample_count;
		error = CGGetDisplayTransferByTable(display, ramp_size,
						    gamma_r, gamma_g, gamma_b,
						    &sample_count);
		if (error != kCGErrorSuccess ||
		    sample_count != ramp_size) {
			fputs(_("Unable to save current gamma ramp.\n"),
			      stderr);
			return -1;
		}
	}

	return 0;
}

void
redshift_restore(redshift_state_t *state)
{
	CGDisplayRestoreColorSyncSettings();
}

void
redshift_free(redshift_state_t *state)
{
	if (state->displays != nullptr) {
		for (int i = 0; i < state->display_count; i++) {
			free(state->displays[i].saved_ramps);
		}
	}
	free(state->displays);
}

void
redshift_print_help(FILE *f)
{
	fputs(_("Adjust gamma ramps on OSX using Quartz.\n"), f);
	fputs("\n", f);

	/* TRANSLATORS: Quartz help output
	   left column must not be translated */
	fputs(_("  preserve={0,1}\tWhether existing gamma should be"
		" preserved\n"),
	      f);
	fputs("\n", f);
}

int
redshift_set_option(redshift_state_t *state, const char * key, const char * value)
{
	if (strcasecmp(key, "preserve") == 0) {
		state->preserve = atoi(value);
	} else {
		fprintf(stderr, _("Unknown method parameter: `%s'.\n"), key);
		return -1;
	}

	return 0;
}

static void
redshift_set_temperature_for_display(redshift_state_t *state, int display,
				   const color_setting_t *setting)
{
	uint32_t ramp_size = state->displays[display].ramp_size;

	/* Create new gamma ramps */
	float *gamma_ramps = (float *) malloc(3*ramp_size*sizeof(float));
	if (gamma_ramps == nullptr) {
		fprintf(stderr, "malloc");
		return;
	}

	float *gamma_r = &gamma_ramps[0*ramp_size];
	float *gamma_g = &gamma_ramps[1*ramp_size];
	float *gamma_b = &gamma_ramps[2*ramp_size];

	if (state->preserve) {
		/* Initialize gamma ramps from saved state */
		::memcpy(gamma_ramps, state->displays[display].saved_ramps,
		       3*ramp_size*sizeof(float));
	} else {
		/* Initialize gamma ramps to pure state */
		for (int i = 0; i < ramp_size; i++) {
			float value = (double)i/ramp_size;
			gamma_r[i] = value;
			gamma_g[i] = value;
			gamma_b[i] = value;
		}
	}

	colorramp_fill_float(gamma_r, gamma_g, gamma_b, ramp_size,
			     setting);

   
	CGError error =
		CGSetDisplayTransferByTable(state->displays[display].display, ramp_size,
					    gamma_r, gamma_g, gamma_b);
	if (error != kCGErrorSuccess) {
		free(gamma_ramps);
		return;
	}

	free(gamma_ramps);
}

int
redshift_set_temperature(redshift_state_t *state,
		       const color_setting_t *setting)
{
	for (int i = 0; i < state->display_count; i++) {
		redshift_set_temperature_for_display(state, i, setting);
	}

	return 0;
}
