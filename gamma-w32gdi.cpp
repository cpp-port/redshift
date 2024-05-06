/* gamma-redshift.c -- Windows GDI gamma adjustment source
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
#include "framework.h"
#undef USUAL_OPERATING_SYSTEM_SUPPRESSIONS
#include "acme/_operating_system.h"
#include <stdio.h>
#include "gamma-w32gdi.h"


#ifndef WINVER
# define WINVER  0x0500
#endif
//#include <windows.h>
//#include <wingdi.h>

//#ifdef ENABLE_NLS
//# include <libintl.h>
//# define _(s) gettext(s)
//#else
//# define _(s) s
//#endif



//#include "gamma-w32gdi.h"
#include "colorramp.h"

#define GAMMA_RAMP_SIZE  256


int
redshift_init(redshift_state_t *state)
{
   state->saved_ramps = nullptr;
   state->preserve = 0;

   return 0;
}

int
redshift_start(redshift_state_t *state)
{
   BOOL r;

   /* Open device context */
   HDC hDC = GetDC(nullptr);
   if (hDC == nullptr)
   {
      fputs(("Unable to open device context.\n"), stderr);
      return -1;
   }

   /* Check support for gamma ramps */
   int cmcap = GetDeviceCaps(hDC, COLORMGMTCAPS);
   if (cmcap != CM_GAMMA_RAMP)
   {
      fputs(("Display device does not support gamma ramps.\n"),
            stderr);
      return -1;
   }

   /* Allocate space for saved gamma ramps */
   state->saved_ramps = (::uint16_t *) malloc(3*GAMMA_RAMP_SIZE*sizeof(::uint16_t));
   if (state->saved_ramps == nullptr)
   {
      perror("malloc");
      ReleaseDC(nullptr, hDC);
      return -1;
   }

   /* Save current gamma ramps so we can restore them at program exit */
   r = GetDeviceGammaRamp(hDC, state->saved_ramps);
   if (!r)
   {
      fputs(("Unable to save current gamma ramp.\n"), stderr);
      ReleaseDC(nullptr, hDC);
      return -1;
   }

   /* Release device context */
   ReleaseDC(nullptr, hDC);

   return 0;
}

void
redshift_free(redshift_state_t *state)
{
   /* Free saved ramps */
   free(state->saved_ramps);
}


void
redshift_print_help(char * psz, int iLeft)
{
   iLeft -= snprintf(psz, iLeft, ("Adjust gamma ramps with the Windows GDI.\n"));
   iLeft -= snprintf(psz, iLeft, "\n");

   /* TRANSLATORS: Windows GDI help output
      left column must not be translated */
   iLeft -= snprintf(psz, iLeft, "  preserve={0,1}\tWhether existing gamma should be"
           " preserved\n");
   iLeft -= snprintf(psz, iLeft, "\n");
}

int
redshift_set_option(redshift_state_t *state, const char *key, const char * value)
{
   if (_stricmp(key, "preserve") == 0)
   {
      state->preserve = atoi(value);
   }
   else
   {
      fprintf(stderr, ("Unknown method parameter: `%s'.\n"), key);
      return -1;
   }

   return 0;
}

void
redshift_restore(redshift_state_t *state)
{
   /* Open device context */
   HDC hDC = GetDC(nullptr);
   if (hDC == nullptr)
   {
      fputs(("Unable to open device context.\n"), stderr);
      return;
   }

   /* Restore gamma ramps */
   BOOL r = SetDeviceGammaRamp(hDC, state->saved_ramps);
   if (!r) fputs(("Unable to restore gamma ramps.\n"), stderr);

   /* Release device context */
   ReleaseDC(nullptr, hDC);
}

int
redshift_set_temperature(redshift_state_t *state,
                         const color_setting_t *setting)
{
   BOOL r;

   /* Open device context */
   HDC hDC = GetDC(nullptr);
   if (hDC == nullptr)
   {
      fputs(("Unable to open device context.\n"), stderr);
      return -1;
   }

   /* Create new gamma ramps */
   ::uint16_t *gamma_ramps = (::uint16_t *) malloc(3*GAMMA_RAMP_SIZE*sizeof(::uint16_t));
   if (gamma_ramps == nullptr)
   {
      perror("malloc");
      ReleaseDC(nullptr, hDC);
      return -1;
   }

   ::uint16_t *gamma_r = &gamma_ramps[0*GAMMA_RAMP_SIZE];
   ::uint16_t *gamma_g = &gamma_ramps[1*GAMMA_RAMP_SIZE];
   ::uint16_t *gamma_b = &gamma_ramps[2*GAMMA_RAMP_SIZE];

   if (state->preserve)
   {
      /* Initialize gamma ramps from saved state */
      ::memcpy(gamma_ramps, state->saved_ramps,
             3*GAMMA_RAMP_SIZE*sizeof(::uint16_t));
   }
   else
   {
      /* Initialize gamma ramps to pure state */
      for (int i = 0; i < GAMMA_RAMP_SIZE; i++)
      {
         ::uint16_t value = (::uint16_t) ((double)i/GAMMA_RAMP_SIZE *
                              (UINT16_MAX+1));
         gamma_r[i] = value;
         gamma_g[i] = value;
         gamma_b[i] = value;
      }
   }

   colorramp_fill(gamma_r, gamma_g, gamma_b, GAMMA_RAMP_SIZE,
                  setting);

   /* Set new gamma ramps */
   r = SetDeviceGammaRamp(hDC, gamma_ramps);
   if (!r)
   {
      DWORD dwError = GetLastError();
      /* TODO it happens that SetDeviceGammaRamp returns false on
         occasions where the adjustment seems to be successful.
         Does this only happen with multiple monitors connected? */
      fputs(("Unable to set gamma ramps.\n"), stderr);
      free(gamma_ramps);
      ReleaseDC(nullptr, hDC);
      return -1;
   }

   free(gamma_ramps);

   /* Release device context */
   ReleaseDC(nullptr, hDC);

   return 0;
}



