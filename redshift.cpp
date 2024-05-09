//
//  redshift.cpp
//  app_core_ambient
//
//  Created by Camilo Sasuke on 2021-06-21 19:11 BRT <3ThomasBS_!!
//  Copyright (c) 2021 Camilo Sasuke Thomas Borregaard Soerensen. All rights reserved.
//
#include "framework.h"


#ifdef _WIN32


//#include "acme/_operating_system.h"

#include "gamma-w32gdi.h"


#elif defined(LINUX) || defined(FREEBSD) || defined(OPENBSD)


#include "redshift/gamma-randr.h"


#else


#include "redshift/gamma-quartz.h"


#endif


#include <stdlib.h>


redshift_state_t * redshift_alloc()
{

   return (redshift_state_t * )malloc(sizeof(redshift_state_t));

}


void redshift_destroy(redshift_state_t *p)
{

   free(p);

}



//
//CLASS_DECL_REDSHIFT color_setting_t * redshift_color_setting_alloc()
//{
//   return (color_setting_t *)malloc(sizeof(color_setting_t));
//}
//CLASS_DECL_REDSHIFT void redshift_color_setting_destroy(color_setting_t * p)
//{
//
//   free(p);
//
//}
//
//
