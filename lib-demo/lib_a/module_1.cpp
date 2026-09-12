//***************************************************************************
//
// Program example for subject Operating Systems
//
// Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2026
//
// Module for library
//
//***************************************************************************

#include <cstdio>

// Header file for library
#include "modules.h"


int module_1()
{
  // Self identification: file, fuction name and line number
  printf( "File: %s Function: %s Line: %d\n", __FILE__, __FUNCTION__, __LINE__ );

  return 0;
}
