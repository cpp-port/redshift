// created by camilo 2021-02-11 05:21 BRT <3TBS_!!
#pragma once


#include "_.h"
#include "redshift.h"


typedef struct _REDSHIFT_STATE redshift_state_t;



CLASS_DECL_REDSHIFT redshift_state_t * redshift_alloc();
CLASS_DECL_REDSHIFT void redshift_destroy(redshift_state_t *);

//CLASS_DECL_REDSHIFT color_setting_t * redshift_color_setting_alloc();
//CLASS_DECL_REDSHIFT void redshift_color_setting_destroy(color_setting_t *);

CLASS_DECL_REDSHIFT int redshift_init(redshift_state_t * state);
CLASS_DECL_REDSHIFT int redshift_start(redshift_state_t * state);
// Checks native backend availability without initializing or changing a display.
CLASS_DECL_REDSHIFT int redshift_is_supported();
// Returns 1 after start successfully finds usable native gamma ramps; otherwise 0.
// Safe before initialization, after free, and with a null state.
CLASS_DECL_REDSHIFT int redshift_is_supported(const redshift_state_t * state);
CLASS_DECL_REDSHIFT void redshift_free(redshift_state_t * state);

CLASS_DECL_REDSHIFT void redshift_print_help(char * psz, int iLeft);
CLASS_DECL_REDSHIFT int redshift_set_option(redshift_state_t * state, const char * key,
   const char * value);

CLASS_DECL_REDSHIFT void redshift_restore(redshift_state_t * state);
CLASS_DECL_REDSHIFT int redshift_set_temperature(redshift_state_t * state,  const color_setting_t * color);




