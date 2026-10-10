#include "platform.h"
#include "gamma-unsupported.h"
#include <stdio.h>

static int report_unsupported(redshift_state_t *state)
{
   if (!state || !state->unsupported_reported)
   {
      fputs("Redshift: native display gamma adjustment is not supported on this platform.\n", stderr);
      if (state) state->unsupported_reported = 1;
   }
   return -1;
}

int redshift_init(redshift_state_t *state)
{
   if (!state) return -1;
   state->supported = 0;
   state->unsupported_reported = 0;
   return 0;
}

int redshift_start(redshift_state_t *state)
{
   return report_unsupported(state);
}

void redshift_free(redshift_state_t *)
{
}

void redshift_restore(redshift_state_t *)
{
}

void redshift_print_help(char *buffer, int capacity)
{
   if (buffer && capacity > 0)
      snprintf(buffer, capacity, "Native display gamma adjustment is not supported on this platform.\n");
}

int redshift_set_option(redshift_state_t *state, const char *, const char *)
{
   return report_unsupported(state);
}

int redshift_set_temperature(redshift_state_t *state, const color_setting_t *)
{
   return report_unsupported(state);
}
