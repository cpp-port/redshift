/* gamma-redshift.c -- X redshift gamma adjustment source
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
//#include "framework.h"
//#include "acme/_operating_system.h"
//xxx #undef scoped_restore
#undef _



#include <stdio.h>

#include <stdint.h>
#include <string.h>

#include <malloc.h>

#include <stdlib.h>

#ifdef ENABLE_NLS
# include <libintl.h>
# define _(s) gettext(s)
#else
# define _(s) s
#endif

#include <xcb/xcb.h>

// apt install libxcb-randr0-dev
#include <xcb/randr.h>


#include "gamma-randr.h"
#include "redshift/_.h"
#include "redshift/redshift.h"
#include "colorramp.h"


#define redshift_VERSION_MAJOR  1
#define redshift_VERSION_MINOR  3


int
redshift_init(redshift_state_t *state)
{
	/* Initialize state. */
	state->screen_num = -1;
	state->crtc_num = -1;

	state->crtc_count = 0;
	state->crtcs = nullptr;

	state->preserve = 0;

	xcb_generic_error_t *error;

	/* Open X server connection */
	state->conn = xcb_connect(nullptr, &state->preferred_screen);

	/* Query redshift version */
	xcb_randr_query_version_cookie_t ver_cookie =
		xcb_randr_query_version(state->conn, redshift_VERSION_MAJOR,
					redshift_VERSION_MINOR);
	xcb_randr_query_version_reply_t *ver_reply =
		xcb_randr_query_version_reply(state->conn, ver_cookie, &error);

	/* TODO What does it mean when both error and ver_reply is nullptr?
	   Apparently, we have to check both to avoid seg faults. */
	if (error || ver_reply == nullptr) {
		int ec = (error != 0) ? error->error_code : -1;
		fprintf(stderr, _("`%s' returned error %d\n"),
			"redshift Query Version", ec);
		xcb_disconnect(state->conn);
		return -1;
	}

	if (ver_reply->major_version != redshift_VERSION_MAJOR ||
	    ver_reply->minor_version < redshift_VERSION_MINOR) {
		fprintf(stderr, _("Unsupported redshift version (%u.%u)\n"),
			ver_reply->major_version, ver_reply->minor_version);
		free(ver_reply);
		xcb_disconnect(state->conn);
		return -1;
	}

	free(ver_reply);

	return 0;
}

