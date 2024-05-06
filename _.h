#pragma once



#include "acme/_.h"


#ifdef _WIN32
#define CLASS_DECL_EXPORT __declspec(dllexport)
#define CLASS_DECL_IMPORT __declspec(dllimport)
#else
#define CLASS_DECL_EXPORT
#define CLASS_DECL_IMPORT
#endif


#if defined(_redshift_project)
    #define CLASS_DECL_REDSHIFT CLASS_DECL_EXPORT
#else
    #define CLASS_DECL_REDSHIFT CLASS_DECL_IMPORT
#endif



