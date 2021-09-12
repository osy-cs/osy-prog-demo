//***************************************************************************
//
// Program example for subject Operating Systems
//
// Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2021
//
// Module for library
//
//***************************************************************************

#include <stdio.h>
#include <stdlib.h>

// Header file for library
#include "modules.h"


int module_2()
{
  // Self identification: file, fuction name and line number
  printf( "File: %s Function: %s Line: %d\n", __FILE__, __FUNCTION__, __LINE__ );
}