int
redshift_start(redshift_state_t *state)
{
	xcb_generic_error_t *error;

	int screen_num = state->screen_num;
	if (screen_num < 0) screen_num = state->preferred_screen;

	/* Get screen */
	const xcb_setup_t *setup = xcb_get_setup(state->conn);
	xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
	state->screen = nullptr;

	for (int i = 0; iter.rem > 0; i++) {
		if (i == screen_num) {
			state->screen = iter.data;
			break;
		}
		xcb_screen_next(&iter);
	}

	if (state->screen == nullptr) {
		fprintf(stderr, _("Screen %i could not be found.\n"),
			screen_num);
		return -1;
	}

	/* Get list of CRTCs for the screen */
	xcb_randr_get_screen_resources_current_cookie_t res_cookie =
		xcb_randr_get_screen_resources_current(state->conn,
						       state->screen->root);
	xcb_randr_get_screen_resources_current_reply_t *res_reply =
		xcb_randr_get_screen_resources_current_reply(state->conn,
							     res_cookie,
							     &error);

	if (error) {
		fprintf(stderr, _("`%s' returned error %d\n"),
			"redshift Get Screen Resources Current",
			error->error_code);
		return -1;
	}

	state->crtc_count = res_reply->num_crtcs;
	state->crtcs = (redshift_crtc_state_t *)calloc(state->crtc_count, sizeof(redshift_crtc_state_t));
	if (state->crtcs == nullptr) {
		perror("malloc");
		state->crtc_count = 0;
		return -1;
	}

	xcb_randr_crtc_t *crtcs =
		xcb_randr_get_screen_resources_current_crtcs(res_reply);

	/* Save CRTC identifier in state */
	for (int i = 0; i < state->crtc_count; i++) {
		state->crtcs[i].crtc = crtcs[i];
	}

	free(res_reply);

	/* Save size_i32 and gamma ramps of all CRTCs.
	   Current gamma ramps are saved so we can restore them
	   at program exit. */
	for (int i = 0; i < state->crtc_count; i++) {
		xcb_randr_crtc_t crtc = state->crtcs[i].crtc;

		/* Request size_i32 of gamma ramps */
		xcb_randr_get_crtc_gamma_size_cookie_t gamma_size_cookie =
			xcb_randr_get_crtc_gamma_size(state->conn, crtc);
		xcb_randr_get_crtc_gamma_size_reply_t *gamma_size_reply =
			xcb_randr_get_crtc_gamma_size_reply(state->conn,
							    gamma_size_cookie,
							    &error);

		if (error) {
			fprintf(stderr, _("`%s' returned error %d\n"),
				"redshift Get CRTC Gamma Size",
				error->error_code);
			return -1;
		}

		unsigned int ramp_size = gamma_size_reply->size;
		state->crtcs[i].ramp_size = ramp_size;

		free(gamma_size_reply);

		if (ramp_size == 0) {
			fprintf(stderr, _("Gamma ramp size_i32 too small: %i\n"),
				ramp_size);
			return -1;
		}

		/* Request current gamma ramps */
		xcb_randr_get_crtc_gamma_cookie_t gamma_get_cookie =
			xcb_randr_get_crtc_gamma(state->conn, crtc);
		xcb_randr_get_crtc_gamma_reply_t *gamma_get_reply =
			xcb_randr_get_crtc_gamma_reply(state->conn,
						       gamma_get_cookie,
						       &error);

		if (error) {
			fprintf(stderr, _("`%s' returned error %d\n"),
				"redshift Get CRTC Gamma", error->error_code);
			return -1;
		}

		unsigned short *gamma_r =
			xcb_randr_get_crtc_gamma_red(gamma_get_reply);
		unsigned short *gamma_g =
			xcb_randr_get_crtc_gamma_green(gamma_get_reply);
		unsigned short *gamma_b =
			xcb_randr_get_crtc_gamma_blue(gamma_get_reply);

		/* Allocate space for saved gamma ramps */
		state->crtcs[i].saved_ramps =(unsigned short *)
			malloc(3*ramp_size*sizeof(unsigned short));
		if (state->crtcs[i].saved_ramps == nullptr) {
			perror("malloc");
			free(gamma_get_reply);
			return -1;
		}

		/* Copy gamma ramps into CRTC state */
		::memcpy(&state->crtcs[i].saved_ramps[0*ramp_size], gamma_r,
		       ramp_size*sizeof(unsigned short));
		::memcpy(&state->crtcs[i].saved_ramps[1*ramp_size], gamma_g,
		       ramp_size*sizeof(unsigned short));
		::memcpy(&state->crtcs[i].saved_ramps[2*ramp_size], gamma_b,
		       ramp_size*sizeof(unsigned short));

		free(gamma_get_reply);
	}

	return 0;
}

void
redshift_restore(redshift_state_t *state)
{
	xcb_generic_error_t *error;

	/* Restore CRTC gamma ramps */
	for (int i = 0; i < state->crtc_count; i++) {
		xcb_randr_crtc_t crtc = state->crtcs[i].crtc;

		unsigned int ramp_size = state->crtcs[i].ramp_size;
		unsigned short *gamma_r = &state->crtcs[i].saved_ramps[0*ramp_size];
		unsigned short *gamma_g = &state->crtcs[i].saved_ramps[1*ramp_size];
		unsigned short *gamma_b = &state->crtcs[i].saved_ramps[2*ramp_size];

		/* Set gamma ramps */
		xcb_void_cookie_t gamma_set_cookie =
			xcb_randr_set_crtc_gamma_checked(state->conn, crtc,
							 ramp_size, gamma_r,
							 gamma_g, gamma_b);
		error = xcb_request_check(state->conn, gamma_set_cookie);

		if (error) {
			fprintf(stderr, _("`%s' returned error %d\n"),
				"redshift Set CRTC Gamma", error->error_code);
			fprintf(stderr, _("Unable to restore CRTC %i\n"), i);
		}
	}
}

void
redshift_free(redshift_state_t *state)
{
	/* Free CRTC state */
	for (int i = 0; i < state->crtc_count; i++) {
		free(state->crtcs[i].saved_ramps);
	}
	free(state->crtcs);

	/* Close connection */
	xcb_disconnect(state->conn);
}

void
redshift_print_help(FILE *f)
{
	fputs(_("Adjust gamma ramps with the X redshift extension.\n"), f);
	fputs("\n", f);

	/* TRANSLATORS: redshift help output
	   left column must not be translated */
	fputs(_("  screen=N\t\tX screen to apply adjustments to\n"
		"  crtc=N\t\tCRTC to apply adjustments to\n"
		"  preserve={0,1}\tWhether existing gamma should be"
		" preserved\n"),
	      f);
	fputs("\n", f);
}

int
redshift_set_option(redshift_state_t *state, const char * key, const char * value)
{
	if (strcasecmp(key, "screen") == 0) {
		state->screen_num = atoi(value);
	} else if (strcasecmp(key, "crtc") == 0) {
		state->crtc_num = atoi(value);
	} else if (strcasecmp(key, "preserve") == 0) {
		state->preserve = atoi(value);
	} else {
		fprintf(stderr, _("Unknown method parameter: `%s'.\n"), key);
		return -1;
	}

	return 0;
}

static int
redshift_set_temperature_for_crtc(redshift_state_t *state, int crtc_num,
			       const color_setting_t *setting)
{
	xcb_generic_error_t *error;

	if (crtc_num >= state->crtc_count || crtc_num < 0) {
		fprintf(stderr, _("CRTC %d does not exist. "),
			state->crtc_num);
		if (state->crtc_count > 1) {
			fprintf(stderr, _("Valid CRTCs are [0-%d].\n"),
				state->crtc_count-1);
		} else {
			fprintf(stderr, _("Only CRTC 0 exists.\n"));
		}

		return -1;
	}

	xcb_randr_crtc_t crtc = state->crtcs[crtc_num].crtc;
	unsigned int ramp_size = state->crtcs[crtc_num].ramp_size;

	/* Create new gamma ramps */
	unsigned short *gamma_ramps = (unsigned short *)malloc(3*ramp_size*sizeof(unsigned short));
	if (gamma_ramps == nullptr) {
		perror("malloc");
		return -1;
	}

	unsigned short *gamma_r = &gamma_ramps[0*ramp_size];
	unsigned short *gamma_g = &gamma_ramps[1*ramp_size];
	unsigned short *gamma_b = &gamma_ramps[2*ramp_size];

	if (state->preserve) {
		/* Initialize gamma ramps from saved state */
		::memcpy(gamma_ramps, state->crtcs[crtc_num].saved_ramps,
		       3*ramp_size*sizeof(unsigned short));
	} else {
		/* Initialize gamma ramps to pure state */
		for (int i = 0; i < ramp_size; i++) {
			unsigned short value = (double)i/ramp_size * (UINT16_MAX+1);
			gamma_r[i] = value;
			gamma_g[i] = value;
			gamma_b[i] = value;
		}
	}

	colorramp_fill(gamma_r, gamma_g, gamma_b, ramp_size,
		       setting);

	/* Set new gamma ramps */
	xcb_void_cookie_t gamma_set_cookie =
		xcb_randr_set_crtc_gamma_checked(state->conn, crtc,
						 ramp_size, gamma_r,
						 gamma_g, gamma_b);
	error = xcb_request_check(state->conn, gamma_set_cookie);

	if (error) {
		fprintf(stderr, _("`%s' returned error %d\n"),
			"redshift Set CRTC Gamma", error->error_code);
		free(gamma_ramps);
		return -1;
	}

	free(gamma_ramps);

	return 0;
}

int
redshift_set_temperature(redshift_state_t *state,
		      const color_setting_t *setting)
{
	int r;

	/* If no CRTC number has been specified,
	   set temperature on all CRTCs. */
	if (state->crtc_num < 0) {
		for (int i = 0; i < state->crtc_count; i++) {
			r = redshift_set_temperature_for_crtc(state, i,
							   setting);
			if (r < 0) return -1;
		}
	} else {
		return redshift_set_temperature_for_crtc(state, state->crtc_num,
						      setting);
	}

	return 0;
}



//redshift_state_t * redshift_alloc()
//{
//
//   return (redshift_state_t * )malloc(sizeof(redshift_state_t));
//
//}
//
//
//void redshift_destroy(redshift_state_t *p)
//{
//
//   free(p);
//
//}
